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

#define RIGHT_BUTTON_PIN   A0
#define BUZZ_PIN           2

// NFC setup
const int NFC_GPO_PIN = 8;   // Not used but required for setup
const int NFC_LPD_PIN = A1;  // Not used but required for setup
const int nNFC = 4; // number of NFC messages
String NFC_protocols[nNFC] = {"mailto:", "tel:", "sms:", "https://"};
String NFC_messages[nNFC] = {"seth.mcneill@erau.edu",
                          "9287773894",
                          "3048372463",
                          "erau.edu"};

ST25DV st25dv(NFC_GPO_PIN, -1, &Wire);

void setup() {
  Serial.begin(115200);
  delay(3000);
  
  pinMode(RIGHT_BUTTON_PIN, INPUT);
  pinMode(BUZZ_PIN, OUTPUT);

  if (st25dv.begin() != 0) {
    Serial.println("Failed to start NFC ST25");
    while(1);
  } else {
    Serial.println("NFC ST25 started");
  }
  if(st25dv.writeURI(NFC_protocols[0], NFC_messages[0], "")) {
    Serial.println("NFC write failed");
  } else {
    Serial.println("NFC write succeeded");
  }

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
