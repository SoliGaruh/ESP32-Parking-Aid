/*
ESP32 Parking Aid copyright by Steve Tattersall. It has been developed using Microsoft VS Code and PlatformIO extension.
This code is placed in the public domain and may be used freely as long as the author is credited.

Description
-----------

Illuminates an RGB LED based on the distance an object is away from the HC-SR04 ultrasonic
sensor. It was developed to assist parking a car in a garage.

The RGB LED's red, green and blue pins are connected to PIN_RED, PIN_GREEN and PIN_BLUE.
A 220 ohm resitor is connected inline between each of the LED colour pins and the corresponding ESP32
GPIO ports. The LED's GND is connected to GND on the ESP32.

The wiring for the HC-SR04 is shown in the file "ESP32 HC-SR04 Wiring.png". The HC-SR04 is used
as it supports the 3.3V logic of the ESP32.

The system is connected to mains power so there has been no consideration to power this from a
battery.
*/

#include <Arduino.h>

// RGB LED pins
#define PIN_RED 23   // GIOP23
#define PIN_GREEN 22 // GIOP22
#define PIN_BLUE 21  // GIOP21

// Ultrasonic sensor pins
#define PIN_TRIGGER 5
#define PIN_ECHO 18

// define sound speed in cm per microsecond
#define SOUND_SPEED 0.0343

// distances in cm used to set the colour of the LED
#define WARN_DIST 200
#define STOP_DIST 65

// This is how long the LED will stay lit once the car is parked.
#define PARKED_LED_ONTIME_MS 30000 // 30 seconds

// enum representing where the car is
enum carLocation_enum : char
{
  unknown = 0,
  longRange,
  mediumRange,
  shortRange
};

// car location is initialsed to unknown to ensure the system lights up an LED on startup
carLocation_enum carLocation = unknown;

// time (millis()) when the car enters short range. This is used to determine how long the
// LED will remain lit when the car is parked.
unsigned long shortRangeStartTime;

// used to determine whether the LED has been switched off due to the car being in short range for LED_PARKED_ONTIME_MS ms.
// Assumed parked.
bool ledTimedOut = false;

// used for distance sensor calculations
long duration;
float distanceCm;

// Initialise the Serial monitor and the GPIO pins
void setup()
{
  Serial.begin(115200); // Starts the serial communication
  delay(500);

  pinMode(PIN_TRIGGER, OUTPUT); // Sets the trigPin as an Output
  pinMode(PIN_ECHO, INPUT);     // Sets the echoPin as an Input

  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
}

void loop()
{
  // Clears the trigPin
  digitalWrite(PIN_TRIGGER, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 microseconds
  digitalWrite(PIN_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIGGER, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(PIN_ECHO, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_SPEED / 2;

  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  if (distanceCm > WARN_DIST)
  {
    // the point here and below is not to write to the LED pins if the right colour is already set.
    if (carLocation != longRange)
    {
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 255);
      analogWrite(PIN_BLUE, 0);

      carLocation = longRange;
      Serial.println("LONG");
    }
  }
  else if (distanceCm > STOP_DIST)
  {
    if (carLocation != mediumRange)
    {
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 255);

      Serial.println("MEDIUM");

      carLocation = mediumRange;
    }
  }
  else if (distanceCm <= STOP_DIST)
  {
    if (carLocation != shortRange)
    {
      // record the current time to determine when to turn off the red LED
      shortRangeStartTime = millis();
      ledTimedOut = false;

      analogWrite(PIN_RED, 255);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE, 0);

      Serial.println("SHORT");

      carLocation = shortRange;
    }

    // turn off the red LED if it has been on for LED_PARKED_ONTIME_MS milliseconds
    if (millis() - shortRangeStartTime > PARKED_LED_ONTIME_MS)
    {
      // switch off the LED if it hasn't already been switched off
      if (!ledTimedOut)
      {
        Serial.println("TIMEOUT : LED off");
        analogWrite(PIN_RED, 0);
        analogWrite(PIN_GREEN, 0);
        analogWrite(PIN_BLUE, 0);

        ledTimedOut = true;
      }
    }
  }

  // checking the distance 5 times/s is adequate
  delay(200);
}