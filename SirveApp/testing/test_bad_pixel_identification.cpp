#include "test_bad_pixel_identification.h"

TestBadPixelIdentification::TestBadPixelIdentification()
{}

std::vector<std::vector<uint16_t>> TestBadPixelIdentification::generate_test_input_with_dead_pixels()
{
    std::vector<std::vector<uint16_t>> input_pixels;

    // Generate 5 "frames" of test input
    for (int i = 0; i < 5; i++)
    {
        std::vector<uint16_t> frame;

        //Each "frame" has 10 pixels
        for (int j = 0; j < 10; j++) {
            if (j % 3 == 0) {
                // Set every third pixel to the same value across all the frames
                // This will simulate pixels that are dead across all the frames
                frame.push_back(j);
            } else {
                // Set other values "randomly" such that the stddev won't be 0
                frame.push_back(1.0 + i + j * 0.1);
            }
        }
        input_pixels.push_back(frame);
    }

    return input_pixels;
}

void TestBadPixelIdentification::test_get_dead_pixel_mask() {
    std::vector<std::vector<uint16_t>> input_pixels = generate_test_input_with_dead_pixels();

    std::vector<short> dead_pixel_mask = BadPixelIdentification::get_dead_pixel_mask(input_pixels);

    for (int i = 0; i < dead_pixel_mask.size(); i++)
    {    
        // Verify that dead pixels are correctly identified
        if (i % 3 == 0) {
            QCOMPARE(dead_pixel_mask[i], 1);
        }
        // Verify that undead (zombie? teehee) pixels are correctly identified
        else {
            QCOMPARE(dead_pixel_mask[i], 0);
        }
    }
}