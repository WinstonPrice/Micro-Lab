// Winston Price 
// Jack Michael Hanna

#include <Wire.h>
#include <VL6180X.h>
#include <PCA95x5.h>

#define SCALING 3
#define AIN1 0
#define AIN2 1
#define BIN1 8
#define BIN2 A2
#define THRESHOLD 150  // Distance threshold for obstacle avoidance in mm

VL6180X sensor;
PCA9535 muxU31;

void setup() {
  Serial.begin(115200);
  delay(3000);
  Wire.begin();

  muxU31.attach(Wire, 0x20);
  muxU31.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
  muxU31.direction(0x1CFF);
  muxU31.write(PCA95x5::Port::P11, PCA95x5::Level::H);

  sensor.init();
  sensor.configureDefault();
  sensor.setScaling(SCALING);
  sensor.setTimeout(100);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
}

void loop() {
  int distance = sensor.readRangeSingleMillimeters();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" mm");

  if (sensor.timeoutOccurred()) {
    Serial.println(" TIMEOUT");
  }

  if (distance < THRESHOLD) {
    // If obstacle detected, stop and turn
    stopMotors();
    turnRight();
    Serial.println("Turning");
  } else {
    // Move forward
    moveForward();
  }

  delay(100); // Short delay between sensor readings
}

void moveForward() {
  analogWrite(AIN1, 100);  
  analogWrite(AIN2, 0);
  analogWrite(BIN1, 0);
  analogWrite(BIN2, 100);
}

void stopMotors() {
  analogWrite(AIN1, 0);
  analogWrite(AIN2, 0);
  analogWrite(BIN1, 0);
  analogWrite(BIN2, 0);
}

void turnRight() {
  analogWrite(AIN1, 100);  
  analogWrite(AIN2, 0);
  analogWrite(BIN1, 100);
  analogWrite(BIN2, 0);
  // delay(1000); // Turn for 1 second
}
