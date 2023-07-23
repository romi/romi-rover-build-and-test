#include "State.h"

static const char *kNoErrorMessage = "No error";

State::State()
        : state_(STATE_STARTING_UP),
          error_(ERROR_NONE),
          error_message_(kNoErrorMessage),
          battery_state_(STATE_BATTERY_OK), // Assume all OK
          voltage_(-1.0f),
          blink_on_(false),
          interval_(-1),
          last_time_(0)
{
}

void State::init()
{
	FastLED.addLeds<NEOPIXEL, PIN_ADDR_LED>(leds_, NUM_LEDS);
        set(STATE_STARTING_UP);
}

void State::set(DeviceState s)
{
        Serial.print("#!State: ");
        Serial.println(state_);        
        state_ = s;
        update_leds();
        delay(250); // Make sure it's seen
}

void State::set_error(DeviceError err, const char *message)
{
        error_ = err;
        error_message_ = message;
        set(STATE_ERROR);
        Serial.print("#!Error: ");
        Serial.println(message);
}

DeviceError State::error() const
{
        return error_;
}

const char *State::message() const
{
        return error_message_;
}

void State::clear_error()
{
        set(STATE_READY); // TODO: is this right?
        error_ = ERROR_NONE;
        error_message_ = kNoErrorMessage;
}

void State::update_leds()
{
        switch (state_) {
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
        case STATE_INITIALIZING_MOTORCONTROLLER:
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
        switch (error_) {
        case ERROR_NONE:
                // Should never happen!
                set_leds(CRGB::Green, CRGB::Black, CRGB::Black);
                break;
        case ERROR_ODRIVE:
                set_leds(CRGB::Red, CRGB::Black, CRGB::Black);
                break;
        case ERROR_BATTERY:
                set_leds(CRGB::Black, CRGB::Red, CRGB::Black);
                break;
        case ERROR_HOMING:
                set_leds(CRGB::Black, CRGB::Black, CRGB::Red);
                break;
        }
}

void State::set_leds(const struct CRGB &color0,
                     const struct CRGB &color1,
                     const struct CRGB &color2)
{
        leds_[0] = color0;
        leds_[1] = color1;
        leds_[2] = color2;        
        FastLED.show();
        delay(1000);
}

void State::clear_leds()
{
        leds_[0] = CRGB::Black;
        leds_[1] = CRGB::Black;
        leds_[2] = CRGB::Black;        
        FastLED.show();
}

void State::set_battery(float value)
{
        voltage_ = value;
        if (voltage_ >= 14.0f) {
                battery_state_ = STATE_BATTERY_OK;
                interval_ = 0;
        } else if (13.0f <= voltage_ && voltage_ < 14.0f) {
                battery_state_ = STATE_BATTERY_LOW;
                interval_ = 2000;
        } else {
                battery_state_ = STATE_BATTERY_CRITICAL;
                interval_ = 500;
        }
}

void State::update()
{
        if (interval_ > 0)
                check_leds_timeout();
}

void State::check_leds_timeout()
{
        unsigned long now = millis();
        if (now - last_time_ > interval_) {
                blink_leds();
                last_time_ = now;
        }
}

void State::blink_leds()
{
        if (blink_on_) {
                clear_leds();
                blink_on_ = false;
        } else {
                update_leds();
                blink_on_ = true;
        }
}

