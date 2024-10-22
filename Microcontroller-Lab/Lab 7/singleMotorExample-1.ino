/* motorExample-20221019
 * 
 * Demonstration of driving motors on CEC 326 board
 * https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/
 * 
 * Seth McNeill
 * 2022 October 19
 */

#define AIN1 0
#define AIN2 1

void setup() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
}

void loop() {
  analogWrite(AIN1, 75);
  analogWrite(AIN2, 0);
  delay(2000);
  analogWrite(AIN1, 0);
  analogWrite(AIN2, 255);
  delay(2000);
  analogWrite(AIN1, 255);
  analogWrite(AIN2, 0);
  delay(1000);
  analogWrite(AIN1, 255);
  analogWrite(AIN2, 255);
  delay(2000);
}
