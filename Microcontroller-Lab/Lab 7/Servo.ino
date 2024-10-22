// Winston Price 
// Jack Michael Hanna

#include <Servo.h>
#include <Wire.h>
#include <VL6180X.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789 display
#include <PCA95x5.h>

// Define pins
#define TFT_CS            4  // display chip select
#define TFT_RST           A3 // display reset
#define TFT_DC            3  // display D/C
#define SERVO_PIN         9 // Not 100% sure what the pin is

// Define screen dimensions
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135

// Scaling
#define SCALING 3

// Create display and sensor objects
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Servo myservo;
VL6180X sensor;
PCA9535 muxU31;

int servo_angle= 0;  // Variable to store servo position
int distance = 0;  // Variable to store distance

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000);
  Serial.println("Starting...");

  // Initialize the servo
  myservo.attach(SERVO_PIN);
  Serial.println("Not Servo Error");

  Wire.begin();
  muxU31.attach(Wire, 0x20);
  muxU31.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
  muxU31.direction(0x1CFF);
  muxU31.write(PCA95x5::Port::P11, PCA95x5::Level::H);

  sensor.init();
  sensor.configureDefault();
  sensor.setScaling(SCALING);
  sensor.setTimeout(100);

  // Initialize the TFT display
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, LOW);
  tft.init(135, 240);


  // Initialize TFT display
  tft.fillScreen(0x0000); // Black background
  tft.setRotation(1);  // Adjust orientation

  // Display startup message
  tft.setTextColor(0xFFFF);  // White text
  tft.setTextSize(2);
  tft.setCursor(0, 20);
  tft.println("Starting program...");
  delay(3000); // Show message for 3 seconds
  tft.fillScreen(0x0000); // Clear screen

  Serial.println("Setup Complete!");
}

void loop() {
  // Clear the screen before each scan
  tft.fillScreen(0x0000);
  drawAxes();  // Draw axes for cartesian plot

  // Sweep the servo from 0 to 180 degrees in 10-degree increments
  for (servo_angle = 0; servo_angle <= 180; servo_angle += 10) {
    myservo.write(servo_angle);  // Move servo to the position
    delay(300);  // Wait for the servo to move to position

    // Measure distance using VL6180 sensor
    int distance = sensor.readRangeSingleMillimeters();
    Serial.print("Angle: ");
    Serial.print(servo_angle);
    Serial.print(" degrees, Distance: ");
    Serial.print(distance);
    Serial.println(" mm");

    // Plot the data on TFT display
    plotCartesian(servo_angle, distance);
  }

  delay(1000);  // Wait before starting a new scan
}

// Function to plot distance data on a cartesian graph
void plotCartesian(int angle, int distance) {

  int x = map(angle, 0, 180, 20, SCREEN_WIDTH - 20);  
  int y = map((distance / 3) - 35, 0, 200, SCREEN_HEIGHT - 20, 20);  

  // Draw a point for the distance measured
  tft.fillCircle(x, y, 3, 0xFFFF);
}

// Function to draw axes on the display for the cartesian plot
void drawAxes() {
  // Draw x-axis (angle)
  tft.drawLine(20, SCREEN_HEIGHT - 20, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, 0xFFFF);
  tft.setCursor(SCREEN_WIDTH - 30, SCREEN_HEIGHT - 15);
  tft.setTextSize(1);
  tft.println("Angle");

  // Draw y-axis (distance)
  tft.drawLine(20, 20, 20, SCREEN_HEIGHT - 20, 0xFFFF);
  tft.setCursor(5, 10);
  tft.println("Dist");
}
