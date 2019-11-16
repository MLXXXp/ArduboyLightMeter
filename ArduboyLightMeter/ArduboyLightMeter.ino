// ===========================================================================
// Use an Arduboy as a light meter by using one of the RGB LEDs for the sensor
// ===========================================================================

// Version 1.0

/*
------------------------------------------------------------------------------
Copyright (c) 2019, Scott Allen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------
*/

#include <Arduboy2.h>
#include <EEPROM.h>

// EEPROM storage (6 bytes)
#define LIGHT_METER_EEPROM_START 780
#define LIGHT_METER_EEPROM_ID1 LIGHT_METER_EEPROM_START
#define LIGHT_METER_EEPROM_ID2 (LIGHT_METER_EEPROM_START + sizeof(char))
#define LIGHT_METER_EEPROM_CAL_MIN (LIGHT_METER_EEPROM_ID2 + sizeof(char))
#define LIGHT_METER_EEPROM_CAL_MAX (LIGHT_METER_EEPROM_CAL_MIN + sizeof(int))

#define STORAGE_ID_1 'L'
#define STORAGE_ID_2 'm'

// This sets the reading rate in readings per second. It is also used for the
// auto increment/decrement repeat rate for setting calibration values.
#define FRAME_RATE 5

// The number of frames to wait before button auto-repeat starts
#define DELAY_FRAMES 5

// The value to increment a calibration field by when auto-repeating
#define REPEAT_CHANGE 5

// The pin used by the ADC to read the light intensity
#define ADC_PIN RED_LED

// The full scale ADC value
#define ADC_MAX 1023

// The high value for the scaled floating point display range.
// (The low value is fixed at 0.)
#define SCALE_HIGH 100.0

// Default calibration values
#define DEFAULT_CAL_MIN 200
#define DEFAULT_CAL_MAX 360

// All the constant stings
const char StrMainHeading[] PROGMEM = "LIGHT METER";
const char StrMin[] PROGMEM = "Min:";
const char StrMax[] PROGMEM = "Max:";
const char StrNegative[] PROGMEM = "NEGATIVE";
const char StrOverflow[] PROGMEM = "OVERFLOW";
const char StrMainButtonRst[] PROGMEM = "A:Reset Max/Min";
const char StrMainButtonCal[] PROGMEM = "B:Cal";

const char StrCalHeading[] PROGMEM = "CALIBRATION";
const char StrCalMin[] PROGMEM = "Minimum raw: ";
const char StrCalMax[] PROGMEM = "Maximum raw: ";
const char StrRawReading[] PROGMEM =  "Raw reading: ";
const char StrDispReading[] PROGMEM = "Displayed: ";
const char StrCalButtonSel[] PROGMEM = "\x1B:Select";
const char StrCalButtonCopy[] PROGMEM = "\x1A:Copy Value";
const char StrCalButtonInc[] PROGMEM = "\x18:Value +";
const char StrCalButtonDec[] PROGMEM = "\x19:Value -";
const char StrCalButtonExit[] PROGMEM = "A:Exit";
const char StrCalButtonSave[] PROGMEM = "B:Save";

const char StrSaveQ1[] PROGMEM = "SAVE TO";
const char StrSaveQ2[] PROGMEM = "EEPROM?";
const char StrSaveButtonYes[] PROGMEM = "A:Yes";
const char StrSaveButtonNo[] PROGMEM = "B:No";
const char StrSaved[] PROGMEM = "SAVED";

#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8
#define BAR_HEIGHT 10
#define VAL_CHARS 6

// Defines for text and field locations
#define MAIN_HEADING_X centerStr_P(StrMainHeading)
#define MAIN_HEADING_Y 0
#define MAIN_VALUE_X ((WIDTH / 2) - (CHAR_WIDTH * 3 * 4))
#define MAIN_VALUE_Y 9
#define MAIN_BAR_X 0
#define MAIN_BAR_Y 33
#define MAIN_BAR_NEG_X centerStr_P(StrNegative)
#define MAIN_BAR_NEG_Y (MAIN_BAR_Y + 1)
#define MAIN_BAR_OVL_X centerStr_P(StrOverflow)
#define MAIN_BAR_OVL_Y (MAIN_BAR_Y + 1)
#define MAIN_MIN_X 0
#define MAIN_MIN_Y 45
#define MAIN_MAX_X (rightStr_P(StrMax) - (VAL_CHARS * CHAR_WIDTH))
#define MAIN_MAX_Y MAIN_MIN_Y
#define MAIN_BTN_RST_X 0
#define MAIN_BTN_RST_Y 55
#define MAIN_BTN_CAL_X rightStr_P(StrMainButtonCal)
#define MAIN_BTN_CAL_Y MAIN_BTN_RST_Y

