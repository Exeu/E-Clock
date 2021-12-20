#ifndef rotary_switch_h
#define rotary_switch_h

#include <Arduino.h>

#define switched                            true // value if the button switch has been pressed
#define triggered                           true // controls interrupt handler
#define interrupt_trigger_type            RISING // interrupt triggered on a RISING input
#define debounce                              10 // time to wait in milli secs

class RotarySwitch {
    private:
        int button_switch; // external interrupt pin
        volatile bool interrupt_process_status = { !triggered }; // start with no switch press pending, ie false (!triggered)
        bool initialisation_complete = false; // inhibit any interrupts until initialisation is complete
        int nbClicks = 0;
    public:
        RotarySwitch(int switchPin) {
            button_switch = switchPin;
        };

        bool read_button() {
            int button_reading;
            static bool     switching_pending = false;
            static long int elapse_timer;
            if (interrupt_process_status == triggered) {
                button_reading = digitalRead(button_switch);
                if (button_reading == HIGH) {
                    // switch is pressed, so start/restart wait for button relealse, plus end of debounce process
                    switching_pending = true;
                    elapse_timer = millis(); // start elapse timing for debounce checking
                }
                if (switching_pending && button_reading == LOW) {
                    if (millis() - elapse_timer >= debounce) {
                        switching_pending = false;             // reset for next button press interrupt cycle
                        interrupt_process_status = !triggered; // reopen ISR for business now button on/off/debounce cycle complete
                        nbClicks++;
                        return switched;                       // advise that switch has been pressed
                    }
                }
            }
            return !switched;
        };

        void setInitComplete() {
            initialisation_complete = true;
        };

        int getNbClicks() {
            return nbClicks;
        };

        void resetNbClicks() {
            nbClicks = 0;
        }

        void isr0() {
            if (initialisation_complete == false) {
                return;
            }

            if (interrupt_process_status == !triggered) {
                if (digitalRead(button_switch) == HIGH) {
                    interrupt_process_status = triggered; 
                }
            }
        };
};
#endif
