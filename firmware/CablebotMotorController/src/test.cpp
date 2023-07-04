#include <Arduino.h>
#include <FastLED.h>
#include "test.h"

void run_tests_all()
{
        run_tests_leds();
        run_tests_motor();
}

// extern CRGB leds[3];

// void run_tests_leds_set(const struct CRGB &color)
// {
//         leds[0] = color;
//         leds[1] = color;
//         leds[2] = color;        
//         FastLED.show();
// }

// void run_tests_leds_clear()
// {
//         run_tests_leds_set(CRGB::Black);
// }

// void run_tests_leds_1()
// {
//         run_tests_leds_set(CRGB::Red);
//         delay(2000);
//         run_tests_leds_set(CRGB::Green);
//         delay(2000);
//         run_tests_leds_set(CRGB::Blue);
//         delay(2000);
//         run_tests_leds_clear();
// }

void run_tests_leds()
{
        // run_tests_leds_1();
}

void run_tests_motor()
{
}
