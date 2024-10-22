// Winston Price 
// Hailey Choi


#include <Wire.h>
#include <PCA95x5.h>           // For PCA9535 library
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789 display

// Define pins
#define RIGHT_BUTTON_PIN   A0
#define LEFT_BUTTON_PIN    9
#define FRONT_BUTTON_PIN   2
#define BACK_BUTTON_PIN    4
#define TFT_CS            4  // display chip select
#define TFT_RST           A3 // display reset
#define TFT_DC            3  // display D/C
#define SD_CS             7

// Declare variables
// instatiate a tft object for the display
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135

PCA9555 muxU31;

// Initialize TFT display
//Adafruit_TFTLCD tft(TFT_CS, TFT_CD, TFT_WR, TFT_RD, TFT_RESET);

// Setup for I2C GPIO expander buttons
#define PCA9535_ADDR 0x20

// Variables for character position
int posX = SCREEN_WIDTH / 2;
int posY = 68; // Initial Y position for character
int lastRightButtonVal = 1;
int lastLeftButtonVal = 1;
int lastFrontButtonVal = 1;
int lastBackButtonVal = 1;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000);
  Serial.println("Starting...");

  // Setup pins
  pinMode(RIGHT_BUTTON_PIN, INPUT);
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FRONT_BUTTON_PIN, INPUT);
  pinMode(BACK_BUTTON_PIN, INPUT);

  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, LOW);
  tft.init(135, 240);
  //pinMode(BUZZ_PIN, OUTPUT);
  //pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin();
  muxU31.attach(Wire, PCA9535_ADDR);
  muxU31.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
  muxU31.direction(0xFFFC); 

  // Initialize TFT display
  // tft.begin();
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

  // Part 3
  tft.fillRect(posX, posY, 10, 10, 0xF800);
  delay(3000);
  tft.fillRect(posX, posY, 20, 20, 0x00FF);
  delay(3000);
  tft.fillRect(posX, posY, 30, 30, 0xFFFF);
  delay(3000);
  tft.fillScreen(0x0000);
  
  // Animation 
  int cube_size = 10;
  for(int animation_time = 0; animation_time < 25; animation_time++ ) {
    tft.fillRect(posX, posY, cube_size, cube_size, 0xFFFF);
    tft.fillRect(posX, posY, cube_size, -1 * cube_size, 0xFFFF);
    tft.fillRect(posX, posY, -1 * cube_size, cube_size, 0xFFFF);
    tft.fillRect(posX, posY, -1 * cube_size, -1 * cube_size, 0xFFFF);
    delay(200);
    cube_size += 5;     
  }
  tft.fillScreen(0x0000);
  
}


void loop() {
  checkButtons(); 
}


void checkButtons() {


  // Right Button
  int curRightButtonVal = digitalRead(RIGHT_BUTTON_PIN);
  if (curRightButtonVal == LOW && lastRightButtonVal != curRightButtonVal) {
    Serial.println("Right button pressed!");
    moveCharacter_x(10);          // Move right
  }
  lastRightButtonVal = curRightButtonVal;

  // Left Button
  uint16_t buttonState = muxU31.read();
  if (!(buttonState & 0x0001) && lastLeftButtonVal !=  buttonState) {
    Serial.println("Left Button pressed!");
    moveCharacter_x(-10);         // Move left
  }
  lastLeftButtonVal =  buttonState;
  
  
  // Front Button
  if (buttonState & 0x0002 && lastFrontButtonVal !=  buttonState) {
    Serial.println("Front Button pressed!");
    moveCharacter_y(-10);
  }
  lastFrontButtonVal =  buttonState;
 
  // Back Button 
  if (buttonState & 0x0004 && lastBackButtonVal !=  buttonState) {
    Serial.println("Back Button pressed!");
    moveCharacter_y(10);
  }
  lastBackButtonVal =  buttonState;
}

// Function to move character in the x direction
void moveCharacter_x(int x_pixel) {
  
  tft.fillRect(posX, posY, 10, 10, 0x0000); // Create a black rectangle

  // Update position
  posX += x_pixel;
  
  // Screen bounds
  if (posX < 0) posX = 0;
  if (posX > SCREEN_WIDTH - 10) posX = SCREEN_WIDTH - 10;

  // Draw new character position
  tft.fillRect(posX, posY, 10, 10, 0xF800); // Create a red rectangle
}

// Function to move character in the y direction
void moveCharacter_y(int y_pixel) {
  
  tft.fillRect(posX, posY, 10, 10, 0x0000); // Create a red rectangle

  // Update position
  posY += y_pixel;
  
  // Screen bounds
  if (posY < 0) posY = 0;
  if (posY > SCREEN_HEIGHT - 10) posY = SCREEN_HEIGHT - 10;

  // Draw new character position
  tft.fillRect(posX, posY, 10, 10, 0xF800); // Create a red rectangle
}


// Extra Credit
// void animate() {
//   static int animState = 0;
//   static unsigned long lastAnimTime = 0;

//   if (millis() - lastAnimTime > 500) { // Every 500ms
//     lastAnimTime = millis();

//     // Change color of the rectangle from green to red
//     if (animState == 0) {
//       tft.fillRect(50, 250, 50, 50, 0x07E0); // Green
//       animState = 1;
//     } else {
//       tft.fillRect(50, 250, 50, 50, 0xF800); // Red
//       animState = 0;
//     }
//   }
// }
