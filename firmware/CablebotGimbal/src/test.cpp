#include "Arduino.h"

#include <math.h>
#include "fixed.h"
#include "test.h"

// Utility functions -----------------

#define ASSERT_EQUAL() { assert_equal(__FUNCTION__, expected, result); }

void assert_equal(const char *name, int32_t expected, int32_t result)
{
        if (result != expected) {
		Serial.print("Failed: ");
		Serial.print(name);
		Serial.print(": result ");
		Serial.print(result);
		Serial.print(", expected ");
		Serial.println(expected);
        } else {
 		Serial.print("OK: ");
		Serial.println(name);
        }
}

void assert_equal(const char *name, float expected, float result)
{
        float delta = 0.0001f;
        if (expected != 0.0) {
                delta = fabsf(expected) / 100.0f;
        }
        if (fabsf(result - expected) > delta) {
		Serial.print("Failed: ");
		Serial.print(name);
		Serial.print(": result ");
		Serial.print(result, 6);
		Serial.print(", expected ");
		Serial.println(expected, 6);
        } else {
 		Serial.print("OK: ");
		Serial.println(name);
        }
}

// Fixed -----------------

void run_test_to_fixed_1()
{
        int32_t expected = 2048;
        fixed_t result = to_fixed(0.5);
        ASSERT_EQUAL();
}

void run_test_to_fixed_2()
{
        int32_t expected = -2 * 4096;
        fixed_t result = to_fixed(-2);
        ASSERT_EQUAL();
}

void run_test_to_fixed_3()
{
        int32_t expected = -2 * 4096 - 2048;
        fixed_t result = to_fixed(-2.5f);
        ASSERT_EQUAL();
}

void run_test_frac_1()
{
        float expected = 0.5f;
        fixed_t a = to_fixed(expected);
        float result = fixed_frac(a);
        ASSERT_EQUAL();
}

void run_test_float_1()
{
        float expected = 0.5f;
        fixed_t a = to_fixed(expected);
        float result = fixed_float(a);
        ASSERT_EQUAL();
}

void run_test_add_1()
{
        float expected = 0.7f;
        fixed_t a = to_fixed(0.5);
        fixed_t b = to_fixed(0.2);
        fixed_t c = fixed_add(a, b);
        float result = fixed_float(c);
        ASSERT_EQUAL();
}

void run_test_add_2()
{
        float expected = 0.0f;
        fixed_t a = to_fixed(0.5);
        fixed_t b = to_fixed(-0.5);
        fixed_t c = fixed_add(a, b);
        float result = fixed_float(c);
        ASSERT_EQUAL();
}

void run_test_sub_1()
{
        float expected = 0.3f;
        fixed_t a = to_fixed(0.5);
        fixed_t b = to_fixed(0.2);
        fixed_t c = fixed_sub(a, b);
        float result = fixed_float(c);
        ASSERT_EQUAL();
}

void run_test_mul_1()
{
        float expected = 0.1f;
        fixed_t a = to_fixed(0.5f);
        fixed_t b = to_fixed(0.2f);
        fixed_t c = fixed_mul(a, b);
        float result = fixed_float(c);
        // Serial.println(a);
        // Serial.println(b);
        // Serial.println(c);
        // Serial.println(result);
        ASSERT_EQUAL();
}

void run_test_mul_2()
{
        float expected = -1.0f;
        fixed_t a = to_fixed(0.5f);
        fixed_t b = to_fixed(-2.0f);
        fixed_t c = fixed_mul(a, b);
        float result = fixed_float(c);
        ASSERT_EQUAL();
}

void run_tests_fixed()
{
        run_test_to_fixed_1();
        run_test_to_fixed_2();
        run_test_to_fixed_3();
        run_test_frac_1();
        run_test_float_1();
        run_test_add_1();
        run_test_add_2();
        run_test_sub_1();
        run_test_mul_1();
        run_test_mul_2();
}

void run_tests_all()
{
        run_tests_fixed();
}
