// Winston Price 
// Jack Michael Hanna

#include <Wire.h>     // I2C library
#include <VL6180X.h>  // distance sensor library
#include <PCA95x5.h>  // I2C to GPIO library

#define SCALING 1  // Set scaling to 1 for the calibration process

VL6180X sensor;      // distance sensor object
PCA9535 muxU31;      // MUX object for U31 PCA9535

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("Starting...");
  Wire.begin();

  // I2C to GPIO chip setup
  muxU31.attach(Wire, 0x20);
  muxU31.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
  muxU31.direction(0x1CFF);  // 1 is input, see schematic to get upper and lower bytes
  muxU31.write(PCA95x5::Port::P11, PCA95x5::Level::H);  // enable VL6180 distance sensor

  // Distance sensor setup
  sensor.init();
  sensor.configureDefault();
  sensor.setScaling(SCALING);
  sensor.setTimeout(100);
}

unsigned long lastTime = 0;

void loop()
{
  unsigned long curTime = millis();
  unsigned long dt = curTime - lastTime;
  
  // Read sensor value
  uint8_t sensorValue = sensor.readRangeSingleMillimeters();
  
  // Print sensor value and actual distance
  Serial.print("Sensor reading: ");
  Serial.print(sensorValue);
  
  if (sensor.timeoutOccurred()) { 
    Serial.print("TIMEOUT");
  }

  Serial.println();
  lastTime = curTime;

  delay(1000);  // Delay to make it easier to take readings
}
