#ifndef led_clock_h
#define led_clock_h

#include "FastLED.h"
#include <EEPROM.h>

#define DEFAULT_COLOR_HOUR 0x00FF00
#define DEFAULT_COLOR_MINUTE 0x0000FF
#define DEFAULT_COLOR_SECOND 0xFF0000

#define EEPROM_NOTSET 0xFFFFFFFF

struct ColorConfig {
    uint32_t hour;
    uint32_t minute;
    uint32_t second;
};

class LedClock {
    private:
        int ledPin_;
        CRGB leds[60];
        int hourMarkers[4] = {0, 15, 30, 45};
        int brightnessPos;
        ColorConfig colorConfig_ = { DEFAULT_COLOR_HOUR, DEFAULT_COLOR_MINUTE, DEFAULT_COLOR_SECOND };
    public:
        LedClock(int ledPin) {
            ledPin_ = ledPin;
        };

        void init() {
            EEPROM.get(10, colorConfig_);
            if (colorConfig_.hour == EEPROM_NOTSET && colorConfig_.minute == EEPROM_NOTSET && colorConfig_.second == EEPROM_NOTSET) {
                Serial.println("No color config found in EEPRO... Setting default.");
                colorConfig_ = { DEFAULT_COLOR_HOUR, DEFAULT_COLOR_MINUTE, DEFAULT_COLOR_SECOND };
                EEPROM.put(10, colorConfig_);
            }

            Serial.println("Color config: ");
            Serial.println(colorConfig_.hour);
            Serial.println(colorConfig_.minute);
            Serial.println(colorConfig_.second);
            
            EEPROM.get(0, brightnessPos);
            //Serial.println(brightnessPos);
            if (brightnessPos == -1) {
                brightnessPos = 255;
                EEPROM.put(0, brightnessPos);
            }

            FastLED.addLeds<WS2812B, 2, GRB>(this->leds, 60);
            FastLED.clear();
            FastLED.show();
        }

        void displayTime(int hour, int minute, int second) {
            FastLED.clear();

            if (hour == 12) {
                hour = 0;
            }

            int currentSecondPos = second;
            int currentMinutePos = minute;
            int currentHourPos = (hour * 5);
            int briSubstraction = 255 - getCurrentBrightness();

            leds[currentMinutePos] = colorConfig_.minute;
            leds[currentMinutePos].subtractFromRGB(briSubstraction);

            leds[currentHourPos] = colorConfig_.hour;
            leds[currentHourPos].subtractFromRGB(briSubstraction);

            leds[currentSecondPos] = colorConfig_.second;
            leds[currentSecondPos].subtractFromRGB(briSubstraction);

            int previousSecond = currentSecondPos - 1;
            if (previousSecond < 0) {
                previousSecond = 59;
            }

            leds[previousSecond] = colorConfig_.second;
            leds[previousSecond].subtractFromRGB(250);

            int nextSecond = currentSecondPos + 1;
            if (nextSecond > 59) {
                nextSecond = 0;
            }

            leds[nextSecond] = colorConfig_.second;
            leds[nextSecond].subtractFromRGB(250);

            this->insertHourMarkers();

            FastLED.show();
        }

        void displayTime(int hour) {
            FastLED.clear();

            this->insertHourMarkers();

            if (hour == 12) {
                hour = 0;
            }

            int currentHourPos = (hour * 5);
            leds[currentHourPos] = colorConfig_.hour;
            FastLED.show();
        }

        void displayTime(int hour, int minute) {
            FastLED.clear();

            this->insertHourMarkers();

            if (hour == 12) {
                hour = 0;
            }

            int currentMinutePos = minute;
            int currentHourPos = (hour * 5);

            leds[currentHourPos] = colorConfig_.hour;
            leds[currentMinutePos] = colorConfig_.minute;
            
            FastLED.show();
        }

        void insertHourMarkers() {
            for (int s = 0; s < 4; s++) {
                if ((int) leds[hourMarkers[s]] > 0) {
                    continue;
                }

                leds[hourMarkers[s]] = CRGB::White;
                leds[hourMarkers[s]].subtractFromRGB(250);
            }
        }

        int getCurrentBrightness() {
            return brightnessPos;
        }

        void updateBrightness(int brightness) {
            if (brightness < 1 || brightness > 255) {
                return;
            }

            brightnessPos = brightness;
            Serial.println(brightnessPos);
        }

        void saveBrightnessToEEPROM() {
            Serial.println("save value to eeprom");
            EEPROM.update(0, brightnessPos);
        }

        void updateColors(uint32_t hour, uint32_t minute, uint32_t second) {
            Serial.println("Updateing colors..");
            Serial.println(hour);
            Serial.println(minute);
            Serial.println(second);
            
            colorConfig_.hour = hour;
            colorConfig_.minute = minute;
            colorConfig_.second = second;

            EEPROM.put(10, colorConfig_);
        }
};

#endif