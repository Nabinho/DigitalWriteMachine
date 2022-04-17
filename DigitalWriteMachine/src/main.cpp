/*******************************************************************************
  Digital Write Machine
  (v1.0 - 17/04/2022)
  Created by: @Nabinho
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version (<https://www.gnu.org/licenses/>).
*******************************************************************************/

//------------------------------- Global Variables -------------------------

// Board Selection Verification
#if !defined(ARDUINO_AVR_A_STAR_32U4) // Pololu A Star Micro
#error Use this code with Pololu AStar Micro
#endif

// Libraries
#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_VL53L0X.h>
#include <LiquidCrystal_I2C.h>

// Uncomment to disable DEBUG
#define DEBUG

// LCD Variables and object
#define LCD_ADDR 0x3F
#define N_COLUMNS 16
#define N_ROWS 2
LiquidCrystal_I2C lcd(LCD_ADDR, N_COLUMNS, N_ROWS);

// QWERTY KeyBoard Module Variables
#define CARDKB_ADDR 0x5F
char reading;
String letter;
String readingHEX;
int index;
int index_name;

// VL53L0X sensor object
Adafruit_VL53L0X LOX_SENSOR = Adafruit_VL53L0X();

// Pins connected to double relay module
const int RELAYS[2] = {7, 8};
const int RELAYS_SIZE = sizeof(RELAYS) / sizeof(RELAYS[0]);

// Pin connected to Buzzer
const int BUZZER = 10;

// Pin connected to touch capacite sensor
const int TOUCH = 6;

// Pin connected to OpenLog reset
const int RESET = 4;

// Variables for OpenLog comands
char buff[50];

// Variables to store the text writen
String LineOne = "";
String LineTwo = "";
String FileName = "";

// Variable to control file name editing
bool name = true;
bool leave = false;
bool save = false;
bool left = false;

//------------------------------- M5Stack CardKB Module Handler -------------------------

