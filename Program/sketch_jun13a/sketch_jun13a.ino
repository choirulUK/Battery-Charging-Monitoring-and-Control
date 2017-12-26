#include "ACS712.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "LiquidCrystal_I2C.h"
#include "Keypad.h"
#include <Wire.h>

//defenisi parameter batterai
#define TEGANGAN_MAX 13         //volt
#define ARUS_MAX 4               //Amphere
#define SUHU_MAX 32               //celcius

//definisi pin output
#define RELAY_PIN 13
#define LED_PIN 12
#define BUZZER_PIN 11

//definisi pin 2 sensor input
#define SENSOR_TEGANGAN_PIN A1
#define SENSOR_ARUS_PIN A0
#define SENSOR_SUHU_PIN 2

//definisi jumlah IO sensor
#define JML_OUTPIN 3
#define JML_INPIN 2

//definisikan parameter sensor tegangan
#define R1 30000
#define R2 7500
#define V_REFF 5.0

//definisi alamat LCD
#define alamatLCD 0x27

#define NOTHING 254

//setting KEYPAD
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {10, 9, 8, 7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 5, 4, 3}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad kypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
char stateKeypad, customKey;

//setting sensor arus                                   //setting sensor arus
ACS712 sensorArus(ACS712_30A, SENSOR_ARUS_PIN);

//setting sensor suhu                                   //setting sensor suhu
OneWire suhuWire(SENSOR_SUHU_PIN);
DallasTemperature sensorSuhu(&suhuWire);

//setting LCD
LiquidCrystal_I2C lcd(alamatLCD, 16, 2);                     //setting LCD
byte switchTampil = 0;

//alaram state
byte alarmState = 0;
boolean alarmBuzzer = 0;

enum komponen_output {
  relay = 0,
  led,
  buzzer
};
enum komponen_input {
  tegangan = 0,
  arus,
  suhu
};
enum onoff {
  off = 0,
  on,
};

typedef struct
{
  byte pin[JML_OUTPIN];
  float data[JML_OUTPIN];
} inout;

inout output;
inout input;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  //INIT_PIN
  ioInit();

  pinMode(output.pin[relay], OUTPUT);
  pinMode(output.pin[led], OUTPUT);
  pinMode(output.pin[buzzer], OUTPUT);
  pinMode(input.pin[tegangan], INPUT);
  pinMode(input.pin[arus], INPUT);

  //kalibrasi sensor arus
  sensorArus.calibrate();


  //sensor suhu start
  sensorSuhu.begin();

  //LCD Start
  lcd.begin();
  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.print("     POLTEK");
  lcd.setCursor(0, 1);
  lcd.print("   BALIKPAPAN");
  delay(1000);
  lcd.clear();
  lcd.print("     TEKNIK");
  lcd.setCursor(0, 1);
  lcd.print("MESIN-ALAT BERAT");
  delay(2000);
  lcd.clear();
  
  lcd.print("OK");

  output.data[relay] = on;
  output.data[led] = on;
  output.data[buzzer] = on;
}

void loop() {
  customKey = kypad.getKey();

  if (customKey) {
    Serial.println(customKey);
    if (customKey == 'D')
      alarmBuzzer = 1;
    else
      stateKeypad = customKey;
    lcd.clear();
  }

  if (stateKeypad == '1')
  {
    switchTampil = 0;
    tampilDataLCD();
  }
  else if (stateKeypad == '2' || stateKeypad == 'D')
  {
    switchTampil = 1;
    tampilDataLCD();
  }
  else if (stateKeypad == '3' || stateKeypad == 'D')
  {
    switchTampil = 2;
    tampilDataLCD();
  }
  else if (stateKeypad == 'A' || stateKeypad == 'D')
  {
    lcd.clear();
    tampilDataLCD();
    switchTampil++;
    if (switchTampil > 2)
      switchTampil = 0;
    delay(500);
  }
  else if (stateKeypad == '4' || stateKeypad == 'D')
  {
    switchTampil = 3;
    tampilDataLCD();
  }
  else if (stateKeypad == '5' || stateKeypad == 'D')
  {
    switchTampil = 4;
    tampilDataLCD();
  }
  else if (stateKeypad == '6' || stateKeypad == 'D')
  {
    switchTampil = 5;
    tampilDataLCD();
  }
  else if (stateKeypad == 'B' || stateKeypad == 'D')
  {
    lcd.clear();
    tampilDataLCD();
    switchTampil++;
    if (switchTampil > 5)
      switchTampil = 3;
    delay(500);
  }


  inRun();
  if (input.data[tegangan] > TEGANGAN_MAX)
    alarmState |= 0b00000001;
  else
    alarmState &= 0 << 0;
  if (input.data[arus] > ARUS_MAX)
    alarmState |= 0b00000010;
  else
    alarmState &= 0 << 1;
  if (input.data[suhu] > SUHU_MAX)
    alarmState |= 0b00000100;
  else
    alarmState &= 0 << 2;

  outRun();
  alarm();
  Serial.println(alarmState);
}

