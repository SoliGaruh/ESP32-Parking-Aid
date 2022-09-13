# ESP32-Parking-Aid

Illuminates an RGB LED based on the distance an object is away from the HC-SR04 ultrasonic
sensor. It was developed to assist parking a car in a garage.

The RGB LED's red, green and blue pins are connected to PIN_RED, PIN_GREEN and PIN_BLUE.

A 220 ohm resitor is connected inline between each of the LED colour pins and the corresponding ESP32
GPIO ports. The LED's GND is connected to GND on the ESP32.

The wiring for the HC-SR04 is shown in the file "ESP32 HC-SR04 Wiring.png". The HC-SR04 is used
as it supports the 3.3V logic of the ESP32.

The system is connected to mains power so there has been no consideration to power this from a
battery.

This project does not depend on any libraries.
