#ifndef led_clock_h
#define led_clock_h

#include "FastLED.h"
#include <EEPROM.h>

class LedClock {
    private:
        int ledPin_;
        CRGB leds[60];
        int hourMarkers[4] = {0, 15, 30, 45};
        int brightnessPos;
    public:
        LedClock(int ledPin) {
            ledPin_ = ledPin;
        };

        void init() {
            EEPROM.get(0, brightnessPos);
            Serial.println(brightnessPos);
            if (brightnessPos == 1) {
                brightnessPos = 255;
                EEPROM.put(0, brightnessPos);
            }

            FastLED.addLeds<WS2812B, 2, GRB>(this->leds, 60);
            FastLED.setBrightness(brightnessPos);
            FastLED.clear();
            FastLED.show();
        }

        void displayTime(int hour, int minute, int second) {
            FastLED.clear();

            this->insertHourMarkers();

            if (hour == 12) {
                hour = 0;
            }

            int currentSecondPos = second;
            int currentMinutePos = minute;
            int currentHourPos = (hour * 5);

            leds[currentMinutePos] = CRGB::Blue;
            leds[currentHourPos] = CRGB::Green;
            leds[currentSecondPos] = CRGB::Red;

            FastLED.show();
        }

        void displayTime(int hour) {
            FastLED.clear();

            this->insertHourMarkers();

            if (hour == 12) {
                hour = 0;
            }
            int currentHourPos = (hour * 5);

            leds[currentHourPos] = CRGB::Green;

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

            leds[currentMinutePos] = CRGB::Blue;
            leds[currentHourPos] = CRGB::Green;

            FastLED.show();
        }

        void insertHourMarkers() {
            for (int s = 0; s < 4; s++) {
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
            FastLED.setBrightness(brightnessPos);
        }

        void saveBrightnessToEEPROM() {
            Serial.println("save value to eeprom");
            EEPROM.update(0, brightnessPos);
        }
};

#endif