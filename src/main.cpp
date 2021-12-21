
#include <Arduino.h>
#include "TimeLib.h"
#include "led_clock.h"
#include "led_time.h"
#include "rotary_switch.h"
#include "programm_mode.h"

#define DEBUG_MODE 

LedTime *ledTime;
LedClock *ledClock;
RotarySwitch *rotarySwitch;
ProgrammMode *programmMode;

// Button

void click() {
    rotarySwitch->isr0();
};

time_t syncSysTime() {
  RtcDateTime currentRtcTime = ledTime->getCurrentRtcDateTime();
  return currentRtcTime.Epoch32Time();
}

// ##############

void setup() {
  Serial.begin(9600);

  ledTime = new LedTime();
  ledTime->init();
  
  ledClock = new LedClock(2);
  ledClock->init();

  setSyncProvider(syncSysTime);
  delay(1000);

  rotarySwitch = new RotarySwitch(10);
  attachInterrupt(digitalPinToInterrupt(10), click, interrupt_trigger_type);
  rotarySwitch->setInitComplete();

  programmMode = new ProgrammMode(5, 6, *rotarySwitch, *ledClock);
  programmMode->begin();
}

void loop() {
  static long prevMillisClockLoop = 0;
  static long prevMillisBrightnessLoop = 0;

  long currenMillis = millis();

  programmMode->programmMode();

  if (currenMillis - prevMillisClockLoop > 1000 && !programmMode->isInProgrammMode()) {
    prevMillisClockLoop = currenMillis;
    ledClock->displayTime(hourFormat12(), minute(), second());
    
#ifdef DEBUG_MODE
    ledTime->digitalClockDisplay();
#endif
  }

  if (currenMillis - prevMillisBrightnessLoop > 3600000 && !programmMode->isInProgrammMode()) {
      prevMillisBrightnessLoop = currenMillis;
      ledClock->saveBrightnessToEEPROM();
  }

  delay(5);
}

