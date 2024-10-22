/*
 * Calculates the angle based on both gyroscope data around the 
 * Y axis and gx and gz accelerometer data.
 * 
 * Seth McNeill 
 * 2022 October 12
*/

#include <Arduino_LSM6DSOX.h>
#define RIGHT_BUTTON_PIN   A0

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(RIGHT_BUTTON_PIN, INPUT);
  
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");

    while (1);
  }

  Serial.println("Theta_g\tTheta_a\tTheta");
}

float lastTheta = 0.0;
unsigned long lastTime = 0;
float alpha = 0.5;

void loop() {
  float gx, gy, gz, ax, ay, az;
  unsigned long curTime = millis();
  unsigned long dt = curTime - lastTime;
  if(!digitalRead(RIGHT_BUTTON_PIN)) {
    lastTheta = 0;
  }

  if (IMU.gyroscopeAvailable() && IMU.accelerationAvailable()) {
    IMU.readGyroscope(gx, gy, gz);
    IMU.readAcceleration(ax, ay, az);

    float theta_g = lastTheta + gy*((float)dt/1000.0);
    Serial.print(theta_g);
    float theta_a = RAD_TO_DEG*atan2(ax,az);
    Serial.print('\t');
    Serial.print(theta_a);
    float theta = alpha*theta_g + (1.0-alpha)*theta_a;
    Serial.print('\t');
    Serial.println(theta);
    lastTheta = theta;
    lastTime = curTime;
  }
}
