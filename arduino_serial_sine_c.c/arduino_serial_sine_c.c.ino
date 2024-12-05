#include <math.h>

//const int ledPin = 13;  // Built-in LED pin
float sineValue;        // Store sine wave value
unsigned long lastTime = 0;
unsigned long startTime = 0;
const float frequency = 1.0;  // Frequency of the sine wave in Hz (1 cycle per second)
const float amplitude = 60.0; // Amplitude of the sine wave (controls the range)
const float offset = 90.0;    // Offset to shift the sine wave between 60 and 120

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);  // Start serial communication
  startTime = millis();
}

void loop() {
  unsigned long currentTime = millis();

  // Calculate sine wave value every second (1000ms)
  if (currentTime - lastTime >= 1000) {
    float elapsedTime = (currentTime - startTime) / 1000.0; // Time in seconds
    sineValue = amplitude * sin(2 * PI * frequency * elapsedTime / 1000.0) + offset;
    Serial.println(sineValue);  // Send sine value to Raspberry Pi
    lastTime = currentTime;
  }

  // Check for incoming data from Raspberry Pi to control the LED
  if (Serial.available() > 0) {
    char incomingChar = Serial.read();
    if (incomingChar == '1') {
      digitalWrite(LED_BUILTIN, HIGH);  // Turn on LED
    } else if (incomingChar == '0') {
      digitalWrite(LED_BUILTIN, LOW);   // Turn off LED
    }
  }
}
