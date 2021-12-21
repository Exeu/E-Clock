#ifndef led_time_h
#define led_time_h

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <Arduino.h>
#include <RtcDS3231.h>
#include <TimeLib.h>

RtcDS3231<TwoWire> Rtc(Wire);
#define countof(a) (sizeof(a) / sizeof(a[0]))



class LedTime {
    private:

    public:
        void init() {
            Rtc.Begin();

            Serial.print("compiled: ");
            Serial.print(__DATE__);
            Serial.println(__TIME__);

            Rtc.Begin();
            Serial.println("Begin RTC....");

            RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
            Serial.println();

            if (!Rtc.IsDateTimeValid()) 
            {
                if (Rtc.LastError() != 0)
                {
                    Serial.print("RTC communications error = ");
                    Serial.println(Rtc.LastError());
                }
                else
                {
                    Serial.println("RTC lost confidence in the DateTime!");
                    Rtc.SetDateTime(compiled);
                }
            }

            if (!Rtc.GetIsRunning())
            {
                Serial.println("RTC was not actively running, starting now");
                Rtc.SetIsRunning(true);
            }

            RtcDateTime now = Rtc.GetDateTime();
            if (now < compiled) 
            {
                Serial.println("RTC is older than compile time!  (Updating DateTime)");
                //Rtc.SetDateTime(compiled);
            }
            else if (now > compiled) 
            {
                Serial.println("RTC is newer than compile time. (this is expected)");
            }
            else if (now == compiled) 
            {
                Serial.println("RTC is the same as compile time! (not expected but all is fine)");
            }

            // never assume the Rtc was last configured by you, so
            // just clear them to your needed state
            Rtc.Enable32kHzPin(false);
            Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
            
            // Updating the system time according to the current set dtc time.
            RtcDateTime currentRtcTime = this->getCurrentRtcDateTime();
            setTime(currentRtcTime.Epoch32Time());

            printDateTime(currentRtcTime);
        };

        RtcDateTime getCurrentRtcDateTime() {
            return Rtc.GetDateTime();
        }

        void printDigits(int digits) {
            Serial.print(":");
            if(digits < 10)
                Serial.print('0');
            Serial.print(digits);
        }

        void digitalClockDisplay() {
            Serial.print("System current: ");
            Serial.print(hour());
            printDigits(minute());
            printDigits(second());
            Serial.print(" ");
            Serial.print(day());
            Serial.print(" ");
            Serial.print(month());
            Serial.print(" ");
            Serial.print(year()); 
            Serial.println(); 
        }

        void printDateTime(const RtcDateTime& dt) {
            char datestring[20];
            Serial.print("Current RTC Time: ");
            snprintf_P(datestring, 
                    countof(datestring),
                    PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
                    dt.Month(),
                    dt.Day(),
                    dt.Year(),
                    dt.Hour(),
                    dt.Minute(),
                    dt.Second() );
            Serial.println(datestring);
        }
};

#endif