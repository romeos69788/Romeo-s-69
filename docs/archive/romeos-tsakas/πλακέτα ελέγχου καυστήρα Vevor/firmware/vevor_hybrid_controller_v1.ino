/*
 * H.T.C. — Hybrid Thermal Controller (Arduino Nano)
 * Αποθήκευση κώδικα όπως βρέθηκε (2026-05-18).
 * Πλακέτα: πλακέτα ελέγχου καυστήρα Vevor — Rev 1.0
 */
#include <SPI.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

const int ONE_WIRE_BUS = 2;
const int CS_PIN = 10;
const int RELAY_PUMP = 4;
const int RELAY_VEVOR_BTN = 7;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
hd44780_I2Cexp lcd(0x27);

const int VEVOR_FULL = 128;
const int VEVOR_MID = 197;
const int VEVOR_IDLE = 230;
const int VEVOR_OFF = 250;

bool isHeatingCycle = true;
bool lastHeatingCycle = true;

void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  pinMode(RELAY_PUMP, OUTPUT);
  digitalWrite(RELAY_PUMP, HIGH);
  pinMode(RELAY_VEVOR_BTN, OUTPUT);
  digitalWrite(RELAY_VEVOR_BTN, HIGH);
  SPI.begin();
  sensors.begin();
  lcd.begin(20, 4);
  lcd.backlight();
  lcd.clear();
  delay(200);
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM BOOTING...");
  Serial.println(F("--- ΣΥΣΤΗΜΑ ΕΝΔΟΔΑΠΕΔΙΑΣ ΕΝΕΡΓΟ ---"));
  delay(2000);
  lcd.clear();
}

void loop() {
  sensors.requestTemperatures();
  float tempBoiler = sensors.getTempCByIndex(0);
  float tempSupply = sensors.getTempCByIndex(1);
  float tempReturn = sensors.getTempCByIndex(2);

  if (tempBoiler < 35.0) {
    isHeatingCycle = true;
  } else if (tempBoiler >= 42.0) {
    isHeatingCycle = false;
  }

  if (tempBoiler > 25.0) {
    digitalWrite(RELAY_PUMP, LOW);
  } else {
    digitalWrite(RELAY_PUMP, HIGH);
  }

  String vevorStatus = "";
  if (!isHeatingCycle) {
    setPotentiometer(VEVOR_OFF);
    vevorStatus = "OFF ";
  } else {
    if (tempBoiler < 37.0) {
      setPotentiometer(VEVOR_FULL);
      vevorStatus = "FULL";
    } else if (tempBoiler < 40.0) {
      setPotentiometer(VEVOR_MID);
      vevorStatus = "MID ";
    } else if (tempBoiler < 42.0) {
      setPotentiometer(VEVOR_IDLE);
      vevorStatus = "IDLE";
    }
  }

  if (isHeatingCycle != lastHeatingCycle) {
    lcd.setCursor(0, 3);
    lcd.print("Katastasi: BTN PRESS");
    Serial.println(F(">>> ΕΝΤΟΛΗ ΣΤΟ ΡΕΛΕ VEVOR (ΠΑΤΗΜΑ 2.5 ΔΕΥΤ.) <<<"));
    toggleVevorPower();
    lastHeatingCycle = isHeatingCycle;
  }

  lcd.setCursor(0, 0);
  lcd.print("Boiler:    ");
  lcd.print(tempBoiler, 1);
  lcd.print(" C  ");
  lcd.setCursor(0, 1);
  lcd.print("Prosagogi: ");
  lcd.print(tempSupply, 1);
  lcd.print(" C  ");
  lcd.setCursor(0, 2);
  lcd.print("Epistrofi: ");
  lcd.print(tempReturn, 1);
  lcd.print(" C  ");
  if (vevorStatus != "") {
    lcd.setCursor(0, 3);
    lcd.print("Katastasi: ");
    lcd.print(vevorStatus);
    lcd.print("    ");
  }

  Serial.print(F("B: "));
  Serial.print(tempBoiler, 1);
  Serial.print(F(" | P: "));
  Serial.print(tempSupply, 1);
  Serial.print(F(" | E: "));
  Serial.print(tempReturn, 1);
  Serial.print(F(" | Katastasi: "));
  Serial.println(vevorStatus);
  delay(1000);
}

void toggleVevorPower() {
  digitalWrite(RELAY_VEVOR_BTN, LOW);
  delay(2500);
  digitalWrite(RELAY_VEVOR_BTN, HIGH);
}

void setPotentiometer(int value) {
  if (value > 255) {
    value = 255;
  }
  if (value < 0) {
    value = 0;
  }
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x11);
  SPI.transfer(value);
  digitalWrite(CS_PIN, HIGH);
}
