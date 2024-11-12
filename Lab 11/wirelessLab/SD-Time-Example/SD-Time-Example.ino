#include <SD.h>

File file;
const uint8_t CS = 10; // SD chip select

    // YOUR SKETCH SHOULD UPDATE THESE SIX VALUSE
    // EACH TIME BEFORE CREATING YOUR SD CARD FILE
unsigned int year = 2015;
byte month = 6;      
byte day = 18;
byte hour = 7;
byte minute = 8;
byte second = 9;

void dateTime(uint16_t* date, uint16_t* time)
{
  *date = FAT_DATE(year, month, day);
  *time = FAT_TIME(hour, minute, second);
}

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("Starting...");
  
      // THIS LINE SETS YOUR SKETCH TO SAVE YOUR
      // TIME AND DATE INTO ANY FILE YOU CREATE.
  SdFile::dateTimeCallback(dateTime);
  
  if (!SD.begin(CS))
  {
    Serial.println("SD.begin failed");
    while(1);
  }
  file = SD.open("datetest.txt", FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file");
  }
  file.print(__DATE__);
  file.print(" ");
  file.print(__TIME__);
  file.close();
  Serial.println("Done");
}

void loop() {delay(100);}
