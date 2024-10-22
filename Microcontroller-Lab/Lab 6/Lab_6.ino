// Winston Price
// Kaylee Mason

#include <Arduino_LSM6DSOX.h>
#include <Wire.h>
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7789.h> 
#include <SPI.h>
#include <SD.h>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135

// Define pins
#define RIGHT_BUTTON_PIN A0
#define LEFT_BUTTON_PIN 9
#define FRONT_BUTTON_PIN 2
#define BACK_BUTTON_PIN 4
#define TFT_CS 4
#define TFT_RST A3
#define TFT_DC 3
#define SD_CS 7

// Setup display
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// line variables
int posX = SCREEN_WIDTH / 2;
int posY = SCREEN_HEIGHT / 2;
int lineLength = 50; // Length of the line

// IMU variables
float lastTheta = 0.0;
unsigned long lastTime = 0;
float alpha = 0.5;

// File for SD card
File dataFile;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize display
  tft.init(SCREEN_HEIGHT, SCREEN_WIDTH);
  tft.fillScreen(0x0000); // Clear screen
  tft.setRotation(1);  // Adjust orientation
  tft.setTextSize(2);
  tft.setTextColor(0xFFFF);  // White text
  
  // Setup buttons
  pinMode(RIGHT_BUTTON_PIN, INPUT);

  // Error Check the IMU for initialization
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  
  Serial.println("IMU Initialized!");

  // Error Check the SD card for initialization
  if (!SD.begin(SD_CS)) {
    Serial.println("Card failed, or not present.");
    while (1);
  }

  Serial.println("SD Card Initialized!");

  // Open file
  dataFile = SD.open("IMUDATA.TXT", FILE_WRITE | O_TRUNC);
  if (dataFile) {
    dataFile.close();
  }

  Serial.println("t\tax\taz\ttheta_g\ttheta_a\ttheta");
  lastTime = millis();
}

void loop() {

  IMUData();
  
}

void IMUData() {
  // Read IMU data and update position
  float gx, gy, gz, ax, ay, az;
  unsigned long curTime = millis();
  unsigned long time_change = curTime - lastTime;

  if(!digitalRead(RIGHT_BUTTON_PIN)) {
    lastTheta = 0;
  }

  if (IMU.gyroscopeAvailable() && IMU.accelerationAvailable()) {
    IMU.readGyroscope(gx, gy, gz);
    IMU.readAcceleration(ax, ay, az);

    float theta_g = lastTheta + gy * ((float)time_change / 1000.0);
    float theta_a = RAD_TO_DEG * atan2(ax, az);
    float theta = alpha * theta_g + (1.0 - alpha) * theta_a;


    // print data to serial
    serialPrint(curTime, ax, az, theta_g, theta_a, theta);
    
    // Save data to SD card
    saveToSD(curTime, ax, az, theta_g, theta_a, theta);

    // Move character on screen based on theta
    moveCharacter(theta);

    // Draw line at angle theta
    // drawLineAtAngle(theta);

    lastTheta = theta;
    lastTime = curTime;
  }
}

void serialPrint(unsigned long curTime, float ax, float az, float theta_g, float theta_a, float theta) {
    // Print data to Serial
    Serial.println("Writing to IMUDATA.TXT...");
    Serial.print(curTime);
    Serial.print('\t');
    Serial.print(ax);
    Serial.print('\t');
    Serial.print(az);
    Serial.print('\t');
    Serial.print(theta_g);
    Serial.print('\t');
    Serial.print(theta_a);
    Serial.print('\t');
    Serial.println(theta);
    Serial.println(' '); // New line
}

void saveToSD(unsigned long t, float ax, float az, float theta_g, float theta_a, float theta) {
  dataFile = SD.open("IMUDATA.TXT", FILE_WRITE);
  if (dataFile) {
    dataFile.print(t);
    dataFile.print('\t'); // Tab character
    dataFile.print(ax);
    dataFile.print('\t');
    dataFile.print(az);
    dataFile.print('\t');
    dataFile.print(theta_g);
    dataFile.print('\t');
    dataFile.print(theta_a);
    dataFile.print('\t');
    dataFile.println(theta);
    dataFile.close();
  }
}


// Function to move character on screen based on angle
void moveCharacter(float angle) {
  // Map angle to screen width
  int newPosX = map(angle, -180, 180, 0, SCREEN_WIDTH);

  // remove the old line
  tft.drawLine(posX, posY, posX, posY + 10, 0x0000);

  posX = newPosX;

  // Draw a new line
  tft.drawLine(posX, posY, posX, posY + 10, 0xF800); // Red line
}


// Function to draw a line at a given angle
void drawLineAtAngle(float angle) {
  // Convert angle to radians for trigonometric functions
  float angleRad = radians(angle);

  // Calculate the new end coordinates of the line using trigonometry
  int endX = posX + lineLength * cos(angleRad);
  int endY = posY - lineLength * sin(angleRad); 

  // remove the old line
  tft.drawLine(posX, posY, prevEndX, prevEndY, 0x0000); // Black color to erase

  // Draw the new line at the given angle
  tft.drawLine(posX, posY, endX, endY, 0xF800); // Red color

  // Update the previous end coordinates
  prevEndX = endX;
  prevEndY = endY;
}
