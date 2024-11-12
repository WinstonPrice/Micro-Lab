/* sd-time.ino
 *  
 *  This script sets the RTC from an NTP server and then 
 *  logs data using the actual date/time.
 *  
 *  Seth McNeill
 *  2022 March 10
 */

#include "WiFiNINA.h"  // for A4-A7 and wifi/bluetooth
#include <PCA95x5.h>           // MUX library for buttons
#include "Anitracks_ADS7142.h"  // ADC library
#include <SPI.h>
#include <SD.h>
#include "defines.h"
// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include <Timezone_Generic.h>             // https://github.com/khoih-prog/Timezone_Generic

#define RIGHT_BUTTON_PIN   A0
#define BUZZ_PIN           2
#define SERVO_PIN          9
#define NEO_PIN           17  // WARNING! THIS IS GPIO NOT D NUMBER for NeoPixels
#define NEO_COUNT         18  // number of NeoPixels
#define TFT_CS            4  // display chip select
#define TFT_RST           A3 // display reset
#define TFT_DC            3  // display D/C
#define SD_CS             7

File myFile;
TimeChangeRule myDST = {"MST", Second, Sun, Mar, 2, -420};    //Daylight time = UTC - 7 hours
TimeChangeRule mySTD = {"MST", First, Sun, Nov, 2, -420};     //Standard time = UTC - 7 hours
Timezone *myTZ;
TimeChangeRule *tcr;        //pointer to the time change rule, use to get TZ abbrev
int status = WL_IDLE_STATUS;      // the Wifi radio's status
char timeServer[]         = "time.nist.gov";  // NTP server
unsigned int localPort    = 2390;             // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48;       // NTP timestamp is in the first 48 bytes of the message
const int UDP_TIMEOUT     = 2000;     // timeout in miliseconds to wait for an UDP packet to arrive
byte packetBuffer[NTP_PACKET_SIZE];   // buffer to hold incoming and outgoing packets
datetime_t  currTime;
char        datetime_buf[256];

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
PCA9535 muxU31;      // MUX object for U31 PCA9535
ADS7142 adc2(0x18);  // POT and TEMP

void setup() {
  Serial.begin(115200);
  //while(!Serial) delay(100);
  delay(2000);
  Serial.println("Starting...");
  
  Wire.begin();
  muxU31.attach(Wire, 0x20);
  muxU31.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
  muxU31.direction(0x1CFF);  // 1 is input, see schematic to get upper and lower bytes

  pinMode(RIGHT_BUTTON_PIN, INPUT);
  pinMode(BUZZ_PIN, OUTPUT);
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);

  Serial.print("Initializing SD card...");
  // THIS LINE SETS YOUR SKETCH TO SAVE YOUR
  // TIME AND DATE INTO ANY FILE YOU CREATE.
  SdFile::dateTimeCallback(dateTime);
  // see if the card is present and can be initialized:
  if (!SD.begin(SD_CS)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  delay(200);
  
  adc2.begin();

  Serial.print(F("\nStart RP2040_RTC_Time_WiFiNINA on ")); Serial.print(BOARD_NAME);
  Serial.print(F(" with ")); Serial.println(SHIELD_TYPE);
  Serial.println(RP2040_RTC_VERSION);
  Serial.println(TIMEZONE_GENERIC_VERSION);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println(F("WiFi shield not present"));
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println(F("Please upgrade the firmware"));
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED)
  {
    Serial.print(F("Connecting to WPA SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid);
  }

  // you're connected now, so print out the data
  Serial.print(F("You're connected to the network, IP = "));
  Serial.println(WiFi.localIP());

  myTZ = new Timezone(myDST, mySTD);

  // Start the RTC
  rtc_init();

  Udp.begin(localPort);

  getNTPTime();

  displayRTCTime();

}

int isLogging = false;
bool firstPass = true;  // for first time through -- write headers
int logdT = 1000; // time between log points in milliseconds
long int lastLogTime = 0;
int leftButtonState;
int leftLastButtonState = 1;
int rightButtonState;
int rightLastButtonState = 1;
String baseName = "data";
String ext = ".csv";
int lastFileNum = 0;
String fname = "";  // should get changed by the code

void loop() {
  uint16_t temp0;
  uint16_t potVal;
  uint16_t mux1in = muxU31.read();
  leftButtonState = mux1in & 0x01;
  rightButtonState = digitalRead(RIGHT_BUTTON_PIN);

  if(!adc2.read2Ch(&temp0, &potVal)) {
    Serial.println("read2CH failed");
  }
  // check for button state changes to prevent repeatedly doing the action
  if(leftButtonState != leftLastButtonState && leftButtonState == 0) {
    fname = findFname();
    Serial.print("Now writing to: ");
    Serial.println(fname);
    File dataFile = SD.open(fname, FILE_WRITE);  
    dataFile.print("millis,Light,Potentiometer,Battery\n");
    dataFile.close();
  }
  // toggle logging
  if(rightButtonState != rightLastButtonState && rightButtonState == 0) {
    if(fname != "") {
      isLogging = !isLogging;
      if(isLogging) {
        Serial.println("Logging start");
      } else {
        Serial.println("Logging stop");
      }
    } else {
      Serial.println("Please press Left button to initialize");
    }
  }
  if(isLogging && (millis() - lastLogTime > logdT)) {
    File dataFile = SD.open(fname, FILE_WRITE);  
    dataFile.print(millis());
    dataFile.print(',');
    dataFile.print(temp0);
    dataFile.print(',');
    dataFile.println(potVal);
    lastLogTime = millis();
    dataFile.close();
  }
  displayRTCTime();

  leftLastButtonState = leftButtonState;
  rightLastButtonState = rightButtonState;
  delay(10);
}

void displayTime()
{
  rtc_get_datetime(&currTime);

  // Display time from RTC
  DateTime now = DateTime(currTime); 

  time_t utc = now.get_time_t();
  time_t local = myTZ->toLocal(utc, &tcr);

  printDateTime(utc, "UTC");
  printDateTime(local, tcr -> abbrev);
}

void displayRTCTime()
{
  static unsigned long displayRTCTime_timeout = 0;

#define DISPLAY_RTC_INTERVAL     10000L

  // Send status report every STATUS_REPORT_INTERVAL (60) seconds: we don't need to display frequently.
  if ((millis() > displayRTCTime_timeout) || (displayRTCTime_timeout == 0))
  {
    Serial.println("============================");
    displayTime();

    displayRTCTime_timeout = millis() + DISPLAY_RTC_INTERVAL;
  }
}

// send an NTP request to the time server at the given address
void sendNTPpacket(char *ntpSrv)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)

  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(ntpSrv, 123); //NTP requests are to port 123

  Udp.write(packetBuffer, NTP_PACKET_SIZE);

  Udp.endPacket();
}

