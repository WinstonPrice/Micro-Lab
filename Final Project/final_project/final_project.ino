#include <Arduino_LSM6DSOX.h>

#define AIN1 0
#define AIN2 1
#define BIN1 8
#define BIN2 A2

float front_back_angle = 0;
float left_right_angle = 0;

float front_back_acceleration = 0;
float left_right_acceleration = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // Initialize motor driver pins
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
}

void loop() {
  readIMUAngle();
  // angle_to_acceleration();
  angle_to_motor();
}

void readIMUAngle() {
  if (IMU.accelerationAvailable()) {
    float ax, ay, az;
    IMU.readAcceleration(ax, ay, az);

    // Calculate angles in degrees
    front_back_angle = RAD_TO_DEG * atan2(ay, az);
    left_right_angle = RAD_TO_DEG * atan2(ax, az);

    Serial.print("Front-back angle: " );
    Serial.println(front_back_angle);

    Serial.print("Left-right angle: " );
    Serial.println(left_right_angle);

  }
}

void angle_to_acceleration() {
  // Convert angles into pseudo-acceleration (proportional to tilt)
  front_back_acceleration = front_back_angle / 90.0 * 9.81;  // Scale to range of ±9.81
  left_right_acceleration = left_right_angle / 90.0 * 9.81;  // Scale to range of ±9.81

  Serial.print("Front-back acceleration: " );
  Serial.println(front_back_acceleration);

  Serial.print("Left-right acceleration: " );
  Serial.println(left_right_acceleration);
  Serial.println();
  delay(1000);
}

void angle_to_motor() {
  // Constrain angles to avoid unrealistic values
  front_back_angle = constrain(front_back_angle, -90, 90);
  left_right_angle = constrain(left_right_angle, -90, 90);

  // Base motor speed determined by the forward/backward tilt
  int baseSpeed = map(abs(front_back_angle), 0, 90, 0, 255); // Scale to PWM range

  // Calculate adjustments for turning based on left/right tilt
  int turnAdjustment = map(left_right_angle, -90, 90, -baseSpeed, baseSpeed);

  // Calculate motor speeds
  int leftMotorSpeed = baseSpeed + turnAdjustment;  // More power for turning right
  int rightMotorSpeed = baseSpeed - turnAdjustment; // More power for turning left

  // Ensure speeds are within PWM bounds
  leftMotorSpeed = constrain(leftMotorSpeed, -255, 255);
  rightMotorSpeed = constrain(rightMotorSpeed, -255, 255);

  // Set motor directions and speeds
  if (leftMotorSpeed >= 0) {
    analogWrite(AIN1, leftMotorSpeed);
    analogWrite(AIN2, 0);
  } else {
    analogWrite(AIN1, 0);
    analogWrite(AIN2, -leftMotorSpeed); // Reverse direction
  }

  if (rightMotorSpeed >= 0) {
    analogWrite(BIN1, rightMotorSpeed);
    analogWrite(BIN2, 0);
  } else {
    analogWrite(BIN1, 0);
    analogWrite(BIN2, -rightMotorSpeed); // Reverse direction
  }

  // Debug output for verification
  Serial.print("Left Motor Speed: ");
  Serial.println(leftMotorSpeed);

  Serial.print("Right Motor Speed: ");
  Serial.println(rightMotorSpeed);
  Serial.println();
}

