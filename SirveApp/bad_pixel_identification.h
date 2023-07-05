#pragma once
#ifndef BAD_PIXEL_IDENTIFICATION_H
#define BAD_PIXEL_IDENTIFICATION_H

#include <cmath>
#include <vector>


namespace BadPixelIdentification
{
    std::vector<short> get_dead_pixel_mask(const std::vector<std::vector<uint16_t>> & input_pixels);
};

#endif