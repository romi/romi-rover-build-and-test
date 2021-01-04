#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ImageIO.h"

using namespace std;
using namespace testing;
using namespace romi;

class imageio_tests : public ::testing::Test
{
protected:

        static constexpr const char *jpg_file = "/tmp/imageio_tests.jpg";
        static constexpr const char *png_file = "/tmp/imageio_tests.png";
        
        Image bw;
        Image rgb;
        
	imageio_tests() {
                uint8_t grey[] = { 0,  32,  64,  96,
                                   32, 64,  96,  128,
                                   64, 96,  128, 160,
                                   96, 128, 160, 192 };
        
                bw.import(Image::BW, grey, 4, 4);
                
                uint8_t red[] = { 0,0,0,  32,0,0,  64,0,0,  96,0,0,
                                  32,0,0, 64,0,0,  96,0,0,  128,0,0,
                                  64,0,0, 96,0,0,  128,0,0, 160,0,0,
                                  96,0,0, 128,0,0, 160,0,0, 192,0,0 };
        
                rgb.import(Image::RGB, red, 4, 4);
        }

	~imageio_tests() override = default;

	void SetUp() override {
        }

	void TearDown() override {}
};

TEST_F(imageio_tests, store_jpg_returns_error_on_invalid_file)
{
        bool success = ImageIO::store_jpg(bw, "/foo/bar");
        ASSERT_EQ(success, false);
}

TEST_F(imageio_tests, successful_store_jpg)
{
        bool success = ImageIO::store_jpg(bw, jpg_file);
        ASSERT_EQ(success, true);
}

TEST_F(imageio_tests, successful_store_and_load_jpg_1)
{
        bool success = ImageIO::store_jpg(bw, jpg_file);
        ASSERT_EQ(success, true);

        Image image;
        success = ImageIO::load(image, jpg_file);

        ASSERT_EQ(image.type(), Image::BW);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 1);

        float *p0 = bw.data();
        float *p1 = image.data();
        for (size_t i = 0; i < image.length(); i++) {
                ASSERT_NEAR(p0[i], p1[i], 0.004);
        }
}

TEST_F(imageio_tests, successful_store_and_load_jpg_2)
{
        bool success = ImageIO::store_jpg(rgb, jpg_file);
        ASSERT_EQ(success, true);

        Image image;
        success = ImageIO::load(image, jpg_file);

        ASSERT_EQ(image.type(), Image::RGB);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 3);

        // float *p0 = rgb.data();
        // float *p1 = image.data();
        // for (size_t i = 0; i < image.length(); i++) {
        //         ASSERT_NEAR(p0[i], p1[i], 0.1);
        // }
}

TEST_F(imageio_tests, store_png_returns_error_on_invalid_file)
{
        bool success = ImageIO::store_png(bw, "/foo/bar");
        ASSERT_EQ(success, false);
}

TEST_F(imageio_tests, successful_store_png)
{
        bool success = ImageIO::store_png(bw, png_file);
        ASSERT_EQ(success, true);
}

TEST_F(imageio_tests, successful_store_and_load_png_1)
{
        bool success = ImageIO::store_png(bw, png_file);
        ASSERT_EQ(success, true);

        Image image;
        success = ImageIO::load(image, png_file);

        ASSERT_EQ(image.type(), Image::BW);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 1);

        float *p0 = bw.data();
        float *p1 = image.data();
        for (size_t i = 0; i < image.length(); i++) {
                ASSERT_NEAR(p0[i], p1[i], 0.004);
        }
}

TEST_F(imageio_tests, successful_store_and_load_png_2)
{
        bool success = ImageIO::store_png(rgb, png_file);
        ASSERT_EQ(success, true);

        Image image;
        success = ImageIO::load(image, png_file);

        ASSERT_EQ(image.type(), Image::RGB);
        ASSERT_EQ(image.width(), 4);
        ASSERT_EQ(image.height(), 4);
        ASSERT_EQ(image.channels(), 3);

        float *p0 = rgb.data();
        float *p1 = image.data();
        for (size_t i = 0; i < image.length(); i++) {
                ASSERT_NEAR(p0[i], p1[i], 0.004);
        }
}
