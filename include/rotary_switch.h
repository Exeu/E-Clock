#ifndef rotary_switch_h
#define rotary_switch_h

#include <Arduino.h>

#define switched true
#define triggered true
#define interrupt_trigger_type RISING
#define debounce 10

class RotarySwitch
{
private:
    int button_switch;
    volatile bool interrupt_process_status = {!triggered};
    bool initialisation_complete = false;
    int nbClicks = 0;

public:
    RotarySwitch(int switchPin)
    {
        button_switch = switchPin;
    };

    bool read_button()
    {
        int button_reading;
        static bool switching_pending = false;
        static long int elapse_timer;
        if (interrupt_process_status == triggered)
        {
            button_reading = digitalRead(button_switch);
            if (button_reading == HIGH)
            {
                switching_pending = true;
                elapse_timer = millis();
            }
            if (switching_pending && button_reading == LOW)
            {
                if (millis() - elapse_timer >= debounce)
                {
                    switching_pending = false;             
                    interrupt_process_status = !triggered;
                    nbClicks++;
                    return switched;
                }
            }
        }
        return !switched;
    };

    void setInitComplete()
    {
        initialisation_complete = true;
    };

    int getNbClicks()
    {
        return nbClicks;
    };

    void resetNbClicks()
    {
        nbClicks = 0;
    }

    void isr0()
    {
        if (initialisation_complete == false)
        {
            return;
        }

        if (interrupt_process_status == !triggered)
        {
            if (digitalRead(button_switch) == HIGH)
            {
                interrupt_process_status = triggered;
            }
        }
    };
};
#endif
