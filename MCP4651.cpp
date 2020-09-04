#include "MCP4651.h"
#include <Arduino.h>
#include <Wire.h>

void MCP4651::pot0_set_Value (int pot_address, int value) {
  Wire.beginTransmission(pot_address);
  Wire.write(0x00);
  Wire.write(value);
  Wire.endTransmission(pot_address);
}

void MCP4651::pot1_set_Value (int pot_address, int value) {
  Wire.beginTransmission(pot_address);
  Wire.write(0x010);
  Wire.write(value);
  Wire.endTransmission(pot_address);
}

void MCP4651::pot0_inc (int pot_address) {
  Wire.beginTransmission(pot_address);
  Wire.write(0x04);
  Wire.endTransmission(pot_address);
}

void MCP4651::pot1_inc (int pot_address) {
  Wire.beginTransmission(pot_address);
  Wire.write(0x14);
  Wire.endTransmission(pot_address);
}

void MCP4651::pot0_dec (int pot_address) {
  Wire.beginTransmission(pot_address);
  Wire.write(0x08);
  Wire.endTransmission(pot_address);
}

void MCP4651::pot1_dec (int pot_address) {
  Wire.beginTransmission(pot_address);
  Wire.write(0x18);
  Wire.endTransmission(pot_address);
}

int MCP4651::pot0_read (int pot_address) {
  int data = 0;
  Wire.beginTransmission(pot_address);
  Wire.write(0x0C);
  Wire.endTransmission(pot_address);
  Wire.requestFrom(pot_address,2);
  unsigned long time = millis();
  while (!Wire.available()) if ((millis() - time)>500) return 0x00;
  while (Wire.available()) data = Wire.read();
  return data;
}

int MCP4651::pot1_read (int pot_address) {
  int data = 0;
  Wire.beginTransmission(pot_address);
  Wire.write(0x1C);
  Wire.endTransmission(pot_address);
  Wire.requestFrom(pot_address,2);
  unsigned long time = millis();
  while (!Wire.available()) if ((millis() - time)>500) return 0x00;
  while (Wire.available()) data = Wire.read();
  return data;
}
