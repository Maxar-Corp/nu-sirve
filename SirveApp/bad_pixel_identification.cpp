#include "bad_pixel_identification.h"

std::vector<short> BadPixelIdentification::get_dead_pixel_mask(const std::vector<std::vector<uint16_t>>& input_pixels)
{    
    std::vector<short> dead_pixel_mask;

    size_t num_frames = input_pixels.size();
    // Loop through each pixel
    for (int i = 0; i < input_pixels[0].size(); i++) 
    {
        double mean = 0.0;
        double squared_diff_sum = 0.0;

        for (int j = 0; j < num_frames; j++) {
            mean += input_pixels[j][i];
        }
        mean /= num_frames;

        for (int j = 0; j < num_frames; j++) {
            double diff = input_pixels[j][i] - mean;
            squared_diff_sum += diff * diff;
        }

        double variance = squared_diff_sum / num_frames;
        double std_deviation = std::sqrt(variance);

        dead_pixel_mask.push_back(std_deviation == 0);
    }

    return dead_pixel_mask;
}