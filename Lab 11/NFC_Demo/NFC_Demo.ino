/*
 * NFC_Demo
 * 
 * This is a demo of using the ST25DV NFC chip.
 * 
 * Seth McNeill
 * 2022 November 17
 * 
 * This code is in the public domain.
 */


#include "ST25DVSensor.h"      // for the NFC chip
#include <WiFiNINA.h>          // for WiFi connectivity

#define RIGHT_BUTTON_PIN   A0
#define BUZZ_PIN           2

// NFC setup
const int NFC_GPO_PIN = 8;    // Not used but required for setup
const int NFC_LPD_PIN = A1;   // Not used but required for setup
const int nNFC = 4;           // number of NFC messages

// NFC messages
String NFC_protocols[nNFC] = {"mailto:", "tel:", "sms:", "http://"};
String NFC_messages[nNFC] = {"pricew7@my.erau.edu",
                             "9287773894",
                             "3048372463",
                             "erau.edu"};

// WiFi credentials
const char SSID[] = "EagleNet";  // your network SSID (name)
int status = WL_IDLE_STATUS;     // the WiFi radio's status

ST25DV st25dv(NFC_GPO_PIN, -1, &Wire);

void setup() {
  Serial.begin(115200);
  delay(3000);
  
  pinMode(RIGHT_BUTTON_PIN, INPUT);
  pinMode(BUZZ_PIN, OUTPUT);

  // Initialize NFC
  if (st25dv.begin() != 0) {
    Serial.println("Failed to start NFC ST25");
    while (1);
  } else {
    Serial.println("NFC ST25 started");
  }

  // Initial NFC write
  if (st25dv.writeURI(NFC_protocols[0], NFC_messages[0], "")) {
    Serial.println("NFC write failed");
  } else {
    Serial.println("NFC write succeeded");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to open SSID: ");
    Serial.println(SSID);
    status = WiFi.begin(SSID);

    // wait 10 seconds for connection:
    delay(1000);
  }

  Serial.print("You're connected to the network");

  // Retrieve IP address and convert to String
  IPAddress ip = WiFi.localIP();
  String ipString = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
  
  // Update the last NFC message to display the IP address
  NFC_messages[3] = ipString;

  
}

void loop() {
  static int rightLastButtonState = 1;
  int rightButtonState = digitalRead(RIGHT_BUTTON_PIN);
  static int NFC_idx = 1;
  
  if(rightButtonState != rightLastButtonState && !rightButtonState) {
    if(st25dv.writeURI(NFC_protocols[NFC_idx], NFC_messages[NFC_idx], "")) {
      Serial.println("NFC write failed");
    } else {
      Serial.println("NFC write succeeded");
      Serial.println(NFC_messages[NFC_idx]);
    }
    NFC_idx += 1;     // increment NFC_idx
    NFC_idx %= nNFC;  // make sure NFC_idx isn't > nNFC
  }
  rightLastButtonState = rightButtonState;
}
