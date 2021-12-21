#include <Arduino.h>
#include <RotaryEncoder.h>
#include <rotary_switch.h>
#include <led_clock.h>
#include <led_time.h>
#include <RtcDS3231.h>

class ProgrammMode {
  private:
    RotarySwitch *rotarySwitch_;
    LedClock *ledClock_;

    static ProgrammMode* anchor;
    RotaryEncoder *encoder = nullptr;
    byte pinA_;
    byte pinB_;
    bool _programmMode = false;
    int currentMode = 0;
    
    int programmedHour = 0;
    int programmedMinute = 0;

    void isr() {
      encoder->tick();
    }
    
    static void marshall() {
      anchor->isr();
    }

    void mode1() {
        ledClock_->insertHourMarkers();

        int encoderPosition = encoder->getPosition();
        if (encoderPosition < 0) {
            encoderPosition = 0;
            encoder->setPosition(encoderPosition);
        }
        if (encoderPosition > 11) {
            encoderPosition = 11;
            encoder->setPosition(11);
        }

        ledClock_->displayTime(encoderPosition);
        programmedHour = encoderPosition;
    }

    void mode2() {
        ledClock_->insertHourMarkers();

        int encoderPosition = encoder->getPosition();
        if (encoderPosition < 0) {
            encoderPosition = 0;
            encoder->setPosition(encoderPosition);
        }
        if (encoderPosition > 59) {
            encoderPosition = 59;
            encoder->setPosition(59);
        }

        ledClock_->displayTime(programmedHour, encoderPosition);
        programmedMinute = encoderPosition;
    }

  public:
    ProgrammMode(byte pinA, byte pinB, RotarySwitch &rotarySwitch, LedClock &ledClock): rotarySwitch_(&rotarySwitch), ledClock_(&ledClock) {
      anchor = this;
      pinA_ = pinA;
      pinB_ = pinB;
    }

    void begin() {
        encoder = new RotaryEncoder(pinA_, pinB_, RotaryEncoder::LatchMode::FOUR0);

        attachInterrupt(digitalPinToInterrupt(pinA_), ProgrammMode::marshall, CHANGE);
        attachInterrupt(digitalPinToInterrupt(pinB_), ProgrammMode::marshall, CHANGE);

        encoder->setPosition(ledClock_->getCurrentBrightness());
    }

    int getEncoderPos() {
        return encoder->getPosition();
    }

    RotaryEncoder::Direction getEncoderDirection() {
        return encoder->getDirection();
    }

    bool isInProgrammMode() {
        return currentMode > 0;
    }

    // programmMode Loop
    void programmMode() {
        if (rotarySwitch_->read_button() == switched) {
            Serial.println("switched");
            int clicks = rotarySwitch_->getNbClicks();
            Serial.println(clicks);

            if (clicks == 1) {
                this->currentMode = 1;
                int currHour = hourFormat12();
                if (currHour == 12) {
                    currHour = 0;
                }

                encoder->setPosition(currHour);
                Serial.println("Progmode 1");
                Serial.println(currentMode);

            }

            if (clicks == 2) {
                this->currentMode = 2;
                Serial.println("Progmode 2");
                Serial.println(currentMode);
            }

            if (clicks == 3) {
                Serial.println("Return");
                Serial.println(currentMode);

                this->currentMode = 0;
                rotarySwitch_->resetNbClicks();

                RtcDateTime newDateTime = RtcDateTime(year() - 2000, month(), day(), programmedHour, programmedMinute, 0);
                Rtc.SetDateTime(newDateTime);
                setTime(newDateTime.Epoch32Time());

                encoder->setPosition(ledClock_->getCurrentBrightness());
            }

            return;
        }

        if (isInProgrammMode()) {
            switch (currentMode) {
                case 1:
                    mode1();
                    break;
                case 2:
                    mode2();
                    break;
                default:
                    break;
            }
        } else {
            int currentBrightness = ledClock_->getCurrentBrightness();
            int encoderPos = encoder->getPosition();

            if (encoderPos < 1) {
                encoderPos = 1;
                encoder->setPosition(encoderPos);
            }

            if (encoderPos > 17) {
                encoderPos = 17;
                encoder->setPosition(encoderPos);
            }

            encoderPos = encoderPos * 15;

            if (encoderPos != currentBrightness) {
                ledClock_->updateBrightness(encoderPos);
            }
        }
    }
};

ProgrammMode* ProgrammMode::anchor = NULL;