#define CAL_HEADING_X centerStr_P(StrCalHeading)
#define CAL_HEADING_Y 0
#define CAL_MIN_X CHAR_WIDTH
#define CAL_MIN_Y 8
#define CAL_MAX_X CHAR_WIDTH
#define CAL_MAX_Y 16
#define CAL_RAW_X 0
#define CAL_RAW_Y 24
#define CAL_DISPLAYED_X 0
#define CAL_DISPLAYED_Y 32
#define CAL_BUTTON_SEL_X 0
#define CAL_BUTTON_SEL_Y 40
#define CAL_BUTTON_COPY_X rightStr_P(StrCalButtonCopy)
#define CAL_BUTTON_COPY_Y CAL_BUTTON_SEL_Y
#define CAL_BUTTON_INC_X 0
#define CAL_BUTTON_INC_Y 48
#define CAL_BUTTON_DEC_X rightStr_P(StrCalButtonDec)
#define CAL_BUTTON_DEC_Y CAL_BUTTON_INC_Y
#define CAL_BUTTON_EXIT_X 0
#define CAL_BUTTON_EXIT_Y 56
#define CAL_BUTTON_SAVE_X rightStr_P(StrCalButtonSave)
#define CAL_BUTTON_SAVE_Y CAL_BUTTON_EXIT_Y

#define SAVE_Q_1_X centerStr2_P(StrSaveQ1)
#define SAVE_Q_1_Y 12
#define SAVE_Q_2_X centerStr2_P(StrSaveQ2)
#define SAVE_Q_2_Y (SAVE_Q_1_Y + (CHAR_HEIGHT * 2))
#define SAVE_BUTTON_YES_X 0
#define SAVE_BUTTON_YES_Y 56
#define SAVE_BUTTON_NO_X rightStr_P(StrSaveButtonNo)
#define SAVE_BUTTON_NO_Y SAVE_BUTTON_YES_Y

#define SAVE_SAVED_X centerStr2_P(StrSaved)
#define SAVE_SAVED_Y 24

Arduboy2 arduboy;

// The raw reading from the ADC normalised so that higher illumination has
// higher values.
int adcReading;

float  displayValue;
float  displayMin;
float  displayMax;

int calMin;
int calMax;

bool calMaxSelected = false;

unsigned int delayCount = 0;
bool repeating = false;

enum class State : uint8_t {
  Main,
  Calibrate,
  SavePrompt
};

State currentState = State::Main;

// ========== setup ==========
void setup() {
  arduboy.begin();
  arduboy.setFrameRate(FRAME_RATE);
  arduboy.setTextWrap(false);

  getSavedCal();

  resetMinMax();

  power_adc_enable();  // The library turns the ADC off, so power it on
  pinMode(ADC_PIN, INPUT);
  analogReference(DEFAULT); // Use VCC for the ADC reference
}

// ========== loop ==========
void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }

  arduboy.pollButtons();

  arduboy.clear();

  adcReading = ADC_MAX - analogRead(ADC_PIN);
  displayValue = (float)(adcReading - calMin) * (SCALE_HIGH / (float)(calMax - calMin));

  if (displayValue < displayMin) {
    displayMin = displayValue;
  }
  if (displayValue > displayMax) {
    displayMax = displayValue;
  }

  switch (currentState) {
   case State::Main:
    mainScreen();
    break;
   case State::Calibrate:
    calScreen();
    break;
   case State::SavePrompt:
    saveScreen();
    break;
  }

  arduboy.display();
}

