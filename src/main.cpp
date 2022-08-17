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

// distances in cm for LED colours
#define WARN_DIST 200
#define STOP_DIST 50

#define PARKED_LED_ONTIME_MS 60000 // 60 seconds

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

// time (millis()) when the car enters short range
unsigned long shortRangeStartTime;

// used to know whether the LED has been switched off due to the car being in short range for LED_PARKED_ONTIME_MS ms.
// Assumed parked.
// Yes I could try and work out if it has stopped based on distance measurements but it's not necessary.
bool ledTimedOut = false;

// used for distance sensor calculations
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
  //  Serial.print("Distance (cm): ");
  // Serial.println(distanceCm);

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
}