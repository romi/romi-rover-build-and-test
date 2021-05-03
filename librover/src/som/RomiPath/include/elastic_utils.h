//
// Created by douglas on 22/09/2020.
//

#ifndef ROMIPATH_ELASTIC_UTILS_H
#define ROMIPATH_ELASTIC_UTILS_H
#include <vector>

const double ka = 0.99;
const double alpha = 0.2;
const double beta = 0.2;

typedef double real_t;

typedef struct _point_t {
    real_t x;
    real_t y;
} point_t;


#define NE 4096
#define KE 8.0

std::vector<point_t>
calc_elastic(std::vector<std::pair<uint32_t, uint32_t>>& centresYX, uint32_t width, uint32_t height);

#endif //ROMIPATH_ELASTIC_UTILS_H
