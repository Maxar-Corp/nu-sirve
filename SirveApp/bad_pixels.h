#pragma once
#ifndef BAD_PIXELS_H
#define BAD_PIXELS_H

#include <cmath>
#include <vector>
#include <set>
#include <armadillo>
#include <QProgressDialog>

namespace BadPixels
{
    std::vector<unsigned int> identify_dead_pixels(const std::vector<std::vector<uint16_t>> & input_pixels);
    std::vector<unsigned int> identify_dead_pixels_new(double N, std::vector<std::vector<uint16_t>> & input_pixels, bool only_dead, QProgressDialog & progress);
    void replace_pixels_with_neighbors(std::vector<std::vector<uint16_t>> & original_pixels, std::vector<unsigned int> bad_pixel_indeces, int width_pixels, QProgressDialog & progress);
};

#endif