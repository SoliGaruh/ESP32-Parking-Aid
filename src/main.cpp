#include <Arduino.h>

// RGB LED pins
#define PIN_RED 23   // GIOP23
#define PIN_GREEN 22 // GIOP22
#define PIN_BLUE 21  // GIOP21

// Ultrasonic sensor pins
const int trigPin = 5;
const int echoPin = 18;

// define sound speed in cm/uS
#define SOUND_SPEED 0.034

// ranges in cm for LED colours
#define LONG_RANGE 200
#define MEDIUM_RANGE 100
#define SHORT_RANGE 45

long duration;
float distanceCm;

void setup()
{
  Serial.begin(115200); // Starts the serial communication
  delay(500);

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
}

void loop()
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_SPEED / 2;

  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  if (distanceCm > LONG_RANGE)
  {
    // green
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 255);
    analogWrite(PIN_BLUE, 0);
  }
  else if (distanceCm > MEDIUM_RANGE)
  {
    // blue
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 255);
  }
  else if (distanceCm > SHORT_RANGE)
  {
    // red
    analogWrite(PIN_RED, 255);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 0);
  }

  delay(250);
}
