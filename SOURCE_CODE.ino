#define BLYNK_TEMPLATE_ID "TMPL6v43CmHsH"

#define BLYNK_TEMPLATE_NAME "Griham"

#define BLYNK_AUTH_TOKEN "bkic4ESuxOakSv2uE5Sb_nyVNFvW1Bwu"

#include <LiquidCrystal_I2C.h> //lcd library

#include <Wire.h>//memory initialization

#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>

#include <BlynkSimpleEsp8266.h>

#include <WiFiUdp.h>


#include <EEPROM.h>

#include <ArduinoOTA.h>

char ssid[] = "Home Wifi";
char pass[] = "Prettyhouse675";
const int flashAddress = 0;




int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
//const char * ssid = "Home Wifi";
//const char * password = "Prettyhouse675";
int Upperdepth = 150;
int Lowerdepth = 150;
int motorTrigPin = 15;
int trigUprTnkPin = 12;
int echoUprTnkPin = 13;
int trigLwrTnkPin = 5;
int echoLwrTnkPin = 4;
long timeperiodUpr, timeperiodLwr, cmUpr, cmLwr;
int Uarray = 100;
int avgReadingUpr[10] = {
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray
};
int avgReadingLwr[10] = {
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray,
  Uarray
};
int indexUpr = 0;
int indexLwr = 0;
long avgUprTnkPer = 0;
long avgLwrTnkPer = 0;
float Upperlow;
int Upperhigh = 95;
int Lowerlow = 30;

void setup() {
  Serial.begin(9600);
   EEPROM.begin(512);
  Wire.begin(2, 0); //SDA, SCL
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  lcd.init();
  lcd.backlight();
  lcd.begin(16, 2);
  lcd.setCursor(5, 0);
  lcd.print("GRIHAM");
  lcd.setCursor(1, 1);
  lcd.print(".:AUTOMATION.:");
  delay(500);
  lcd.clear();
  lcd.setCursor(5, 1);
  lcd.print("V7.0");
  delay(500);
  lcd.setCursor(0, 1);
  lcd.clear();
  lcd.print("CONNECTING>");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    {
      lcd.setCursor(0, 0);
      lcd.print("Connect Gateway...");
      delay(500);
    }
  }
  ArduinoOTA.onStart([]() {
    lcd.clear();
    lcd.print("Updating........");
    lcd.setCursor(0, 1);
    lcd.print("DO NOT TURN OFF!");
  });
  ArduinoOTA.onEnd([]() {
    lcd.clear();
    lcd.print("Complete!!!!!!!!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.begin();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(WiFi.localIP());
  lcd.setCursor(0, 1);
  lcd.print("Registered");
  delay(800);
  pinMode(trigUprTnkPin, OUTPUT);
  pinMode(echoUprTnkPin, INPUT);
  pinMode(trigLwrTnkPin, OUTPUT);
  pinMode(echoLwrTnkPin, INPUT);
  pinMode(motorTrigPin, OUTPUT);
  EEPROM.get(flashAddress, Upperlow);
  lcd.clear();
}

float getAverage(int nextVal, int index, int avgReading[]) {
  if (nextVal != 0) {
    avgReading[index] = nextVal;
  }
  int total = 0;
  for (int j = 0; j < 10; j++) {
    total = total + avgReading[j];
  }
  float avg = total / 10;
  return avg;
}

void startLcd(int avgUprTnkPer, int avgLwrTnkPer) {
  if (avgUprTnkPer <= 0) {
    avgUprTnkPer = 1;
  } else if (avgUprTnkPer > 100) {
    avgUprTnkPer = 100;
  }
  if (avgLwrTnkPer <= 0) {
    avgLwrTnkPer = 100;
  }
  if (avgLwrTnkPer > 100) {
    avgLwrTnkPer = 100;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LEVEL1%");
  lcd.setCursor(0, 1);
  lcd.print(avgUprTnkPer);
 // lcd.setCursor(7, 0);
  //lcd.print("||");
 // lcd.setCursor(7, 1);
 // lcd.print("||");
  lcd.setCursor(9, 0);
  lcd.print("LEVEL2%");
  lcd.setCursor(9, 1);
  lcd.print(avgLwrTnkPer);
  Blynk.virtualWrite(V5,(int)avgUprTnkPer);
  Blynk.virtualWrite(V6,(int)avgLwrTnkPer);
}

void startMotor() {

  digitalWrite(motorTrigPin, HIGH);
  Blynk.virtualWrite(V3, HIGH);
}

void stopMotor() {
  digitalWrite(motorTrigPin, LOW);
  Blynk.virtualWrite(V3, LOW);
}

long getEchoTimeFromTankSensor(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(20);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH);
}

void triggersonar() {
  timeperiodUpr = getEchoTimeFromTankSensor(trigUprTnkPin, echoUprTnkPin);
  cmUpr = microsecondsToCentimeters(timeperiodUpr);
  delay(5);
  timeperiodLwr = getEchoTimeFromTankSensor(trigLwrTnkPin, echoLwrTnkPin);
  cmLwr = microsecondsToCentimeters(timeperiodLwr);
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  triggersonar();
  if (indexUpr >= 10) {
    indexUpr = 0;
  } else {
    indexUpr++;
  }
  if (indexLwr >= 10) {
    indexLwr = 0;
  } else {
    indexLwr++;
  }

  float avgUpr = getAverage(cmUpr, indexUpr, avgReadingUpr);
  avgUprTnkPer = (((Upperdepth - avgUpr) / Upperdepth) * 100)+22;
  float avgLwr = getAverage(cmLwr, indexLwr, avgReadingLwr);
  avgLwrTnkPer = (((Lowerdepth - avgLwr) / Lowerdepth) * 100)+22;

  startLcd(avgUprTnkPer, avgLwrTnkPer);

  if (avgUprTnkPer < Upperlow && avgLwrTnkPer > Lowerlow) {
    startMotor();

  } else if (avgUprTnkPer > Upperhigh || avgLwrTnkPer < Lowerlow) {
    stopMotor();

    

  }
  delay(2000);
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

BLYNK_WRITE(V1) {
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable

  if (pinValue == 1) {
    startMotor();
  } else {
    stopMotor();
  }
}

BLYNK_WRITE(V4) {
  Upperlow = param.asInt();

  EEPROM.put(flashAddress, Upperlow);
  EEPROM.commit();
}
