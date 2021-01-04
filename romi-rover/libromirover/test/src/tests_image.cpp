#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Image.h"

using namespace std;
using namespace testing;
using namespace romi;

class image_tests : public ::testing::Test
{
protected:
        
	image_tests() {
        }

	~image_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {}
};

TEST_F(image_tests, test_constructor_1)
{
        Image image;
        
        ASSERT_EQ(image.width(), 0);
        ASSERT_EQ(image.height(), 0);
}

TEST_F(image_tests, test_constructor_2)
{
        Image image(Image::BW, 4, 4);
        
        ASSERT_EQ(image.type(), Image::BW);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 1);
        ASSERT_EQ(image.length(), 16);
        ASSERT_EQ(image.byte_length(), 16 * 4);
}

TEST_F(image_tests, test_constructor_3)
{
        Image image(Image::RGB, 4, 4);
        
        ASSERT_EQ(image.type(), Image::RGB);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 3);
        ASSERT_EQ(image.length(), 3 * 16);
        ASSERT_EQ(image.byte_length(), 3 * 16 * 4);
}

TEST_F(image_tests, test_constructor_4)
{
        uint8_t data[] = { 0,  1,  2,  3,
                           4,  5,  6,  7,
                           8,  9, 10, 11,
                          12, 13, 14, 15 };
        
        Image image(Image::BW, data, 4, 4);
        
        ASSERT_EQ(image.type(), Image::BW);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 1);

        for (size_t y = 0; y < 4; y++) {
                for (size_t x = 0; x < 4; x++) {
                        size_t index = y * 4 + x;
                        ASSERT_EQ(image.get(0, x, y), (float) data[index] / 255.0f);
                }
        }
}

TEST_F(image_tests, test_init)
{
        Image image(Image::RGB, 4, 4);
        image.init(Image::BW, 8, 8);
        
        ASSERT_EQ(image.type(), Image::BW);
        ASSERT_EQ(image.width(), 8);
        ASSERT_EQ(image.height(), 8);
        ASSERT_EQ(image.channels(), 1);
}

TEST_F(image_tests, test_set_get)
{
        uint8_t data[] = { 0,  1,  2,  3,
                           4,  5,  6,  7,
                           8,  9, 10, 11,
                          12, 13, 14, 15 };
        
        Image image(Image::BW, 4, 4);
        for (size_t y = 0; y < 4; y++) {
                for (size_t x = 0; x < 4; x++) {
                        size_t index = y * 4 + x;
                        image.set(0, x, y, data[index]);
                }
        }

        ASSERT_EQ(image.type(), Image::BW);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 1);

        for (size_t y = 0; y < 4; y++) {
                for (size_t x = 0; x < 4; x++) {
                        size_t index = y * 4 + x;
                        ASSERT_EQ(image.get(0, x, y), data[index]);
                }
        }
}

TEST_F(image_tests, test_import)
{
        uint8_t data[] = { 0,  1,  2,  3,
                           4,  5,  6,  7,
                           8,  9, 10, 11,
                          12, 13, 14, 15 };
        
        Image image(Image::RGB, 2, 2);
        image.import(Image::BW, data, 4, 4);

        ASSERT_EQ(image.type(), Image::BW);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 1);

        for (size_t y = 0; y < 4; y++) {
                for (size_t x = 0; x < 4; x++) {
                        size_t index = y * 4 + x;
                        ASSERT_EQ(image.get(0, x, y), data[index] / 255.0f);
                }
        }
}


TEST_F(image_tests, test_fill_bw)
{
        Image image(Image::BW, 4, 4);
        image.fill(0, 1.0f);

        ASSERT_EQ(image.type(), Image::BW);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 1);

        for (size_t y = 0; y < 4; y++) {
                for (size_t x = 0; x < 4; x++) {
                        ASSERT_EQ(image.get(0, x, y), 1.0f);
                }
        }
}

TEST_F(image_tests, test_fill_rgb)
{
        Image image(Image::RGB, 4, 4);
        image.fill(0, 0.0f);
        image.fill(1, 1.0f);
        image.fill(2, 2.0f);

        ASSERT_EQ(image.type(), Image::RGB);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 3);

        for (size_t chan = 0; chan < 3; chan++) {
                for (size_t y = 0; y < 4; y++) {
                        for (size_t x = 0; x < 4; x++) {
                                ASSERT_EQ(image.get(chan, x, y), chan * 1.0f);
                        }
                }
        }
}

