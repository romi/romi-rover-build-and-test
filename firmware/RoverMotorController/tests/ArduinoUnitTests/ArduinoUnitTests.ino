#include "fixed.h"

void ASSERT_EQ(double a, double b)
{
        Serial.print("Computed: ");
        Serial.print(a);
        Serial.print(", expected: ");
        Serial.print(b);
        Serial.println();
}

void ASSERT_NEAR(double a, double b, double e)
{
        Serial.print("Computed: ");
        Serial.print(a);
        Serial.print(", expected: ");
        Serial.print(b);
        Serial.print("+-");
        Serial.print(e);
        Serial.println();
}

void test_fxadd()
{
        // Arrange
        fixed_t a = dtofx(0.0);
        fixed_t b = dtofx(1.0);
        fixed_t c = dtofx(-1.0);
        fixed_t d = dtofx(2.0);
        fixed_t e = dtofx(0.0001);
        fixed_t f = dtofx(12345.0);
        
        // Act
        fixed_t r1 = fxadd(a, b);
        fixed_t r2 = fxadd(b, c);
        fixed_t r3 = fxadd(d, c);
        fixed_t r4 = fxadd(a, e);
        fixed_t r5 = fxadd(e, f);

        // Assert
        ASSERT_EQ(fxtod(r1), 1.0);
        ASSERT_EQ(fxtod(r2), 0.0);
        ASSERT_EQ(fxtod(r3), 1.0);
        ASSERT_NEAR(fxtod(r4), 0.0001, 0.00001);
        ASSERT_NEAR(fxtod(r5), 12345.0001, 0.00001);
}

void test_fxmul()
{
        // Arrange
        fixed_t a = dtofx(0.0);
        fixed_t b = dtofx(1.0);
        fixed_t c = dtofx(-1.0);
        fixed_t d = dtofx(2.0);
        fixed_t e = dtofx(0.01);
        fixed_t f = dtofx(12345.0);
        
        // Act
        fixed_t r1 = fxmul(a, b);
        fixed_t r2 = fxmul(b, c);
        fixed_t r3 = fxmul(d, c);
        fixed_t r4 = fxmul(b, e);
        fixed_t r5 = fxmul(e, f);

        // Assert
        ASSERT_EQ(fxtod(r1), 0.0);
        ASSERT_EQ(fxtod(r2), -1.0);
        ASSERT_EQ(fxtod(r3), -2.0);
        ASSERT_NEAR(fxtod(r4), 0.01, 0.003);
        ASSERT_NEAR(fxtod(r5), 123.45, 0.1);
}

void setup()
{
        Serial.begin(115200);
        while (!Serial)
                ;

        test_fxadd();
        test_fxmul();
}

void loop()
{
}
