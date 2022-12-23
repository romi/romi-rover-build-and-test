#include "State.h"

State::State()
        : state(STATE_STARTING_UP),
          error(ERROR_NONE),
          error_message(""),
          battery_state(STATE_BATTERY_OK), // Assume all OK
          voltage(-1.0f),
          blink_on(false),
          interval(-1),
          last_time(0)
{
}

void State::init()
{
	FastLED.addLeds<NEOPIXEL, PIN_ADDR_LED>(leds, NUM_LEDS);
        set(STATE_STARTING_UP);
}

void State::set(DeviceState s)
{
        Serial.print("State: ");
        Serial.println(state);        
        state = s;
        update_leds();
        delay(250); // Make sure it's seen
}

void State::set_error(DeviceError err, const char *message)
{
        error = err;
        error_message = message;
        set(STATE_ERROR);
        Serial.print("Error: ");
        Serial.println(message);
}

void State::clear_error()
{
        set(STATE_READY); // TODO: is this right?
        error = ERROR_NONE;
        error_message = "";
}

void State::update_leds()
{
        switch (state) {
        case STATE_ERROR:
                update_error_leds();
                break;
        case STATE_STARTING_UP:
                set_leds(CRGB::Blue, CRGB::Black, CRGB::Black);
                break;
        case STATE_INITIALIZING_DEBUG_SERIAL:
                set_leds(CRGB::Blue, CRGB::Blue, CRGB::Black);
                break;
        case STATE_INITIALIZING_CAMERA_SERIAL:
                set_leds(CRGB::Blue, CRGB::Blue, CRGB::Blue);
                break;
        case STATE_INITIALIZING_ODRIVE:
                set_leds(CRGB::Orange, CRGB::Black, CRGB::Black);
                break;
        case STATE_INITIALIZING_POSITION:
                set_leds(CRGB::Orange, CRGB::Orange, CRGB::Black);
                break;
        case STATE_INITIALIZING_SWITCHES:
                set_leds(CRGB::Orange, CRGB::Orange, CRGB::Orange);
                break;
        case STATE_READY:
                set_leds(CRGB::Green, CRGB::Black, CRGB::Black);
                break;
        }
}

void State::update_error_leds()
{
        switch (error) {
        case ERROR_NONE:
                // Should never happen!
                set_leds(CRGB::Green, CRGB::Black, CRGB::Black);
                break;
        case ERROR_ODRIVE:
                set_leds(CRGB::Red, CRGB::Black, CRGB::Black);
                break;
        case ERROR_BATTERY:
                set_leds(CRGB::Red, CRGB::Red, CRGB::Black);
                break;
        }
}

void State::set_leds(const struct CRGB &color0,
                     const struct CRGB &color1,
                     const struct CRGB &color2)
{
        leds[0] = color0;
        leds[1] = color1;
        leds[2] = color2;        
        FastLED.show();
        delay(1000);
}

void State::clear_leds()
{
        leds[0] = CRGB::Black;
        leds[1] = CRGB::Black;
        leds[2] = CRGB::Black;        
        FastLED.show();
}

void State::set_battery(float value)
{
        voltage = value;
        if (voltage >= 16.0f) {
                battery_state = STATE_BATTERY_OK;
                interval = 0;
        } else if (15.5f <= voltage && voltage < 16.0f) {
                battery_state = STATE_BATTERY_LOW;
                interval = 2000;
        } else {
                battery_state = STATE_BATTERY_CRITICAL;
                interval = 500;
        }
}

void State::handle_leds()
{
        if (interval > 0)
                check_leds_timeout();
}

void State::check_leds_timeout()
{
        unsigned long now = millis();
        if (now - last_time > interval) {
                blink_leds();
                last_time = now;
        }
}

void State::blink_leds()
{
        if (blink_on) {
                clear_leds();
                blink_on = false;
        } else {
                update_leds();
                blink_on = true;
        }
}