void read_CardKB()
{
  Wire.requestFrom(CARDKB_ADDR, 1);
  while (Wire.available())
  {
    reading = Wire.read();
    if (reading != 0)
    {
      readingHEX = String(reading, HEX);
#ifdef DEBUG
      Serial.print("CardKB Reading = ");
      Serial.println(reading);
      Serial.print("CardKB Reading in HEX = ");
      Serial.println(readingHEX);
#endif
      lcd.clear();
      if (name)
      {
        lcd.print("WRITE FILE NAME:");
      }
      else if (leave)
      {
        lcd.print("WANT TO leave FILE?");
        lcd.setCursor(0, 1);
        lcd.print("   PRESS Y OR N   ");
      }
      else
      {
        lcd.print(LineOne);
      }
      lcd.setCursor(0, 1);
      if (readingHEX == "8")
      {
        if (name)
        {
          index = FileName.length();
          FileName.remove(index - 1);
          index = FileName.length();
          lcd.print(FileName + ".txt");
        }
        else
        {
          index = LineTwo.length();
          LineTwo.remove(index - 1);
          index = LineTwo.length();
          lcd.print(LineTwo);
        }
#ifdef DEBUG
        if (name)
        {
          Serial.println("Backspace pressed");
          Serial.print("FileName - ");
          Serial.println(FileName + ".txt");
        }
        else
        {
          Serial.println("Backspace pressed");
          Serial.print("LineOne - ");
          Serial.println(LineOne);
        }
#endif
      }
      else if (readingHEX == "ff8b")
      {
        if (name)
        {
          FileName = "";
          index_name = FileName.length();
          lcd.print(FileName + ".txt");
        }
        else
        {
          LineTwo = "";
          index = LineTwo.length();
          lcd.print(LineTwo);
        }
#ifdef DEBUG
        if (name)
        {
          Serial.println("Delete pressed");
          Serial.print("FileName erased.");
          Serial.println(FileName + ".txt");
        }
        else
        {
          Serial.println("Delete pressed");
          Serial.print("Line erased.");
          Serial.println(LineTwo);
        }
#endif
      }
      else if (readingHEX == "d" || readingHEX == "a3")
      {
        if (name)
        {
          FileName += ".txt";
          name = false;
        }
        else
        {
          lcd.clear();
          lcd.print("WANT TO SAVE FILE?");
          lcd.setCursor(0, 1);
          lcd.print("   PRESS Y OR N   ");
          save = true;
        }
#ifdef DEBUG
        if (name)
        {
          Serial.println("Enter pressed - Saving name file and entering it...");
        }
        else
        {
          Serial.println("Enter pressed - Saving written file...");
        }

#endif
      }
      else if (readingHEX == "1b" || readingHEX == "ff80")
      {
#ifdef DEBUG
        Serial.println("Esc pressed - leaving editing request mode...");
#endif
        lcd.clear();
        lcd.print("WANT TO EXIT FILE?");
        lcd.setCursor(0, 1);
        lcd.print("   PRESS Y OR N   ");
        leave = true;
      }
      else if (index > 18)
      {
        LineTwo.remove(index - 1);
        index = LineTwo.length();
        lcd.print(LineTwo);
#ifdef DEBUG
        Serial.print("Text too long - ignoring last character pressed - ");
        Serial.println(LineTwo);
#endif
      }
      else if (leave)
      {
        if (reading == 'y' || reading == 'Y')
        {
          lcd.clear();
          lcd.print("EXITING  FILE...");
          LineOne = "";
          LineTwo = "";
          Serial1.write(26);
          delay(100);
          Serial1.println("rm " + FileName);
          delay(500);
          lcd.setCursor(0, 1);
          lcd.print("      DONE      ");
          delay(500);
          lcd.clear();
          leave = false;
          left = true;
        }
        else if (reading == 'n' || reading == 'N')
        {
          lcd.clear();
          lcd.print("CANCELING EXIT..");
          leave = false;
          delay(500);
          lcd.setCursor(0, 1);
          lcd.print("      DONE      ");
          delay(500);
          lcd.clear();
          lcd.print(LineOne);
          lcd.setCursor(0, 1);
          lcd.print(LineTwo);
        }
      }
      else if (save)
      {
        if (reading == 'y' || reading == 'Y')
        {
          lcd.clear();
          lcd.print("SAVING   FILE...");
          LineOne = "";
          LineTwo = "";
          Serial1.write(26);
          delay(100);
          lcd.setCursor(0, 1);
          lcd.print("      DONE      ");
          delay(500);
          lcd.clear();
          save = false;
          left = true;
        }
        else if (reading == 'n' || reading == 'N')
        {
          lcd.clear();
          lcd.print("CANCELING SAVE..");
          save = false;
          delay(500);
          lcd.setCursor(0, 1);
          lcd.print("      DONE      ");
          delay(500);
          lcd.clear();
          lcd.print(LineOne);
          lcd.setCursor(0, 1);
          lcd.print(LineTwo);
        }
      }
      else if (index_name > 8)
      {
        FileName.remove(index - 1);
        index_name = FileName.length();
        lcd.print(FileName + ".txt");
#ifdef DEBUG
        Serial.print("Name too long - ignoring last character pressed - ");
        Serial.println(FileName + ".txt");
#endif
      }
      else
      {
        letter = String(reading);
        digitalWrite(RELAYS[0], LOW);
        digitalWrite(RELAYS[1], LOW);
        delay(25);
        digitalWrite(RELAYS[0], HIGH);
        digitalWrite(RELAYS[1], HIGH);
        if (name)
        {
          FileName += letter;
          index_name = FileName.length();
          lcd.print(FileName + ".txt");
        }
        else
        {
          LineTwo += letter;
          index = LineTwo.length();
          lcd.print(LineTwo);
        }
#ifdef DEBUG
        if (name)
        {
          Serial.print("FileName - ");
          Serial.println(FileName + ".txt");
        }
        else
        {
          Serial.print("LineTwo - ");
          Serial.println(LineTwo);
        }
#endif
      }
    }
  }
}

