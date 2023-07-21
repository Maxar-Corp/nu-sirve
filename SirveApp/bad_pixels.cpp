#include "bad_pixels.h"

std::vector<unsigned int> BadPixels::identify_dead_pixels(const std::vector<std::vector<uint16_t>>& input_pixels)
{    
    std::vector<unsigned int> dead_pixels;

    size_t num_frames = input_pixels.size();
    // Loop through each pixel
    for (auto i = 0; i < input_pixels[0].size(); i++)
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

        if (std_deviation == 0)
        {
            dead_pixels.push_back(i);
        }
    }

    return dead_pixels;
}

void BadPixels::replace_pixels_with_neighbors(std::vector<std::vector<uint16_t>> & original_pixels, std::vector<unsigned int> bad_pixel_indeces)
{
    //intentionally do nothing rn
}