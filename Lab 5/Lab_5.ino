#include "OneWireNg_CurrentPlatform.h"
#include <OneWire.h>
#include <Anitracks_ADS7142.h>
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <PCA95x5.h>
#include <QMC5883LCompass.h>
#include <Arduino_LSM6DSOX.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>  // Include the TFT library

#define ADC_ADDR1 0x1F  // ADS7142 I2C address for light1 & light2
#define ADC_ADDR2 0x18  // ADS7142 I2C address for temp & pot

#define TFT_CS   4  // display chip select
#define TFT_RST  A3 // display reset
#define TFT_DC   3  // display D/C
#define SD_CS    7 // Not 100% sure if I need this or not
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Initialization of sensors and ADCs 
PCA9535 muxU31;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
QMC5883LCompass compass;
ADS7142 adc1(ADC_ADDR1); // Create ADS7142 object for lights
ADS7142 adc2(ADC_ADDR2); // Create ADS7142 object for temp & pot
OneWire ds(10);

double maxADCPot = 65535.0; // Maximum pot adc value - 16-bit adc
double refVolt = 3.3;       // Reference voltage for ADC

void setup() {
  // Startup   
  Serial.begin(115200);
  delay(3000);  
  Serial.println("Starting...");

  // Initialize sensors and ADCs
  adc1.begin();
  adc2.begin();
  
  Wire.begin();
  muxU31.attach(Wire, 0x20); 
  muxU31.polarity(PCA95x5::Polarity::ORIGINAL_ALL); 
  muxU31.direction(0xFFFC); 

  // upper and lower bytes
  muxU31.write(PCA95x5::Port::P10, PCA95x5::Level::L);  // Disable SHT31
  delay(100);
  muxU31.write(PCA95x5::Port::P10, PCA95x5::Level::H);  // Enable SHT31
  delay(100);
  
  if (!sht31.begin(0x44)) {
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  compass.init();

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // TFT Screen Configuration and setup
  tft.init(135, 240);

  tft.fillScreen(0x0000); // Black background
  tft.setRotation(1);  // Adjust orientation
  tft.setTextColor(0xFFFF);  // White text
  tft.setTextSize(2);
  tft.setCursor(0, 20);
  tft.println("Starting program...");
  delay(3000); // Show message for 3 seconds
  tft.fillScreen(0x0000); // Clear screen

  Serial.println("Setup Complete!");
}



void loop() {
    
  // 1. Output time from millis()
  print_millis();
  
  // 2. Light intensities from ADS7142
  lightIntensity();

  // 3. Potentiometer value as a voltage (0 to 3.3V)
  potentiometerVoltage();

  // 4. Temperature from TEMP0
  temp_from_TEMP0();

  // 5. SHT31 Temperature and Humidity
  temp_from_SHT31();

  // 6. Compass azimuth
  read_compass();

  // 7. IMU Accelerometer and Gyroscope Data
  IMU_data();
  
  // 8. 1-Wire DS18B20 temperature
  temp_from_1wire();
}


void print_millis() {
  unsigned long current_time = millis();
  Serial.println("Time (ms): " + String(current_time));
  tft.println("Time (ms): " + String(current_time));
}

void lightIntensity() {
  uint16_t adcLight1, adcLight2;
  if (!adc1.read2Ch(&adcLight1, &adcLight2)) {
    Serial.println("Failed to read light sensors");
  } else {
    // Convert from 16-bit to 10-bit (0-1023)
    Serial.println("Light 1: " + adcLight1 >> 6);
    Serial.println("Light 2: " + adcLoght >> 6);
  }

  tft.println("Light 1: " + String(adcLight1 >> 6));
  tft.println("Light 2: " + String(adcLight2 >> 6));
}

void potentiometerVoltage() {
  uint16_t adcPot1;
  if (!adc2.read2Ch(NULL, &adcPot1)) {
    Serial.println("Failed to read potentiometer");
  } else {
    float potVoltage = ((float)adcPot1 / maxADCPot) * refVolt;
    Serial.println("Potentiometer Voltage: " + potVoltage);
  }

  tft.println("Pot Voltage: " + String(((float)adcPot1 / maxADCPot) * refVolt));
  
}

void temp_from_TEMP0() {
  uint16_t adcTemp1;
  if (!adc2.read2Ch(&adcTemp1, NULL)) {
    Serial.println("Failed to read temperature sensor");
  } else {
    float tempF = (float)adcTemp1 / maxADCPot * refVolt * 100;  // Conversion example
    Serial.println("Temp0 (F): " + tempF);
  }

  tft.println("Temp0 (F): " + String((float)adcTemp1 / maxADCPot * refVolt * 100));

  

}

void temp_from_SHT31() {
  float sht31TempF = sht31.readTemperature() * 1.8 + 32.0; // read temp and convert to F
  float humidity = sht31.readHumidity(); // Grab the humidity
  
  if (!isnan(sht31TempF)) {
    Serial.println("SHT31 Temp (F): " + sht31TempF);
  } else {
    Serial.println("Failed to read SHT31 temperature");
  }

  if (!isnan(humidity)) {
    Serial.println("SHT31 Humidity (%): " + humidity);
  } else {
    Serial.println("Failed to read SHT31 humidity");
  }

  tft.println("SHT31 Temp (F): " + String(sht31TempF));
  tft.println("SHT31 Humidity: " + String(humidity));
}

void read_compass() {
  compass.read();
  int azimuth = compass.getAzimuth();
  Serial.println("Compass Azimuth: " + azimuth);

  tft.println("Azimuth: " + String(azimuth));
}

void IMU_data() {
  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    Serial.print("Acceleration (g): ");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
    Serial.print(", ");
    Serial.println(z);
  }

  float xG, yG, zG;
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(xG, yG, zG);
    Serial.print("Gyroscope (deg/s): ");
    Serial.print(xG);
    Serial.print(", ");
    Serial.print(yG);
    Serial.print(", ");
    Serial.println(zG);
  }

  tft.println("Acc (g): " + String(x) + ", " + String(y) + ", " + String(z));
  tft.println("Gyro (deg/s): " + String(xG) + ", " + String(yG) + ", " + String(zG));

}

void temp_from_1wire() {
  double ds18b20Temp = Temperature();
  if (!isnan(ds18b20Temp)) {
    Serial.print("DS18B20 Temp (F): ");
    Serial.println(ds18b20Temp);
  } else {
    Serial.println("Failed to read DS18B20 temperature");
  }

  tft.println("DS18B20 Temp (F): " + String(ds18b20Temp));

}

double Temperature() {
  byte addr[8];
  byte data[9];

  if (!ds.search(addr)) {
    ds.reset_search();
    return NAN;  // No valid address found
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);  // Start temperature conversion
  delay(1000);        // Wait for conversion

  ds.reset();
  ds.select(addr);
  ds.write(0xBE);     // Read Scratchpad

  for (int i = 0; i < 9; i++) {
    data[i] = ds.read();
  }

  int16_t rawTemp = (data[1] << 8) | data[0];
  float tempC = (float)rawTemp / 16.0;
  return tempC * 1.8 + 32.0;  // Convert to Fahrenheit
}

void clear_tft() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  delay(3000);
}



