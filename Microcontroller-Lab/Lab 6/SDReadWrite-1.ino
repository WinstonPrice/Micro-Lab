/*
  SD card read/write

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

  Formatting instructions:
  https://learn.adafruit.com/adafruit-micro-sd-breakout-board-card-tutorial/formatting-notes

*/

#include <SPI.h>
#include <SD.h>

#define TFT_CS            4  // display chip select
#define SD_CS             7  // SD card chip select

File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH); // turn off TFT

  Serial.print("Initializing SD card...");

  if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE | O_TRUNC);
//  myFile = SD.open("test.txt", FILE_WRITE);
  // O_TRUNC truncates the file to zero length before writing
  // SD.remove("test.txt"); deletes the file

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.print("testing 1, 2, 3. ");
    myFile.print(__DATE__);  // Upload date
    myFile.print(" ");
    myFile.print(__TIME__);  // Upload time
    myFile.print(" ");
    myFile.println(__FILE__);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  delay(1000);
  // re-open the file for reading:
  myFile = SD.open("test.txt", FILE_READ);
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void loop() {
  // nothing happens after setup
  delay(1000);
}
