#include <Adafruit_NeoPixel.h> // NeoPixel library
#include <Arduino_LSM6DSOX.h>  // Include IMU library
#include <Adafruit_ST7789.h>   // Hardware-specific library for ST7789 display
#include <Adafruit_GFX.h>      // Core graphics library
//#include "WiFiNINA.h"          // for A4-A7 and wifi/bluetooth
#include <PCA95x5.h>           // MUX library
#include <Servo.h>             // Servo

#define RIGHT_BUTTON_PIN   A0
#define SERVO_PIN          9
#define NEO_PIN           17  // WARNING! THIS IS GPIO NOT D NUMBER for NeoPixels
#define NEO_COUNT         18  // number of NeoPixels
#define NEO_MAG           100 // NeoPixel default magnitude
#define AIN1              0   // Motor driver A
#define AIN2              1   // Motor driver A
#define BIN1              8   // Motor driver A
#define BIN2              A2  // Motor driver A
#define TFT_CS            4   // display chip select
#define TFT_RST           A3  // display reset
#define TFT_DC            3   // display D/C

Adafruit_NeoPixel strip(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Servo myservo;
PCA9535 muxU31;

enum States {pid, halt};
States state = halt;

float KP = 3.0;  // proportional gain
float KI = 0.01; // integral gain
float KD = 10.0; // derivative gain

float targetAngle = 0.0; // Target angle to maintain level
float currentAngle = 0.0;
int motorSpeed = 0;
int rightButtonState;
int rightLastButtonState;

void checkButtonState();
void readIMUAngle();
float pidControl(float error);
void updateMotorSpeed(float error);
void displayStatus(float error);

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting...");

  // NeoPixel Setup
  strip.begin();
  strip.show();
  
  // Servo Setup
  myservo.attach(SERVO_PIN);
  
  // MUX Setup
  Wire.begin();
  muxU31.attach(Wire, 0x20);
  muxU31.polarity(PCA95x5::Polarity::ORIGINAL_ALL);

  // IMU Setup
  if (!IMU.begin()) {
    Serial.println("IMU initialization failed!");
    while (1);
  }
  Serial.println("IMU Initialized!");

  // TFT Display Setup
  tft.init(135, 240);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);
  tft.setTextSize(3);
  tft.println("PID Angle Control");

  // Button & Motor Setup
  pinMode(RIGHT_BUTTON_PIN, INPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
}

void loop() {
  checkButtonState();
  readIMUAngle();

  
  float error = targetAngle - currentAngle;

  // if state is pid and not halt, calculate motor speed using PID control
  if (state == pid) {
    motorSpeed = (int)pidControl(error);
  } else if (state == halt) {
    motorSpeed = 0;
  }

  updateMotorSpeed(error);
  displayStatus(error);

  delay(100); // Small delay for stability
}

// Function to check the state of the right button and toggle between PID and halt
void checkButtonState() {
  rightButtonState = digitalRead(RIGHT_BUTTON_PIN);

  if (rightButtonState == LOW && rightButtonState != rightLastButtonState) {
    state = (state == halt) ? pid : halt;
    rightLastButtonState = rightButtonState;
    delay(100); // Debounce delay
  }

  if (rightButtonState != rightLastButtonState) {
    rightLastButtonState = rightButtonState;
  }
}

// Function to read angle from IMU
void readIMUAngle() {
  if (IMU.accelerationAvailable()) {
    float ax, ay, az;
    IMU.readAcceleration(ax, ay, az);
    currentAngle = RAD_TO_DEG * atan2(ax, az); // Angle based on front-to-back tilt
  }
}

// PID control for angle-based stabilization
float pidControl(float error) {
  static float cumulativeError = 0.0;
  static float lastError = 0.0;
  float d = KD * (error - lastError);
  cumulativeError += KI * error;
  Serial.print(cumulativeError);
  Serial.print(",");
  Serial.print(d);
  Serial.print(",");
  lastError = error;
  return KP * error + cumulativeError + d;
}

// Function to update motor speed based on PID control output
void updateMotorSpeed(float error) {
  int absSpeed = min(abs(motorSpeed), 255); // Ensure speed does not exceed 255
  if (motorSpeed > 0) {
    analogWrite(AIN1, absSpeed);
    analogWrite(AIN2, 0);
    analogWrite(BIN1, 0);
    analogWrite(BIN2, absSpeed);
  } else {
    analogWrite(AIN1, 0);
    analogWrite(AIN2, absSpeed);
    analogWrite(BIN1, absSpeed);
    analogWrite(BIN2, 0);
  }
}

// Function to display the system's current status on the TFT screen
void displayStatus(float error) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("PID Angle Control");
  tft.print("Target Angle: ");
  tft.println(targetAngle);
  tft.print("Current Angle: ");
  tft.println(currentAngle);
  tft.print("Error: ");
  tft.println(error);
  tft.print("Motor Speed: ");
  tft.println(motorSpeed);
}
