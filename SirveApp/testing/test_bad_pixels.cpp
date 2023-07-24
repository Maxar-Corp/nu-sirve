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

void TestBadPixels::test_replace_bad_pixels_center()
{
    //Test that bad pixel replacement uses all available neighbor pixels (up to two in each direction)
    std::vector<std::vector<uint16_t>> input_pixels = {
        {
            0, 0, 4, 0, 0,
            0, 0, 2, 0, 0,
            5, 3, 1, 5, 7,
            0, 0, 6, 0, 0,
            0, 0, 8, 0, 0
        },
        {
            0, 0, 4, 0, 0,
            0, 0, 5, 0, 0,
            4, 5, 1, 7, 8,
            0, 0, 6, 0, 0,
            0, 0, 7, 0, 0
        }
    };

    int width_pixels = 5;
    unsigned int bad_pixel_index = 12;

    //Ensure the bad pixels are unmodified
    QCOMPARE(input_pixels[0][bad_pixel_index], 1);
    QCOMPARE(input_pixels[1][bad_pixel_index], 1);

    BadPixels::replace_pixels_with_neighbors(input_pixels, { bad_pixel_index }, width_pixels);

    //Ensure the bad pixels have been replaced
    QCOMPARE(input_pixels[0][bad_pixel_index], 5);
    QCOMPARE(input_pixels[1][bad_pixel_index], 6);
}

void TestBadPixels::test_replace_bad_pixels_edges()
{
    //Test that bad pixel replacement is correct when it hits the edges or corners of the grid
    std::vector<std::vector<uint16_t>> input_pixels = {
        {
            1, 2, 4, 0, 0,
            2, 0, 0, 0, 7,
            4, 0, 0, 0, 2,
            0, 0, 7, 2, 1,
            0, 0, 0, 0, 2
        },
        {
            1, 5, 7, 0, 0,
            5, 0, 0, 0, 3,
            7, 0, 0, 0, 3,
            0, 0, 3, 3, 1,
            0, 0, 0, 0, 3
        }
    };

    int width_pixels = 5;
    unsigned int first_bad_pixel_index = 0;
    unsigned int second_bad_pixel_index = 19;

    //Ensure the bad pixels are unmodified
    QCOMPARE(input_pixels[0][first_bad_pixel_index], 1);
    QCOMPARE(input_pixels[1][first_bad_pixel_index], 1);
    QCOMPARE(input_pixels[0][second_bad_pixel_index], 1);
    QCOMPARE(input_pixels[1][second_bad_pixel_index], 1);

    BadPixels::replace_pixels_with_neighbors(input_pixels, { first_bad_pixel_index, second_bad_pixel_index }, width_pixels);

    //Ensure the bad pixels have been replaced
    QCOMPARE(input_pixels[0][first_bad_pixel_index], 3);
    QCOMPARE(input_pixels[1][first_bad_pixel_index], 6);
    QCOMPARE(input_pixels[0][second_bad_pixel_index], 4);
    QCOMPARE(input_pixels[1][second_bad_pixel_index], 3);
}

void TestBadPixels::test_replace_bad_pixels_overlapping()
{
    //Test that bad pixel replacement ignores nearby bad pixels
    std::vector<std::vector<uint16_t>> input_pixels = {
        {
            0, 1, 8, 0, 0,
            0, 1, 8, 0, 0,
            1, 99, 99, 1, 8,
            0, 1, 8, 0, 0,
            0, 1, 8, 0, 0
        },
        {
            0, 8, 1, 0, 0,
            0, 8, 1, 0, 0,
            8, 99, 99, 8, 1,
            0, 8, 1, 0, 0,
            0, 8, 1, 0, 0
        }
    };

    int width_pixels = 5;
    unsigned int first_bad_pixel_index = 11;
    unsigned int second_bad_pixel_index = 12;

    //Ensure the bad pixels are unmodified
    QCOMPARE(input_pixels[0][first_bad_pixel_index], 99);
    QCOMPARE(input_pixels[1][first_bad_pixel_index], 99);
    QCOMPARE(input_pixels[0][second_bad_pixel_index], 99);
    QCOMPARE(input_pixels[1][second_bad_pixel_index], 99);

    BadPixels::replace_pixels_with_neighbors(input_pixels, { first_bad_pixel_index, second_bad_pixel_index }, width_pixels);

    //Ensure the bad pixels have been replaced
    QCOMPARE(input_pixels[0][first_bad_pixel_index], 1);
    QCOMPARE(input_pixels[1][first_bad_pixel_index], 8);
    QCOMPARE(input_pixels[0][second_bad_pixel_index], 6);
    QCOMPARE(input_pixels[1][second_bad_pixel_index], 3);
}