TEST_F(image_tests, test_crop_1)
{
        uint8_t data[] = { 0,  1,  2,  3,
                           4,  5,  6,  7,
                           8,  9, 10, 11,
                          12, 13, 14, 15 };
        
        Image image(Image::BW, data, 4, 4);
        Image crop;
        image.crop(1, 1, 2, 2, crop);

        
        ASSERT_EQ(crop.type(), Image::BW);
        ASSERT_EQ(crop.width(), 2);
        ASSERT_EQ(crop.height(), 2);
        ASSERT_EQ(crop.channels(), 1);
        ASSERT_EQ(crop.get(0, 0, 0), 5.0f / 255.0f);
        ASSERT_EQ(crop.get(0, 1, 0), 6.0f / 255.0f);
        ASSERT_EQ(crop.get(0, 0, 1), 9.0f / 255.0f);
        ASSERT_EQ(crop.get(0, 1, 1), 10.0f / 255.0f);
}

TEST_F(image_tests, test_crop_2)
{
        uint8_t data[] = { 0,  1,  2,  3,
                           4,  5,  6,  7,
                           8,  9, 10, 11,
                          12, 13, 14, 15 };
        
        Image image(Image::BW, data, 4, 4);
        Image crop;
        image.crop(3, 3, 2, 2, crop);

        
        ASSERT_EQ(crop.type(), Image::BW);
        ASSERT_EQ(crop.width(), 1);
        ASSERT_EQ(crop.height(), 1);
        ASSERT_EQ(crop.channels(), 1);
        ASSERT_EQ(crop.get(0, 0, 0), 15.0f / 255.0f);
}

TEST_F(image_tests, test_crop_3)
{
        uint8_t data[] = { 0,  1,  2,  3,
                           4,  5,  6,  7,
                           8,  9, 10, 11,
                          12, 13, 14, 15 };
        
        Image image(Image::BW, data, 4, 4);
        Image crop;
        image.crop(4, 4, 2, 2, crop);

        
        ASSERT_EQ(crop.type(), Image::BW);
        ASSERT_EQ(crop.width(), 0);
        ASSERT_EQ(crop.height(), 0);
        ASSERT_EQ(crop.channels(), 1);
}

TEST_F(image_tests, test_scale_1)
{
        uint8_t data[] = { 0,  1,  2,  3,
                           4,  5,  6,  7,
                           8,  9, 10, 11,
                          12, 13, 14, 15 };
        
        Image image(Image::BW, data, 4, 4);
        Image scale;
        image.scale(2, scale);

        
        ASSERT_EQ(scale.type(), Image::BW);
        ASSERT_EQ(scale.width(), 2);
        ASSERT_EQ(scale.height(), 2);
        ASSERT_EQ(scale.channels(), 1);
        ASSERT_EQ(scale.get(0, 0, 0), 0.0f / 255.0f);
        ASSERT_EQ(scale.get(0, 1, 0), 2.0f / 255.0f);
        ASSERT_EQ(scale.get(0, 0, 1), 8.0f / 255.0f);
        ASSERT_EQ(scale.get(0, 1, 1), 10.0f / 255.0f);
}

TEST_F(image_tests, test_scale_2)
{
        Image image(Image::BW, 4, 4);
        Image scale;
        image.scale(0, scale);

        
        ASSERT_EQ(scale.type(), Image::BW);
        ASSERT_EQ(scale.width(), 4);
        ASSERT_EQ(scale.height(), 4);
        ASSERT_EQ(scale.channels(), 1);
}

TEST_F(image_tests, test_copy_to_bw)
{
        uint8_t data[] = { 0,  1,  2,  3,
                           4,  5,  6,  7,
                           8,  9, 10, 11,
                          12, 13, 14, 15 };
        
        Image image(Image::BW, data, 4, 4);
        Image copy;
        image.copy_to(copy);

        
        ASSERT_EQ(copy.type(), Image::BW);
        ASSERT_EQ(copy.width(), 4);
        ASSERT_EQ(copy.height(), 4);
        ASSERT_EQ(copy.channels(), 1);

        for (size_t y = 0; y < 4; y++) {
                for (size_t x = 0; x < 4; x++) {
                        ASSERT_EQ(image.get(0, x, y),
                                  copy.get(0, x, y));
                }
        }
}

TEST_F(image_tests, test_copy_to_rgb)
{
        Image image(Image::RGB, 4, 4);
        image.fill(0, 0.0f);
        image.fill(1, 1.0f);
        image.fill(2, 2.0f);
        Image copy(Image::BW, 2, 2);
        image.copy_to(copy);

        
        ASSERT_EQ(copy.type(), Image::RGB);
        ASSERT_EQ(copy.width(), 4);
        ASSERT_EQ(copy.height(), 4);
        ASSERT_EQ(copy.channels(), 3);

        for (size_t chan = 0; chan < 3; chan++) {
                for (size_t y = 0; y < 4; y++) {
                        for (size_t x = 0; x < 4; x++) {
                                ASSERT_EQ(image.get(chan, x, y),
                                          copy.get(chan, x, y));
                        }
                }
        }
}

