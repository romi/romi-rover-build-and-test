#pragma once

#include <Arduino.h>

enum ButEvent {
	BUT_PRESS,
	BUT_RELEASE,
	BUT_CLICK,
	BUT_LONG_CLICK,
	BUT_DOUBLE_CLICK
};

typedef void (*but_function)(ButEvent);

// Add to your code:
// Button myButton = Button(BUTTON_PIN, myButton_function, [DEBOUNCE_TIME]);
// Inside setup(): attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), myButton.update, CHANGE);
// Declare a function: void myButton_function(ButEvent myEvent)

class Button {

	public:
		Button (uint16_t _pin, but_function _but_function, uint8_t _debounce_time=50) {
			pin = _pin;
			debounce_time = _debounce_time;
			function = _but_function;
		}
		uint16_t pin;
		uint32_t debounce_time;
		but_function function;

		void update();

	private:
		bool s;
		bool state;
		bool state_last;
		bool long_triggered = false;
		uint32_t change_t = 0;
		uint32_t press_t = 0;
		uint32_t release_t = 0;
		uint8_t press_count = 0;

		const uint32_t longPress_duration = 1000;
		const uint32_t doublePress_separation = 280;
};

void Button::update()
{
	s = !digitalRead(pin);

	uint32_t now = millis();

	// Record the time for the last change
	if (s != state_last) change_t = now;

	// If button state is stable we analize it
	else if (now - change_t > debounce_time) {

		// Changed state
		if (s != state) {
			state = s;

			// Button pressed
			if (state) {

				press_t = change_t;
				press_count++;
				if (press_count > 1) {
					press_count = 0;
					function(BUT_DOUBLE_CLICK);
				} else {

					function(BUT_PRESS);
					long_triggered = false;
				}

			// Button released
			} else {

				release_t = change_t;
				function(BUT_RELEASE);
			}

		// State remains (only when called from inside the loop)
		} else {
			// Button remains pressed
			if (state) {

				if (now - press_t > longPress_duration && !long_triggered) {
					long_triggered = true;
					function(BUT_LONG_CLICK);
				}

			// Button remains released
			} else if (press_count > 0) {

				// Double click is discarded after some time and single click is assumed
				if (now - press_t > doublePress_separation) {
					press_count = 0;
					if (!long_triggered) function(BUT_CLICK);
				}
			}
		}
	}

	state_last = s;
}
