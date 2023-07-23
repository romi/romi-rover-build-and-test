#ifndef _CABLEBOTMOTORCONTROLLER_STATE_H
#define _CABLEBOTMOTORCONTROLLER_STATE_H

#include <FastLED.h>
#include "IState.h"

// Output pins
#define PIN_ADDR_LED 5

class State : public IState
{
protected:
        static const int NUM_LEDS = 3;
        
        CRGB leds_[NUM_LEDS];
        DeviceState state_;
	DeviceError error_;
        const char *error_message_;
	BatteryState battery_state_;
        float voltage_;
        bool blink_on_;
        unsigned long interval_;
        unsigned long last_time_;

public:
        
        State();
        ~State() override = default;

        void init() override;
        void set(DeviceState s) override;
        void set_error(DeviceError error, const char *message) override;
        DeviceError error() const override;
        const char *message() const override;
        void set_battery(float voltage) override;
        void clear_error() override;
        void update() override;

protected:
        void update_leds();
        void update_error_leds();
        void set_leds(const struct CRGB &color0,
                      const struct CRGB &color1,
                      const struct CRGB &color2);
        void clear_leds();
        void blink_leds();
        void check_leds_timeout();
};
        
        

#endif // _CABLEBOTMOTORCONTROLLER_STATE_H


