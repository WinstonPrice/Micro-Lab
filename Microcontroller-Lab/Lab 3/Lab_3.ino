// Winston Price
// Roee Shemesh
 
#include <WiFiNINA.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <PCA95x5.h> // For PCA9535 library
 
// Define pins
#define RIGHT_BUTTON_PIN A0
#define BUZZ_PIN 2
#define NEO_PIN 17
#define NEO_COUNT 18
 
// Initialize NeoPixel strip
Adafruit_NeoPixel strip(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
 
PCA9555 muxU31;
 
// Setup for I2C GPIO expander buttons
#define PCA9535_ADDR 0x20
 
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000); // Wait for serial to connect
  Serial.println("Starting...");
 
  // Setup pins
  pinMode(RIGHT_BUTTON_PIN, INPUT);
  pinMode(BUZZ_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
 
  // Setup NeoPixels
  strip.begin();
  strip.clear();
  strip.show();
 
  // Initialize I2C and GPIO expander
  Wire.begin();
  muxU31.attach(Wire, PCA9535_ADDR);
  muxU31.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
  muxU31.direction(0xFFFC); // Only last two pins are outputs
 
  Serial.println("Setup Complete!");
}
 
void loop() {
  checkButtons();
 
  // Handle serial input
  handleSerialInput();
 
  // Run NeoPixel effect
  runNeoPixelEffect();
}
 
void checkButtons() {
  // Read the RIGHT_BUTTON_PIN
  if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
    Serial.println("Right button pressed!");
    tone(BUZZ_PIN, 1000, 100);
    //delay(200);
  }
 
  // Read other buttons from the PCA9535
  uint16_t buttonState = muxU31.read();
  if (!(buttonState & 0x0001)) {
    Serial.println("Left Button pressed!");
    digitalWrite(LED_BUILTIN, HIGH); // Turn on built-in LED
  }
 
  if (buttonState & 0x0002) {
    Serial.println("Front Button pressed!");
    digitalWrite(BUZZ_PIN, HIGH); // Turn on buzzer
  } else {
    digitalWrite(BUZZ_PIN, LOW); // Turn off buzzer
  }
 
  if (buttonState & 0x0004) {
    Serial.println("Back Button pressed!");
    digitalWrite(BUZZ_PIN, HIGH); // Turn on buzzer
  } else {
    digitalWrite(BUZZ_PIN, LOW); // Turn off buzzer
  }
}
 
void handleSerialInput() {
  while(Serial.available() > 0) {
    // characters have been received
    char inChar = Serial.read();
    switch(inChar) {
      case 'a': digitalWrite(LED_BUILTIN,HIGH);
      //delay(2000);
      break;
      case 'A': digitalWrite(LED_BUILTIN,LOW); break;
      case 'b': digitalWrite(LEDB, HIGH); break;
      case 'r': digitalWrite(LEDR, HIGH); break;
      case 'g': digitalWrite(LEDG, HIGH); break;
      case 'z': tone(BUZZ_PIN, 1000, 100); break;
      case '\n': break;
      default:
        Serial.print("Unknown character: ");
        Serial.println(inChar);
    }
  }
}
 
void runNeoPixelEffect() {
  for (int i = 0; i < NEO_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(120, 0, 120)); // Set purple color
  }
  strip.show();
  delay(500);
  strip.clear();
  strip.show();
  delay(500);
}