void tampilDataLCD()
{
  Serial.println("masuk");
  if (alarmState == 0 ||
      alarmState == NOTHING);
  //delay(1000);
  if (alarmState == 0b00000011 ||
      alarmState == 0b00000101 ||
      alarmState == 0b00000110 ||
      alarmState == 0b00000111 )
    delay(100);
  //lcd.clear();
  lcd.setCursor(0, 0);
  switch (switchTampil)
  {
    case 0:
      lcd.print("Sen. Arus: ");
      lcd.setCursor(0, 1);
      lcd.print(input.data[arus]);
      lcd.print(" (A)  ");
      break;
    case 1:
      lcd.print("Sen. Tegangan: ");
      lcd.setCursor(0, 1);
      lcd.print(input.data[tegangan]);
      lcd.print(" (V)  ");
      break;
    case 2:
      lcd.print("Sen. Suhu: ");
      lcd.setCursor(0, 1);
      lcd.print(input.data[suhu]);
      lcd.print(" (C) ");
      break;
    case 3:
      lcd.print("Charging: ");
      lcd.setCursor(0, 1);
      if (output.data[relay] == 1)
        lcd.print("On  ");
      else
        lcd.print("Off");
      break;
    case 4:
      lcd.print("Led: ");
      lcd.setCursor(0, 1);
      if (alarmState > 0)
        lcd.print("On  ");
      else
        lcd.print("Off");
      break;
    case 5:
      lcd.print("Buzzer: ");
      lcd.setCursor(0, 1);
      if (alarmState > 0)
        lcd.print("On  ");
      else
        lcd.print("Off");
      break;
  }
}

void tampilDataLaptop()
{
  Serial.println("**********************");
  Serial.print("Sensor Arus: ");
  Serial.println(input.data[arus]);
  Serial.print("Sensor Tegangan: ");
  Serial.println(input.data[tegangan]);
  Serial.print("Sensor Suhu: ");
  Serial.println(input.data[suhu]);
  Serial.print("Relay Status: ");
  Serial.println(output.data[relay]);
  Serial.print("Led Status: ");
  Serial.println(output.data[led]);
  Serial.print("Buzzer Status: ");
  Serial.println(output.data[buzzer]);
  Serial.println("**********************");
}

void outRun()
{
  for (byte a = 0; a <= JML_OUTPIN; a++)
  {
    if ((alarmState > 0) && (a == buzzer) && (alarmBuzzer == 1));
    else
      digitalWrite(output.pin[a], output.data[a]);
  }
}

void ioInit()
{
  output.pin[relay] = RELAY_PIN;
  output.pin[led] = LED_PIN;
  output.pin[buzzer] = BUZZER_PIN;
  input.pin[tegangan] = SENSOR_TEGANGAN_PIN;
  input.pin[arus] = SENSOR_ARUS_PIN;
}

void outTest()
{
  output.data[relay] = on;
  output.data[led] = on;
  output.data[buzzer] = on;
  outRun();
  tampilDataLaptop();
  delay(1000);
  output.data[relay] = off;
  output.data[led] = off;
  output.data[buzzer] = off;
  outRun();
  tampilDataLaptop();
  delay(1000);
}

void inRun()
{
  float temp;
  //baca data sensor arus
  input.data[arus] = sensorArus.getCurrentDC();

  //baca data sensor tegangan
  temp = analogRead(input.pin[tegangan]);
  temp = ((temp * 25.0) / 1024.0);
  input.data[tegangan] = temp;

  //baca data sensor suhu
  sensorSuhu.requestTemperatures();
  input.data[suhu] = sensorSuhu.getTempCByIndex(0);
}


/*
  Frame Alarm
  0b | 0 | 0 | 0 |   0    |    0    |   0
                    suhu  |  arus   | tegangan
*/
void alarm()  //
{
  if (alarmState == 0b00000001)            //tegangan
  {
    Serial.println("masuk tegangan");
    output.data[relay] = off;
    output.data[led] = on;
    output.data[buzzer] = on;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(200);
    output.data[led] = off;
    output.data[buzzer] = off;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(1000);
  }
  else if (alarmState == 0b00000010)
  {
    output.data[relay] = off;
    output.data[led] = on;
    output.data[buzzer] = on;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(200);
    output.data[led] = off;
    output.data[buzzer] = off;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(200);
    output.data[relay] = off;
    output.data[led] = on;
    output.data[buzzer] = on;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(200);
    output.data[led] = off;
    output.data[buzzer] = off;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(1000);
  }
  else if (alarmState == 0b00000100)
  {
    output.data[relay] = off;
    output.data[led] = on;
    output.data[buzzer] = on;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(200);
    output.data[led] = off;
    output.data[buzzer] = off;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(200);
    output.data[led] = on;
    output.data[buzzer] = on;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(200);
    output.data[led] = off;
    output.data[buzzer] = off;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
      goto out;
    delay(200);
    output.data[led] = on;
    output.data[buzzer] = on;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(200);
    output.data[led] = off;
    output.data[buzzer] = off;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(1000);
  }
  else if (alarmState == 0b00000011 ||
           alarmState == 0b00000101 ||
           alarmState == 0b00000110 ||
           alarmState == 0b00000111 )
  {
    output.data[relay] = off;
    output.data[led] = on;
    output.data[buzzer] = on;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(100);
    output.data[led] = off;
    output.data[buzzer] = off;
    outRun();
    customKey = kypad.getKey();
    if (customKey)
    {
      Serial.println(customKey);
      if (customKey == 'D')
        alarmBuzzer = 1;
      else
        stateKeypad = customKey;
      lcd.clear();
      goto out;
    }
    delay(100);
  }
  else
  {
    output.data[led] = off;
    output.data[relay] = on;
    output.data[buzzer] = off;
    outRun();
  }
out:
  output.data[relay] = output.data[relay];
}