// --- Handle the main screen ---
void mainScreen() {
  uint8_t barLength = min(round(displayValue * WIDTH / SCALE_HIGH), WIDTH);

  if (arduboy.justPressed(B_BUTTON)) {
    currentState = State::Calibrate;
  }

  if (arduboy.justPressed(A_BUTTON)) {
    resetMinMax();
  }

  printStr_P(MAIN_HEADING_X, MAIN_HEADING_Y, StrMainHeading);

  arduboy.setCursor(MAIN_VALUE_X, MAIN_VALUE_Y);
  arduboy.setTextSize(3);
  printValue(displayValue);
  arduboy.setTextSize(1);

  if (adcReading != calMin) {
    if (adcReading < calMin) {
      printStr_P(MAIN_BAR_NEG_X, MAIN_BAR_NEG_Y, StrNegative);
    }
    else {
     arduboy.fillRect(MAIN_BAR_X, MAIN_BAR_Y, barLength, BAR_HEIGHT);
    }
    if (adcReading > calMax) {
      arduboy.setTextColor(BLACK);
      arduboy.setTextBackground(WHITE);
      printStr_P(MAIN_BAR_OVL_X, MAIN_BAR_OVL_Y, StrOverflow);
      arduboy.setTextColor(WHITE);
      arduboy.setTextBackground(BLACK);
    }
  }

  printStr_P(MAIN_MIN_X, MAIN_MIN_Y, StrMin);
  printValue(displayMin);
  printStr_P(MAIN_MAX_X, MAIN_MAX_Y, StrMax);
  printValue(displayMax);

  printStr_P(MAIN_BTN_RST_X, MAIN_BTN_RST_Y, StrMainButtonRst);
  printStr_P(MAIN_BTN_CAL_X, MAIN_BTN_CAL_Y, StrMainButtonCal);
}

// --- Handle the calibration screen ---
void calScreen() {
  if ((delayCount != 0) && (--delayCount == 0)) {
    repeating = true;
  }

  if (arduboy.justPressed(A_BUTTON)) {
    currentState = State::Main;
  }

  if (arduboy.justPressed(B_BUTTON)) {
    currentState = State::SavePrompt;
  }

  if (arduboy.justPressed(LEFT_BUTTON)) {
    calMaxSelected = !calMaxSelected;
  }

  if (arduboy.justPressed(RIGHT_BUTTON)) {
    updateCalField(adcReading);
  }

  if (arduboy.justPressed(UP_BUTTON)) {
    calFieldInc();
    startButtonDelay();
  }
  else if (arduboy.justPressed(DOWN_BUTTON)) {
    calFieldDec();
    startButtonDelay();
  }
  else if (repeating && arduboy.pressed(UP_BUTTON)) {
    calFieldInc();
  }
  else if (repeating && arduboy.pressed(DOWN_BUTTON)) {
    calFieldDec();
  }
  else if (repeating) {
    stopButtonRepeat();
  }

  printStr_P(CAL_HEADING_X, CAL_HEADING_Y, StrCalHeading);

  printStr_P(CAL_MIN_X, CAL_MIN_Y, StrCalMin);
  arduboy.print(calMin);
  printStr_P(CAL_MAX_X, CAL_MAX_Y, StrCalMax);
  arduboy.print(calMax);

  // Set the cal field cursor
  if (calMaxSelected) {
    arduboy.setCursor(CAL_MAX_X - CHAR_WIDTH, CAL_MAX_Y);
  }
  else {
    arduboy.setCursor(CAL_MIN_X - CHAR_WIDTH, CAL_MIN_Y);
  }
  arduboy.print('\x10');

  printStr_P(CAL_RAW_X, CAL_RAW_Y, StrRawReading);
  arduboy.print(adcReading);
  printStr_P(CAL_DISPLAYED_X, CAL_DISPLAYED_Y, StrDispReading);
  arduboy.print(displayValue, 1);

  printStr_P(CAL_BUTTON_SEL_X, CAL_BUTTON_SEL_Y, StrCalButtonSel);
  printStr_P(CAL_BUTTON_COPY_X, CAL_BUTTON_COPY_Y, StrCalButtonCopy);
  printStr_P(CAL_BUTTON_INC_X, CAL_BUTTON_INC_Y, StrCalButtonInc);
  printStr_P(CAL_BUTTON_DEC_X, CAL_BUTTON_DEC_Y, StrCalButtonDec);
  printStr_P(CAL_BUTTON_EXIT_X, CAL_BUTTON_EXIT_Y, StrCalButtonExit);
  printStr_P(CAL_BUTTON_SAVE_X, CAL_BUTTON_SAVE_Y, StrCalButtonSave);
}

