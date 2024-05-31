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
    arma::uvec IdentifyBadPixelsMedian(double N, std::vector<std::vector<uint16_t>> & input_pixels, QProgressDialog & progress);

    arma::uvec IdentifyBadPixelsMovingMedian(int half_window_length, double N, std::vector<std::vector<uint16_t>> & input_pixels, QProgressDialog & progress);

    arma::uvec FindDeadBadscalePixels(std::vector<std::vector<uint16_t>>& input_pixels);

    void ReplacePixelsWithNeighbors(std::vector<std::vector<uint16_t>> & original_pixels, std::vector<unsigned int> bad_pixel_indeces, int width_pixels, QProgressDialog & progress);

};

#endif
