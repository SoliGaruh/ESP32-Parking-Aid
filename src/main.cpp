/*
 Copyright (C) 2022 Steve Tattersall

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include <Arduino.h> // required for MS VS Code and PlatformIO IDE

// The resolution in bits to use for the analogRead() function
#define ANALOG_READ_RESOLUTION 12

// the pin the trimport is connected to
#define PIN_POT 2

// RGB LED pins
#define PIN_RED 23   // GIOP23
#define PIN_GREEN 22 // GIOP22
#define PIN_BLUE 21  // GIOP21

// Ultrasonic sensor pins
#define PIN_TRIGGER 5
#define PIN_ECHO 18

// The speed of sound in cm per microsecond
#define SOUND_SPEED 0.0343

// Threshold distances in cm used to set the colour of the LED
#define WARN_DIST 200
#define STOP_DIST 65

// Maximum distance in cm that the trimpot can adjust the threshold distances towards the garage door. 
// The minimum value is always zero. On the ESP32, the trimpot mapping used is 
// map(...,0,4095,0,MAX_DISTANCE_OFFSET).
#define MAX_DISTANCE_OFFSET 30

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

// trimpot mapped offset value
long mappedOffsetValue;

// Maximum trimpot value
const long MaxTrimPotValue = (1 << ANALOG_READ_RESOLUTION) - 1;

// Initialise the Serial monitor and the GPIO pins
void setup()
{
  Serial.begin(115200); // Starts the serial communication
  delay(500);

  // Ultrasonic sensor pins
  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // LED pins
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);

  // Trimpot pin
  pinMode(PIN_POT, INPUT);

  // Set the analog read resolution in bits. I'm trying to ensure that it is always the same for all
  // boards, to ensure the map() function used below works properly.
  analogReadResolution(ANALOG_READ_RESOLUTION);
}

void loop()
{
  // The mapped offset is added to both the warning and stopping distance when determining the position of 
  // the car and the colour of the LED.
  mappedOffsetValue = map(analogRead(PIN_POT), 0, MaxTrimPotValue, 0, MAX_DISTANCE_OFFSET);

  // Prints the distance offset to the Serial Monitor
  Serial.print("Distance Offset (cm) [0 - 30] : ");
  Serial.println(mappedOffsetValue);

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

  // Prints the car's distance to the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  //! IMPORTANT: Never subtract the trimpot's mapped offset value from the threshold distance. Subtraction could
  //! result in the calculated threshold distance being too close to the sensor resulting in an accident.
  if (distanceCm > WARN_DIST + mappedOffsetValue)
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

  //! IMPORTANT: Never subtract the trimpot's mapped offset value from the threshold distance. Subtraction could
  //! result in the calculated threshold distance being too close to the sensor resulting in an accident.
  else if (distanceCm > (STOP_DIST + mappedOffsetValue))
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

  //! IMPORTANT: Never subtract the trimpot's mapped offset value from the threshold distance. Subtraction could
  //! result in the calculated threshold distance being too close to the sensor resulting in an accident.
  else if (distanceCm <= (STOP_DIST + mappedOffsetValue))
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