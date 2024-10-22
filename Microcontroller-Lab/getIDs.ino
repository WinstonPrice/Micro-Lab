/* getIDs.ino
 * 
 * Collects the ID from the Nano Connect RP2040's cryptography chip,
 * and the IDs off the 1-wire temperature sensors.
 * 
 * Seth McNeill
 * 2022 September 05
 * Modified to not use OneWireNG since the regular OneWire now works
 * 2024 August 20
 */

// OneWire 1-Wire library
#include <OneWire.h>

// cryptology chip libraries
#include <ArduinoECCX08.h>
#include <utility/ECCX08JWS.h>
#include <utility/ECCX08DefaultTLSConfig.h>

// the following lines and info is if you want to use a 
// different Wire than default
// must be named this for the ArduinoECCX08 library to work, 
// also must modify ECCX08.cpp to not define a variable with this name
//ECCX08Class ECCX08(Wire, (uint8_t)0x60); 

OneWire  ds(10);  // create a OneWire object attached to pin 10

void setup() {
  Serial.begin(115200);
  while(!Serial);  // wait for serial to begin

  Serial.println("\n===============================");

  // setup DS18B20 1-Wire temperature sensors
  while(!print1WireIDs()) {
    Serial.println("OneWire reading failed. Please let your instructor know.");
    Serial.println("Trying again");
    delay(1000);
  }

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }
  Serial.println("");

  Serial.println("Crypto Serial Number:");
  String serNum = ECCX08.serialNumber();
  Serial.println(serNum);

// Check if the chip is already locked. If not, give it default Arduino configuration.
  if (!ECCX08.locked()) {
    Serial.println("ECCX08 is not locked");
  }
}

void loop() {
  delay(1000);
}


bool print1WireIDs() {
  byte ii;
  byte type_s = 0;
  byte data[9];
  byte addr[8];
  float celsius, fahrenheit;
  bool retVal = false;
  byte cnt = 0;

  ds.reset_search();
  delay(100); // This seems to make it work more consistantly
  while(ds.search(addr)) {
    Serial.print("1-Wire ID ");
    Serial.print(cnt++);
    Serial.print(" =");
    for(ii = 0; ii < 8; ii++) {
      Serial.write(' ');
      Serial.print(addr[ii], HEX);
    }
    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("addr CRC is not valid!");
    }
    // the first ROM byte indicates which chip
    switch (addr[0]) {
      case 0x10:
//        Serial.println("  Chip = DS18S20");  // or old DS1820
        type_s = 1;
        break;
      case 0x28:
//        Serial.println("  Chip = DS18B20");
        type_s = 0;
        break;
      case 0x22:
//        Serial.println("  Chip = DS1822");
        type_s = 0;
        break;
      default:
        Serial.println("Device is not a DS18x20 family device.");
    } 
    // Run a temperature measurement
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);        // start conversion, with parasite power on at the end
    
    delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
    
    byte present = ds.reset();
    if(present != 1) {
      Serial.println("Not present");
    }

    ds.select(addr);    
    ds.write(0xBE);         // Read Scratchpad
    // calculate temperature from data
    for (ii = 0; ii < 9; ii++) {           // we need 9 bytes
      data[ii] = ds.read();
    }
    if (OneWire::crc8(data, 8) != data[8]) {
        Serial.println("data CRC is not valid!");
    }
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
    Serial.print("\tT = ");
    Serial.print(fahrenheit);
    Serial.println(" F");
    retVal = true;
  }
  ds.reset_search();
  return(retVal);
}
