#include "test_bad_pixels.h"

TestBadPixels::TestBadPixels()
{}

std::vector<std::vector<uint16_t>> TestBadPixels::generate_test_input_with_dead_pixels()
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

void TestBadPixels::test_identify_dead_pixels() {
    std::vector<std::vector<uint16_t>> input_pixels = generate_test_input_with_dead_pixels();

    std::vector<unsigned int> dead_pixels = BadPixels::identify_dead_pixels(input_pixels);

    //Ensure that exactly 4 pixels were identified as "dead", e.g. no pixels were falsely labeled
    QCOMPARE(dead_pixels.size(), 4);

    //Ensure the dead pixels were correctly identified
    QCOMPARE(dead_pixels[0], 0);
    QCOMPARE(dead_pixels[1], 3);
    QCOMPARE(dead_pixels[2], 6);
    QCOMPARE(dead_pixels[3], 9);
}