void getNTPTime()
{
  static bool gotCurrentTime = false;

  // Just get the correct ime once
  if (!gotCurrentTime)
  {
    sendNTPpacket(timeServer); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(1000);

    if (Udp.parsePacket())
    {
      Serial.println(F("Packet received"));
      // We've received a packet, read the data from it
      Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      Serial.print(F("Seconds since Jan 1 1900 = "));
      Serial.println(secsSince1900);

      // now convert NTP time into everyday time:
      Serial.print(F("Unix time = "));
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;

      // print Unix time:
      Serial.println(epoch);
      
      // Get the time_t from epoch
      time_t epoch_t = epoch;

      // set the system time to UTC
      // warning: assumes that compileTime() returns US EDT
      // adjust the following line accordingly if you're in another time zone
      setTime(epoch_t);

      // Update RTC
      // Can use either one of these functions
      
      // 1) DateTime(tmElements_t). Must create tmElements_t if not present
      //tmElements_t tm;
      //breakTime(epoch_t, tm);
      //rtc_set_datetime( DateTime(tm) );
      
      // 2) DateTime(year, month, day, hour, min, sec)
      //rtc_set_datetime( DateTime(year(epoch_t), month(epoch_t), day(epoch_t), hour(epoch_t), minute(epoch_t), second(epoch_t) ) );

      // 3) DateTime (time_t)
      //rtc_set_datetime( DateTime(epoch_t) );

      // 4) DateTime(unsigned long epoch). The best and easiest way
      //rtc_set_datetime( DateTime((uint32_t) epoch) );

      // New function in DateTime_Generic.h

      // To be called before while loop to work. Why ???
      rtc_set_datetime(DateTime((uint32_t) epoch));

      uint8_t loopCount = 0;

      while( (loopCount++ < 10 ) && ( ! rtc_set_datetime(DateTime((uint32_t) epoch)) ) )
      {
        Serial.println(F("rtc_set_datetime failed"));
        sleep_ms(500);
      }
      
      // print the hour, minute and second:
      Serial.print(F("The UTC time is "));        // UTC is the time at Greenwich Meridian (GMT)
      Serial.print((epoch  % 86400L) / 3600);     // print the hour (86400 equals secs per day)
      Serial.print(':');

      if (((epoch % 3600) / 60) < 10)
      {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
      Serial.print(':');

      if ((epoch % 60) < 10)
      {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.println(epoch % 60); // print the second

      gotCurrentTime = true;
    }
    else
    {
      // wait ten seconds before asking for the time again
      delay(10000);
    }
  }
}

//////////////////////////////////////////

// format and print a time_t value, with a time zone appended.
void printDateTime(time_t t, const char *tz)
{
  char buf[32];
  char m[4];    // temporary storage for month string (DateStrings.cpp uses shared buffer)
  strcpy(m, monthShortStr(month(t)));
  sprintf(buf, "%.2d:%.2d:%.2d %s %.2d %s %d %s",
          hour(t), minute(t), second(t), dayShortStr(weekday(t)), day(t), m, year(t), tz);
  Serial.println(buf);
}

// callback for SD card write times
void dateTime(uint16_t* date, uint16_t* time)
{
  rtc_get_datetime(&currTime);

  // Display time from RTC
  DateTime now = DateTime(currTime); 

  time_t utc = now.get_time_t();
  time_t local = myTZ->toLocal(utc, &tcr);
  
  *date = FAT_DATE(year(local), month(local), day(local));
  *time = FAT_TIME(hour(local), minute(local), second(local));
}

void dateTimeStr(time_t t, const char *tz)
{
  char buf[32];
  char m[4];    // temporary storage for month string (DateStrings.cpp uses shared buffer)
  strcpy(m, monthShortStr(month(t)));
  sprintf(buf, "%.2d:%.2d:%.2d %s %.2d %s %d %s",
          hour(t), minute(t), second(t), dayShortStr(weekday(t)), day(t), m, year(t), tz);
  Serial.println(buf);
}

String findFname() {
  String fname = baseName + lastFileNum + ext;
  while(SD.exists(fname)) {
    fname = baseName + ++lastFileNum + ext;
  }
  return(fname);
}
