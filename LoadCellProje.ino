
#include "config.h"
#include "HX711.h"
#include "string.h"
#define debug_HX711 
#define calibration 807
#define pinLed D5

/**********************************************************/

int count = 1;

AdafruitIO_Feed *counter = io.feed("Okunan değer");


byte pinData = D2;
byte pinClk = D3;
float units;
float sondeger = 0; //en son ölçülen gram değeri
float sayac = 0; //döngüyü sayar
float sayac2 = 0; //döngü 2yi sayar
float son = 30; //döngü sonu
float i = 0;
String stat = "Bos";
/*
 * Bos: Şu anda bir ağırlık yok ve öncesinde de olmadı.
 * Dolu: Yakın zamanda uzun süreli ağırlık taşındı veya başlandı
 * Kalk: Bir süre oturduktan sonra kalkma uyarısı
 * Dinlen: Kalkma uyarısından sonra bir süre dinlenme durumu
 */

HX711 bascula;

void setup() {

  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // we are connected
  Serial.println();
  Serial.println(io.statusText());


   #ifdef debug_HX711
    Serial.begin(115200);
    Serial.println("[HX7] başlangıç sensörü HX711");
  #endif
  pinMode(pinLed, OUTPUT);
  bascula.begin(pinData,pinClk);
  bascula.set_scale(calibration);
  bascula.tare();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
}

void loop() {
  

  Serial.print("sending -> ");
  Serial.println(count);
  i++;
  if (i==10) //Adafruit'e 10 saniyede bir veri gönder
  {
    io.run();
    counter->save(units);
    i = 0;
  }
  

  #ifdef debug_HX711
   //Serial.print("[HX7] Okunan Değer: ");
   //Serial.print(bascula.get_units(), 10); 
   units = -bascula.get_units();
   if (units < 0)
  {
    units = 0.00;
  }
  units = round(units/10)*10;
  Serial.print("Okunan Değer: ");
  Serial.print(units);
  Serial.print(" gram");
  digitalWrite(pinLed, LOW); //Işığı söndür

  if (stat == "Bos") //Eğer sandalye şu an boşsa
  {
    if (units > 5) //Boş olmaması için eşik değeri, en az 5 gram ölçmesi gerek
    {
      sayac++;
      if (sayac > 3) //Eğer x saniye boyunca 5 gramdan fazla ölçerse DOLU status'una geçer
      {
        stat = "Dolu";
        sayac = 0;
      Serial.print(" ağırlık algılandı... ");
      }
    }
    else //Eğer sandalye boş ise sayacı da sıfırlayalım
    {
      sayac = 0;
    }
  }
  else if (stat == "Dolu")
  {
    if (units > 5)
    {
      sayac++;
      sayac2 = 0;
      if (sayac > 20) //Eğer x saniye boyunca 5 gramdan fazla ölçerse KALK status'una geçmeli
      {
        stat = "Kalk";
        sayac = 0;
      Serial.print(" Çok uzun süre boyunca hareketsiz kaldınız, hareket edin...");
      }
    }
    else
    {
      sayac2++;
      if (sayac2 > 5)
      {
        sayac = 0;
        sayac2 = 0;
        stat = "Bos";
      Serial.print(" Ağırlık algılanmadı");
      }
    }
  }
  else if (stat == "Kalk")
  {
    for (i = 0; i < 5; i++)
    {
      digitalWrite(pinLed, HIGH); //Işığı yak
      delay(100);
      digitalWrite(pinLed, LOW); //Işığı söndür
      delay(100);
    }

    if (units <= 5)
    {
      stat = "Dinlen";
      Serial.print(" Hissedilen ağırlık yok..");
    }
  }
  else if (stat == "Dinlen")
  {
    if (units > 5)
    {
      stat = "Kalk";
      Serial.print(" Yeterli süre boyunca hareket etmediniz, lütfen kalkınız...");
    }
    else
    {
      sayac++;
      if (sayac > 5)
      {
        stat = "Bos";
        sayac = 0;
      Serial.print(" Ağırlık algılanmadı");
      }
    }
  }
  
  Serial.println();
  delay(1000);
  Serial.println();
 #endif

 
  count++;

}