//------------------------------- VL53L0X Distance Sensor Handler -------------------------

void check_NewLine()
{
  VL53L0X_RangingMeasurementData_t measure;

  Serial.print("Reading a measurement... ");
  LOX_SENSOR.rangingTest(&measure, false);

  if (measure.RangeStatus != 4)
  {
    float distance = measure.RangeMilliMeter;
#ifdef DEBUG
    Serial.print("Distance (mm): ");
    Serial.println(distance);
#endif
    while (distance < 40)
    {
      distance = measure.RangeMilliMeter;
      for (int i = 0; i < RELAYS_SIZE; i++)
      {
        digitalWrite(RELAYS[i], LOW);
        delay(25);
        digitalWrite(RELAYS[i], HIGH);
        delay(25);
      }
      if (digitalRead(TOUCH) == 1)
      {
        for (int i = 0; i < RELAYS_SIZE; i++)
        {
          digitalWrite(RELAYS[i], HIGH);
        }
        digitalWrite(BUZZER, HIGH);
        delay(50);
        digitalWrite(BUZZER, LOW);
        Serial1.println(LineOne);
        LineOne = LineTwo;
        LineTwo = "";
        lcd.clear();
        lcd.print(LineOne);
        lcd.setCursor(0, 1);
        lcd.print(LineTwo);
        delay(500);
        break;
      }
    }
  }
  delay(100);
}

//------------------------------- Code Configuration -------------------------

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("<-- Nabinho's Digital Write Machine -->");
  Serial.println("<-- Starting Software -->");
#endif

  // IHM configuration
  lcd.init();
  lcd.backlight();
  lcd.print("    WELCOME!    ");
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print("  STARTING....  ");
  delay(500);

  // Starting Distance Sensor
  LOX_SENSOR.begin();

  // Configure touch sensor as a input
  pinMode(TOUCH, INPUT);

  // Configure buzzer as an output
  pinMode(BUZZER, OUTPUT);

  // Configure relays as outputs and started off
  for (int i = 0; i < RELAYS_SIZE; i++)
  {
    pinMode(RELAYS[i], OUTPUT);
    digitalWrite(RELAYS[i], HIGH);
  }

  lcd.clear();
  lcd.print("WRITE FILE NAME:");
  while (name)
  {
    read_CardKB();
  }

  // OpenLog Reset
  delay(500);
  digitalWrite(RESET, LOW);
  delay(100);
  digitalWrite(RESET, HIGH);
  delay(100);
  Serial1.begin(9600);
  delay(500);

  // OpenLog Command Mode Stratup
  Serial1.write(26);
  Serial1.write(26);
  Serial1.write(26);
  delay(100);

  lcd.clear();
  lcd.print("CREATING FILE...");

  // Creating File to save text
  delay(500);
  Serial1.println("new " + FileName);
  delay(100);

  lcd.setCursor(0, 1);
  lcd.print("      DONE      ");
  delay(500);

  lcd.clear();
  lcd.print("ENTERING FILE...");

  // Entering New File
  delay(500);
  Serial1.println("append " + FileName);
  delay(100);

  lcd.setCursor(0, 1);
  lcd.print("      DONE      ");
  delay(500);

  lcd.clear();
  lcd.print("FINISHING  START");
  delay(500);

  lcd.clear();
  lcd.print("SYSTEM  STARTED!");
  delay(500);
  lcd.clear();
  delay(1000);
}

//------------------------------- Code Repetition -------------------------

void loop()
{
  if (!left)
  {
    read_CardKB();
    check_NewLine();
  }
  else
  {
    lcd.clear();
    lcd.print("LEFT  EDITING...");
    lcd.setCursor(0, 1);
    lcd.print("PLEASE  RESET...");
  }
}