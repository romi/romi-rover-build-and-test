#ifndef ROMIPATH_CENTRES_H
#define ROMIPATH_CENTRES_H
#include <cstdint>
#include <iostream>
#include <set>
#include <vector>

std::vector<std::pair<uint32_t, uint32_t>>
calculate_centers(const std::vector<uint32_t>& segmentation,
                  uint64_t width, uint64_t height);

std::vector<std::pair<uint32_t, uint32_t>>
calculate_centres(const std::vector<double> &kseedsl,
                  const std::vector<double> &kseedsa,
                  const std::vector<double> &kseedsb,
                  const std::vector<double> &kseedsx,
                  const std::vector<double> &kseedsy);

void
plot_centres(std::vector<uint8_t> &vector_byte_image,
             const std::vector<std::pair<uint32_t, uint32_t>> &centres,
             const int width, const int numchannels);

#endif //ROMIPATH_CENTRES_H