// --- Handle the calibration save screens ---
void saveScreen() {
  arduboy.setTextSize(2);
  printStr_P(SAVE_Q_1_X, SAVE_Q_1_Y, StrSaveQ1);
  printStr_P(SAVE_Q_2_X, SAVE_Q_2_Y, StrSaveQ2);
  arduboy.setTextSize(1);

  printStr_P(SAVE_BUTTON_YES_X, SAVE_BUTTON_YES_Y, StrSaveButtonYes);
  printStr_P(SAVE_BUTTON_NO_X, SAVE_BUTTON_NO_Y, StrSaveButtonNo);

  if (arduboy.justPressed(B_BUTTON)) {
    currentState = State::Calibrate;
  }

  if (arduboy.justPressed(A_BUTTON)) {
    saveCal();
    arduboy.clear();
    arduboy.setTextSize(2);
    printStr_P(SAVE_SAVED_X, SAVE_SAVED_Y, StrSaved);
    arduboy.setTextSize(1);
    arduboy.display();
    arduboy.delayShort(2000);
    currentState = State::Calibrate;
  }
}

// Reset the minimum and maximum readings by setting them to extreme opposite
// values. They will then be set to the current reading during the next loop.
void resetMinMax() {
  displayMin = 88888.8;
  displayMax = -88888.8;
}

// Get saved calibration values from EEPROM.
// Initialise EEPROM if "magic" ID is not correct.
void getSavedCal() {
  if ((EEPROM.read(LIGHT_METER_EEPROM_ID1) == STORAGE_ID_1) &&
      (EEPROM.read(LIGHT_METER_EEPROM_ID2) == STORAGE_ID_2)) {
    EEPROM.get(LIGHT_METER_EEPROM_CAL_MIN, calMin);
    EEPROM.get(LIGHT_METER_EEPROM_CAL_MAX, calMax);
  }
  else {
    EEPROM.update(LIGHT_METER_EEPROM_ID1, STORAGE_ID_1);
    EEPROM.update(LIGHT_METER_EEPROM_ID2, STORAGE_ID_2);
    calMin = DEFAULT_CAL_MIN;
    calMax = DEFAULT_CAL_MAX;
    saveCal();
  }
}

// Save the calibration data to EEPROM
void saveCal() {
  EEPROM.put(LIGHT_METER_EEPROM_CAL_MIN, calMin);
  EEPROM.put(LIGHT_METER_EEPROM_CAL_MAX, calMax);
}

// Start the button auto-repeat delay
void startButtonDelay() {
  delayCount = DELAY_FRAMES;
  repeating = false;
}

// Stop the button auto-repeat or delay
void stopButtonRepeat() {
  delayCount = 0;
  repeating = false;
}

// Increase the value of the selected calibration field based on the button
// repeat state
void calFieldInc() {
  int newval = (calMaxSelected ? calMax : calMin) +
                (repeating ? (REPEAT_CHANGE) : 1);

  updateCalField(newval);
}

// Decrease the value of the selected calibration field based on the button
// repeat state
void calFieldDec() {
  int newVal = (calMaxSelected ? calMax : calMin) -
                (repeating ? (REPEAT_CHANGE) : 1);

  updateCalField(newVal);
}

// Update the selected calibration field with the provided value, if allowed
void updateCalField(int newVal) {
  if (calMaxSelected) {
    if (newVal > calMin) {
      calMax = newVal;
    }
  }
  else {
    if (newVal < calMax) {
      calMin = newVal;
    }
  }
}

// Format and print the calculated value at the current cursor location
void printValue(float value) {
  float absValue = abs(value);

  if (value >= 0) {
    arduboy.print(' ');
  }

  for (float i = 10; i < 1000; i *= 10) {
    if (absValue < i) {
      arduboy.print(' ');
    }
  }
  arduboy.print(value, 1);
}

// Print a string in program memory at the given location
void printStr_P(int x, int y, const char* str) {
  arduboy.setCursor(x, y);
  arduboy.print((__FlashStringHelper*)(str));
}

// Calculate the X coordinate to center a string located in program memory
int centerStr_P(const char* str) {
  return (WIDTH / 2) - (strlen_P(str) * CHAR_WIDTH / 2);
}

// Calculate the X coordinate to center a size 2 string located in
// program memory
int centerStr2_P(const char* str) {
  return (WIDTH / 2) - (strlen_P(str) * CHAR_WIDTH);
}

// Calculate the X coordinate to right justify a string in program memory
int rightStr_P(const char* str) {
  return WIDTH - (strlen_P(str) * CHAR_WIDTH) + 1;
}

