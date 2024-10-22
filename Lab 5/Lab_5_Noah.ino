
#include "OneWireNg_CurrentPlatform.h"
#include <OneWire.h>
#include <Anitracks_ADS7142.h>
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <PCA95x5.h>
#include <QMC5883LCompass.h>
#include <Arduino_LSM6DSOX.h>

#define ADC_ADDR1 0x1F  // ADS7142 I2C address for light1 & light2
#define ADC_ADDR2 0x18  // ADS7142 I2C address for temp & pot 

PCA9535 muxU31;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

QMC5883LCompass compass;

ADS7142 adc1(ADC_ADDR1); // Create ADS7142 object for lights
ADS7142 adc2(ADC_ADDR2); // Creat ADS7142 object for temp & pot

OneWire  ds(10);

void setup() {
  Serial.begin(115200);
  // Wait for serial to begin on some processors (RP2040)
  delay(3000);  
  Serial.println("Starting...");

  adc1.begin();
  adc2.begin();

  Wire.begin();
  muxU31.attach(Wire, 0x20); 
  muxU31.polarity(PCA95x5::Polarity::ORIGINAL_ALL); 
  muxU31.direction(0x1CFF); // 1 is input, see
  // schematic to get upper and lower bytes
  muxU31.write(PCA95x5::Port::P10, PCA95x5::Level::L);
  // disable SHT31
  delay(100);
  muxU31.write(PCA95x5::Port::P10, PCA95x5::Level::H);
  // enable SHT31
  delay(100);
  
  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) { // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn’t find SHT31");
    while (1) delay(1);
  }  

  compass.init();

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");

    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Acceleration in g's");
  Serial.println("X\tY\tZ");

  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Gyroscope in degrees/second");
  Serial.println("X\tY\tZ");
}  

String dispStr;
int loopDelay = 5000; // mseconds
double maxADCPot = 65535.0; // maximum pot adc value - just maximum 16-bit adc value
double refVolt = 3.3; // reference voltage
int a;
float x, y, z;

void loop() {

  /*
  String dispStr;
  dispStr = "T(F): ";
  dispStr += String(tF,1);
  dispStr += "\n";
  // Control the display
  tft.fillScreen(ST77XX BLACK); tft.setTextColor(ST77XX YELLOW); 
  // set text color 

  tft.setTextSize(1); 
  // Normal 1:1 pixel scale 

  tft.setCursor(0,0); 
  // Start at top−left corner 

  tft.println(dispStr);
  */
  uint16_t adcLight1;
  uint16_t adcLight2;
  uint16_t adcTemp1;
  uint16_t adcPot1;

  // adcIn1 and adcIn2 are passed by 
  // reference (&adcIn1) so that read2CH
  // can modify them. This is one way to
  // return multiple values from a function.
  if(!adc1.read2Ch(&adcLight1, &adcLight2)) {
    Serial.println("adc1 failed");
  }

  if(!adc2.read2Ch(&adcTemp1, &adcPot1)) {
    Serial.println("adc2 failed");
  }
  // NOTE: The return values are 16-bit numbers
  // created by shifting the 12-bit ADC result
  // left by 4-bits. This becomes clearer if you
  // print the output in HEX and note the last
  // digit is always zero.
  
  adcTemp1 = (float)adcTemp1/maxADCPot*refVolt;
  adcTemp1 = (adcTemp1-0.1)/0.01-40;
  
  // Prints out the light readings from the ADS7142.
  Serial.print("light intensity:");
  Serial.print(6>>adcLight1); // light intensity 1
  Serial.print(", ");
  Serial.println(6>>adcLight2); // light intensity 2
  Serial.print("Temp: ");
  Serial.println(adcTemp1);
  Serial.print("Pot: ");
  Serial.println(((float)adcPot1/maxADCPot)*3.3);

  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  t = t*1.8+32.0;

  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Failed to read humidity");
  }

   // Read compass values
  compass.read();

  // Return Azimuth reading
  a = compass.getAzimuth();
  
  Serial.print("A: ");
  Serial.println(a);

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    Serial.print("a (m/s^2): ");
    Serial.print(x*9.81);
    Serial.print('\t');
    Serial.print(y*9.81);
    Serial.print('\t');
    Serial.println(z*9.81);
  }

  float xG, yG, zG;

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(xG, yG, zG);

    Serial.print("gyro (deg/s): ");
    Serial.print(xG);
    Serial.print('\t');
    Serial.print(yG);
    Serial.print('\t');
    Serial.println(zG);

  Serial.print("Temp (fahrenheit): ");
  Serial.println(oneWireTemp());
  
  delay(loopDelay);
  }
}
  
 

double oneWireTemp() {
   byte i;
  byte present = 0;
  byte type_s;
  byte data[9];
  byte addr[8];
  float celsius, fahrenheit;
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  return fahrenheit;
}
