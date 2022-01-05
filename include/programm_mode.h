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

    static ProgrammMode* anchor_;
    RotaryEncoder *encoder_ = nullptr;
    byte pinA_;
    byte pinB_;
    bool programmMode_ = false;
    int currentMode_ = 0;
    
    int programmedHour_ = 0;
    int programmedMinute_ = 0;

    void isr() {
      encoder_->tick();
    }
    
    static void marshall() {
      anchor_->isr();
    }

    void mode1() {
        ledClock_->insertHourMarkers();

        int encoderPosition = encoder_->getPosition();
        if (encoderPosition < 0) {
            encoderPosition = 0;
            encoder_->setPosition(encoderPosition);
        }
        if (encoderPosition > 11) {
            encoderPosition = 11;
            encoder_->setPosition(11);
        }

        ledClock_->displayTime(encoderPosition);
        programmedHour_ = encoderPosition;
    }

    void mode2() {
        ledClock_->insertHourMarkers();

        int encoderPosition = encoder_->getPosition();
        if (encoderPosition < 0) {
            encoderPosition = 0;
            encoder_->setPosition(encoderPosition);
        }
        if (encoderPosition > 59) {
            encoderPosition = 59;
            encoder_->setPosition(59);
        }

        ledClock_->displayTime(programmedHour_, encoderPosition);
        programmedMinute_ = encoderPosition;
    }

  public:
    ProgrammMode(byte pinA, byte pinB, RotarySwitch &rotarySwitch, LedClock &ledClock): rotarySwitch_(&rotarySwitch), ledClock_(&ledClock) {
        anchor_ = this;
        pinA_ = pinA;
        pinB_ = pinB;
    }

    void begin() {
        encoder_ = new RotaryEncoder(pinA_, pinB_, RotaryEncoder::LatchMode::FOUR0);

        attachInterrupt(digitalPinToInterrupt(pinA_), ProgrammMode::marshall, CHANGE);
        attachInterrupt(digitalPinToInterrupt(pinB_), ProgrammMode::marshall, CHANGE);

        encoder_->setPosition(ledClock_->getCurrentBrightness());
    }

    int getEncoderPos() {
        return encoder_->getPosition();
    }

    RotaryEncoder::Direction getEncoderDirection() {
        return encoder_->getDirection();
    }

    bool isInProgrammMode() {
        return currentMode_ > 0;
    }

    // programmMode Loop
    void programmMode() {
        if (rotarySwitch_->read_button() == switched) {
            Serial.println("switched");
            int clicks = rotarySwitch_->getNbClicks();
            Serial.println(clicks);

            if (clicks == 1) {
                this->currentMode_ = 1;
                int currHour = hourFormat12();
                if (currHour == 12) {
                    currHour = 0;
                }

                encoder_->setPosition(currHour);
                Serial.println("Progmode 1");
                Serial.println(currentMode_);
            }

            if (clicks == 2) {
                this->currentMode_ = 2;
                Serial.println("Progmode 2");
                Serial.println(currentMode_);
            }

            if (clicks == 3) {
                Serial.println("Return");
                Serial.println(currentMode_);

                this->currentMode_ = 0;
                rotarySwitch_->resetNbClicks();

                RtcDateTime newDateTime = RtcDateTime(year() - 2000, month(), day(), programmedHour_, programmedMinute_, 0);
                Rtc.SetDateTime(newDateTime);
                setTime(newDateTime.Epoch32Time());

                encoder_->setPosition(ledClock_->getCurrentBrightness());
            }

            return;
        }

        if (isInProgrammMode()) {
            switch (currentMode_) {
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
            int encoderPos = encoder_->getPosition();

            if (encoderPos < 1) {
                encoderPos = 1;
                encoder_->setPosition(encoderPos);
            }

            if (encoderPos > 17) {
                encoderPos = 17;
                encoder_->setPosition(encoderPos);
            }

            encoderPos = encoderPos * 15;
            if (encoderPos != currentBrightness) {
                ledClock_->updateBrightness(encoderPos);
                ledClock_->displayTime(hourFormat12(), minute(), second());
            }
        }
    }
};

ProgrammMode* ProgrammMode::anchor_ = NULL;