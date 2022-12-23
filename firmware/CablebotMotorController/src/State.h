#ifndef __MOTORCONTROLLER_STATE_H
#define __MOTORCONTROLLER_STATE_H

#include <FastLED.h>

// Output pins
#define PIN_ADDR_LED 5

enum DeviceState {
        STATE_ERROR,
        STATE_STARTING_UP,
        STATE_INITIALIZING_DEBUG_SERIAL,
        STATE_INITIALIZING_CAMERA_SERIAL,
        STATE_INITIALIZING_ODRIVE,
        STATE_INITIALIZING_POSITION,
        STATE_INITIALIZING_SWITCHES,
        STATE_READY
};

enum DeviceError {
        ERROR_NONE = 0,
        ERROR_ODRIVE,
        ERROR_BATTERY
};

enum BatteryState {
        STATE_BATTERY_OK,
        STATE_BATTERY_LOW,
        STATE_BATTERY_CRITICAL
};

class State
{
public:
        static const int NUM_LEDS = 3;
        
        CRGB leds[NUM_LEDS];
        DeviceState state;
	DeviceError error;
        const char *error_message;
	BatteryState battery_state;
        float voltage;
        bool blink_on;
        unsigned long interval;
        unsigned long last_time;
        
        State();
        virtual ~State() = default;

        void init();
        void set(DeviceState s);
        void set_error(DeviceError error, const char *message);
        void set_battery(float voltage);
        void clear_error();
        void handle_leds();

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
        
        

#endif // __MOTORCONTROLLER_STATE_H


