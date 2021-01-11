//
// Created by douglas on 22/09/2020.
//

#ifndef ROMIPATH_IMAGE_UTILS_H
#define ROMIPATH_IMAGE_UTILS_H
#include <vector>
#include <cstdint>
#include <tuple>
#include <map>
#include <set>
#include <random>

std::tuple<uint8_t, uint8_t, uint8_t>
GetPixel(const uint8_t * image, int x, int y, int numchannels, int width);

void
SetPixel(uint8_t * image, int x, int y, int numchannels, int width, uint8_t r, uint8_t g, uint8_t b);

std::vector<float>
convert_RGB_non_interlaced(const uint8_t *rgb_image, const int width, const int height, const int numchannels);

std::vector<uint32_t>
convert_RGB_32Bit_0RGB(const uint8_t *rgb_image, const int width, const int height, const int numchannels);

std::vector<uint8_t>
convert_32Bit_0RGB_RGB(const std::vector<uint32_t>& image);

std::map<int, std::tuple<uint8_t, uint8_t, uint8_t>>
GenerateColourTable(const std::vector<uint32_t>& segmentation);

void
colourise_segments(const int numchannels, const int width, const int height, std::vector<uint8_t> &vector_byte_image,
                   const std::vector<uint32_t> &unsigned_segmentation_labels);

void DrawContoursAroundSegments_vlSlic(
        uint8_t *&			        ubuff,
        int*&					labels,
        const int&				width,
        const int&				height,
        const unsigned int&				color );

void draw_circle(uint8_t * image, uint32_t width, uint32_t height, double radius, uint32_t npoints);

void draw_line(uint8_t * image, uint32_t width, int x0, int y0, int x1, int y1);


#endif //ROMIPATH_IMAGE_UTILS_H
