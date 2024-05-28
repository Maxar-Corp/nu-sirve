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
    std::vector<unsigned int> IdentifyDeadPixels(const std::vector<std::vector<uint16_t>> & input_pixels);
    std::vector<unsigned int> IdentifyDeadPixelsMedian(double N, std::vector<std::vector<uint16_t>> & input_pixels, bool only_dead, QProgressDialog & progress);
    std::vector<unsigned int> IdentifyDeadPixelsMovingMedian(int half_window_length, double N, std::vector<std::vector<uint16_t>> & input_pixels, QProgressDialog & progress);
    void ReplacePixelsWithNeighbors(std::vector<std::vector<uint16_t>> & original_pixels, std::vector<unsigned int> bad_pixel_indeces, int width_pixels, QProgressDialog & progress);
};

#endif
