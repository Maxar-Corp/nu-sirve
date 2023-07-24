#pragma once
#ifndef BAD_PIXELS_H
#define BAD_PIXELS_H

#include <cmath>
#include <vector>
#include <set>

namespace BadPixels
{
    std::vector<unsigned int> identify_dead_pixels(const std::vector<std::vector<uint16_t>> & input_pixels);
    void replace_pixels_with_neighbors(std::vector<std::vector<uint16_t>> & original_pixels, std::vector<unsigned int> bad_pixel_indeces, int width_pixels);
};

#endif