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

void BadPixels::replace_pixels_with_neighbors(std::vector<std::vector<uint16_t>> & original_pixels, std::vector<unsigned int> bad_pixel_indeces, int width_pixels)
{
    //For each frame, replaces any bad pixels with the mean of the value of 2 pixels above, below, left, and right
    //Other bad pixels are exempted from the calculation

    //Creating a set (for faster lookup) of bad pixel indeces simplifies/speeds up some of this code
    std::set<int> bad_pixel_indeces_set(bad_pixel_indeces.begin(), bad_pixel_indeces.end());

    for (auto frame = 0; frame < original_pixels.size(); frame++)
    {
        for (auto i = 0; i < bad_pixel_indeces.size(); i++)
        {
            int bad_pixel_index = bad_pixel_indeces[i];
            std::vector<int> neighbor_pixels;

            //Grab up to two pixels from the left, if possible
            int x_loc = bad_pixel_index % width_pixels;
            if (x_loc > 0)
            {
                neighbor_pixels.push_back(bad_pixel_index - 1);
                if (x_loc > 1)
                {
                    neighbor_pixels.push_back(bad_pixel_index - 2);
                }
            }

            //Grab up to two pixels from the right, if possible
            if (x_loc < width_pixels - 1)
            {
                neighbor_pixels.push_back(bad_pixel_index + 1);
                if (x_loc < width_pixels - 2)
                {
                    neighbor_pixels.push_back(bad_pixel_index + 2);
                }
            }

            //Grab up to two pixels from above, if possible
            if (bad_pixel_index - width_pixels >= 0)
            {
                neighbor_pixels.push_back(bad_pixel_index - width_pixels);
                if (bad_pixel_index - 2*width_pixels >= 0)
                {
                    neighbor_pixels.push_back(bad_pixel_index - 2*width_pixels);
                }
            }

            //Grab up to two pixels from below, if possible
            if (bad_pixel_index + width_pixels < original_pixels[frame].size())
            {
                neighbor_pixels.push_back(bad_pixel_index + width_pixels);
                if (bad_pixel_index + 2*width_pixels < original_pixels[frame].size())
                {
                    neighbor_pixels.push_back(bad_pixel_index + 2*width_pixels);
                }
            }

            //Determine the neighboring pixels' mean value to replace the pixel with
            int running_sum = 0;
            int other_bad_pixels_to_skip = 0;

            for (auto j = 0; j < neighbor_pixels.size(); j++)
            {
                if (bad_pixel_indeces_set.count(neighbor_pixels[j]))
                {
                    other_bad_pixels_to_skip += 1;
                }
                else
                {
                    running_sum += original_pixels[frame][neighbor_pixels[j]];
                }
            }

            //The mean neighboring value is {sum of neighbor values}/{count of neighbors}
            //We have to remember that we skipped any neighbors that are also bad pixels
            double mean_value = 1.0 * running_sum / (neighbor_pixels.size() - other_bad_pixels_to_skip);
            int mean = static_cast<int>(std::round(mean_value));

            //Replace the bad pixel
            original_pixels[frame][bad_pixel_index] = mean;
        }
    }
}