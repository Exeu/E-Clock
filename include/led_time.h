#ifndef led_time_h
#define led_time_h

#include <Wire.h> // must be included here so that Arduino library object file references work
#include <Arduino.h>
#include <RtcDS3231.h>
#include <TimeLib.h>

RtcDS3231<TwoWire> Rtc(Wire);
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay(){

  Serial.print("System current: ");
  // digital clock display of the time
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

class LedTime {
    private:

    public:
        void init() {
            Rtc.Begin();

            Serial.print("compiled: ");
            Serial.print(__DATE__);
            Serial.println(__TIME__);

            //--------RTC SETUP ------------
            // if you are using ESP-01 then uncomment the line below to reset the pins to
            // the available pins for SDA, SCL
            // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL
            
            Rtc.Begin();
            Serial.println("Begin RTC....");

            RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
            Serial.println();

            if (!Rtc.IsDateTimeValid()) 
            {
                if (Rtc.LastError() != 0)
                {
                    // we have a communications error
                    // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
                    // what the number means
                    Serial.print("RTC communications error = ");
                    Serial.println(Rtc.LastError());
                }
                else
                {
                    // Common Causes:
                    //    1) first time you ran and the device wasn't running yet
                    //    2) the battery on the device is low or even missing

                    Serial.println("RTC lost confidence in the DateTime!");

                    // following line sets the RTC to the date & time this sketch was compiled
                    // it will also reset the valid flag internally unless the Rtc device is
                    // having an issue

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
        };

        RtcDateTime getCurrentRtcDateTime() {
            return Rtc.GetDateTime();
        }
};

#endif