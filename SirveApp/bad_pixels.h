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

    arma::uvec identify_bad_pixels_median(double N, std::vector<std::vector<uint16_t>> & input_pixels, QProgressDialog & progress);

    arma::uvec identify_bad_pixels_moving_median(int half_window_length, double N, std::vector<std::vector<uint16_t>> & input_pixels, QProgressDialog & progress);

    arma::uvec find_dead_badscale_pixels(std::vector<std::vector<uint16_t>>& input_pixels);

    void replace_pixels_with_neighbors(std::vector<std::vector<uint16_t>> & original_pixels, std::vector<unsigned int> bad_pixel_indeces, int width_pixels, QProgressDialog & progress);

};

#endif