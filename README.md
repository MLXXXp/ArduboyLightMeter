# Light Meter Sketch for the Arduboy

By Scott Allen

## Description

This sketch turns the Arduboy into a crude light meter by using the red LED of the RGB LED as a sensor.

An LED will emit light when you pass a current through it but most will also work in reverse. If the LED receives light, it will generate a very small current. This program uses the microcontroller's ADC to read the voltage produced by the LED, which will vary in accordance to the intensity of the light shining on it.

Some calibration is supported, to adjust the scaling. Calibration values can be saved to EEPROM.

