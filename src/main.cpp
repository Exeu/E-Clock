/**
 * Serial commands:
 * 
 * C0x00FF00,0xFF0000,0x0000FF -> Update led color C<HOUR>,<MINUTE>,<SECOND>
 * G -> Gets current time settings
 * T1357041600 -> Sets current time to unix timestamp T<TIMESTAMP>
 */


#include <Arduino.h>
#include "TimeLib.h"
#include "led_clock.h"
#include "led_time.h"
#include "rotary_switch.h"
#include "programm_mode.h"
#include "main.h"

//#define DEBUG_MODE 
#define TIME_HEADER  "T"  
#define GET_TIME_HEADER "G"
#define COLOR_HEADER "C"

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

  if (Serial.available()) {
    processSyncMessage();
  }

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
}

void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600;

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime >= DEFAULT_TIME) {
       setTime(pctime);
       RtcDateTime newDateTime = RtcDateTime(year() - 2000, month(), day(), hour(), minute(), second());
       Rtc.SetDateTime(newDateTime);
       Serial.println("Set time");
     }
  } else if (Serial.find(GET_TIME_HEADER)) {
      ledTime->digitalClockDisplay();
      Serial.println("");
      RtcDateTime currentRtcTime = ledTime->getCurrentRtcDateTime();
      ledTime->printDateTime(currentRtcTime);
  } else if (Serial.find(COLOR_HEADER)) {
      String transmittedColorConfigs = Serial.readString();

      String hour = transmittedColorConfigs.substring(0, 8);
      String minute = transmittedColorConfigs.substring(9, 17);
      String second = transmittedColorConfigs.substring(18, 27);
      
      ledClock->updateColors(strtol(hour.c_str(), NULL, 16), strtol(minute.c_str(), NULL, 16), strtol(second.c_str(), NULL, 16));
  }
}

