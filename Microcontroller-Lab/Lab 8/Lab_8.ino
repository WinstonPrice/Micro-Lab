
// Winston Price 
// Andrew Reynolds

#include <Wire.h>
#include <VL6180X.h>                // Distance sensor
#include <Adafruit_GFX.h>           // Core graphics library
#include <Adafruit_ST7789.h>        // Hardware-specific library for TFT display
#include <PeakDetection.h>          // Peak detection library
#include <PCA95x5.h>                // I2C GPIO library

// Define pins for the TFT display
#define TFT_CS            4  // display chip select
#define TFT_RST           A3 // display reset
#define TFT_DC            3  // display D/C
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135

// Define distance sensor
#define SCALING 3

// Define peak detection
#define LAG               5    // Was used in the original example (not sure if it is needed)
#define PEAK_THRESHOLD    10   
#define VALLEY_THRESHOLD  -10  
#define INFLUENCE         0.1  
#define EPSILON           0.01 
#define MAX_DATA_POINTS   200   // Maximum data points to analyze peaks/valleys

// Initialize TFT display object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
VL6180X sensor;
PCA9535 muxU31;

// Variables for storing distance data
int distanceData[MAX_DATA_POINTS];
int dataIndex = 0;

PeakDetection peakDetect;

// Setup function
void setup() {
  Serial.begin(115200);
  delay(3000);
  
  // Initialize distance sensor
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
  tft.init(SCREEN_HEIGHT, SCREEN_WIDTH);

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
  
  // Setup peak detection parameters
  peakDetect.begin(LAG, PEAK_THRESHOLD, INFLUENCE);  
  peakDetect.setEpsilon(EPSILON);                   
}

// Main loop function
void loop() {
  PeaksAndValleys();
  delay(200); 
}

// Function to detect peaks and valleys from the distance data
void PeaksAndValleys() {
  if (dataIndex >= MAX_DATA_POINTS) {
    // Reset the data index after reaching the maximum
    dataIndex = 0;
  }

  // Get the current distance from the sensor
  int distance = sensor.readRangeSingleMillimeters();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" mm");
  
  // Store the distance in the array
  distanceData[dataIndex] = distance;
  dataIndex++;
  
  // Detect peaks and valleys
  peakDetect.update(distance);  // Update the peak detection algorithm with the new distance
  
  if (peakDetect.isPeak()) {
    Serial.println("Peak detected!");

    tft.fillRect(0, 50, 135, 20, ST77XX_BLACK); // Clear a portion of the screen

    // Set the color for the peak (green arrow)
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(10, 50);
    tft.print("Peak detected: ");
    tft.print(distance);
    tft.println(" mm");

    // Draw a green upward arrow for the peak
    tft.drawLine(120, 60, 120, 40, ST77XX_GREEN); // Vertical line (shaft)
    tft.drawLine(115, 45, 120, 40, ST77XX_GREEN); // Left diagonal (arrowhead)
    tft.drawLine(125, 45, 120, 40, ST77XX_GREEN); // Right diagonal (arrowhead)
  }

  if (peakDetect.isValley()) {
    Serial.println("Valley detected!");

    // Clear the area where the arrow will be drawn (optional, to avoid overlapping)
    tft.fillRect(0, 70, 135, 20, ST77XX_BLACK); // Clear a portion of the screen

    // Set the color for the valley (red arrow)
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 70);
    tft.print("Valley detected: ");
    tft.print(distance);
    tft.println(" mm");

    // Draw a red downward arrow for the valley
    tft.drawLine(120, 80, 120, 100, ST77XX_RED);  // Vertical line (shaft)
    tft.drawLine(115, 95, 120, 100, ST77XX_RED);  // Left diagonal (arrowhead)
    tft.drawLine(125, 95, 120, 100, ST77XX_RED);  // Right diagonal (arrowhead)
  }

  // Print the distance data to TFT
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 20);
  tft.print("Distance: ");
  tft.println(distance);
}
