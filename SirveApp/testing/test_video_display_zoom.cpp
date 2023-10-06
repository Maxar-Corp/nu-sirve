#include "test_video_display_zoom.h"

TestVideoDisplayZoom::TestVideoDisplayZoom()
{}

void TestVideoDisplayZoom::test_is_currently_zoomed()
{
    VideoDisplayZoomManager v(640, 480);
    QCOMPARE(v.is_currently_zoomed(), false);

    v.zoom_image(QRect(1, 1, 10, 10));
    QCOMPARE(v.is_currently_zoomed(), true);

    v.zoom_image(QRect(1, 1, 10, 10));
    QCOMPARE(v.is_currently_zoomed(), true);
    
    v.unzoom();
    QCOMPARE(v.is_currently_zoomed(), true);

    v.zoom_image(QRect(1, 1, 10, 10));
    QCOMPARE(v.is_currently_zoomed(), true);

    v.unzoom();
    QCOMPARE(v.is_currently_zoomed(), true);

    v.unzoom();
    QCOMPARE(v.is_currently_zoomed(), false);
}

void TestVideoDisplayZoom::test_zoom_and_unzoom()
{
    VideoDisplayZoomManager v(640, 480);
    v.zoom_image(QRect(320, 240, 320, 240)); //the bottom right quadrant
    v.zoom_image(QRect(320, 240, 320, 240)); //the bottom right quadrant of ^

    //Ensure the last zoom is the final 1/4 of the image in both directions
    QCOMPARE(v.absolute_zoom_list.size(), 3);
    QCOMPARE(v.absolute_zoom_list[2].x, 480);
    QCOMPARE(v.absolute_zoom_list[2].width, 160);
    QCOMPARE(v.absolute_zoom_list[2].y, 360);
    QCOMPARE(v.absolute_zoom_list[2].height, 120);

    v.unzoom();

    //Ensure the last zoom is 1/2 of the image in both directions starting at the midpoint
    QCOMPARE(v.absolute_zoom_list.size(), 2);
    QCOMPARE(v.absolute_zoom_list[1].x, 320);
    QCOMPARE(v.absolute_zoom_list[1].width, 320);
    QCOMPARE(v.absolute_zoom_list[1].y, 240);
    QCOMPARE(v.absolute_zoom_list[1].height, 240);

    v.unzoom();
    //Ensure the "zoom" level is the full image
    QCOMPARE(v.absolute_zoom_list.size(), 1);
    QCOMPARE(v.absolute_zoom_list[0].x, 0);
    QCOMPARE(v.absolute_zoom_list[0].width, 640);
    QCOMPARE(v.absolute_zoom_list[0].y, 0);
    QCOMPARE(v.absolute_zoom_list[0].height, 480);
}

void TestVideoDisplayZoom::test_get_position_within_zoom()
{
    VideoDisplayZoomManager v(640, 480);

    //Ensure pixels are at the right position in an unzoomed image
    std::vector<int> result = v.get_position_within_zoom(480, 360);
    QCOMPARE(result[0], 480);
    QCOMPARE(result[1], 360);

    v.zoom_image(QRect(320, 240, 320, 240)); //the bottom right quadrant of the window
    
    //Ensure a pixel in the middle of the zoom window is correctly found
    result = v.get_position_within_zoom(480, 360); //the center of the bottom right quadrant
    QCOMPARE(result[0], 320);
    QCOMPARE(result[1], 240);

    //Ensuring the edges/corners work
    result = v.get_position_within_zoom(320, 240);
    QCOMPARE(result[0], 0);
    QCOMPARE(result[1], 0);
    result = v.get_position_within_zoom(640, 480);
    QCOMPARE(result[0], 640);
    QCOMPARE(result[1], 480);
    
    //Ensure points outside the zoom window aren't returned
    result = v.get_position_within_zoom(160, 120); //a point in the top left quadrant
    QCOMPARE(result[0], -1);
    QCOMPARE(result[1], -1);

    
    v.zoom_image(QRect(320, 240, 320, 240)); //the bottom right quadrant of the remaining window

    //Ensure the same 4 tests still pass (middle, corner, corner, outside) with additional zoom
    result = v.get_position_within_zoom(560, 420);
    QCOMPARE(result[0], 320);
    QCOMPARE(result[1], 240);
    result = v.get_position_within_zoom(480, 360);
    QCOMPARE(result[0], 0);
    QCOMPARE(result[1], 0);
    result = v.get_position_within_zoom(640, 480);
    QCOMPARE(result[0], 640);
    QCOMPARE(result[1], 480);
    result = v.get_position_within_zoom(320, 240);
    QCOMPARE(result[0], -1);
    QCOMPARE(result[1], -1);
}

void TestVideoDisplayZoom::test_is_any_piece_within_zoom()
{
    VideoDisplayZoomManager v(640, 480);

    //hyper zoom into a randomly chosen weird box in the top left
    v.zoom_image(QRect(10, 10, 21, 17));
    //In order to loosely maintain the aspect ratio of the original image while zooming, the zooming gets ...
    // ... manipulated by the zooming logic to guard against, e.g., stretching the viewport by selecting a very vertical rectangle
    //So for 21/17, the width/height get reset to be 23/17

    //Now the zoom window is from:
    // 10 to 33 in the X direction with each "original" pixel now being 640/23 = 27.826087 true pixels wide
    // 10 to 27 in the Y direction with each "original" pixel now being 480/17 = 28.235294 true pixels high
    
    v.zoom_image(QRect(160, 120, 320, 240));
    //Now, we zoom into the center (half the width, half the height, centered) of that viewport
    //In theory, the new view window stretches from "original" pixel:
    // X direction (~15.75, ~27.25): Math explanation: 10 + 1/4 * 23 , 10 + 3/4 * 23
    // Y direction: (~14.25, ~22.75): Math explanation: 10 + 1/4 * 17 , 10 + 3/4 * 17

    //Reminder that the a pixel of N,M stretches at its bottom right corner to hit N+1,M+1, therefore:
    //On the left side, X direction - no part of column/pixel 14 is in the viewport but part of 15 is
    //On the right side, X direction - part of column/pixel 27 is in the viewport but no part of 28
    //On the top side, Y direction - no part of row/pixel 13 is in the viewport but part of 14 is
    //On the bottom side, Y direction - part of row/pixel 22 is in the viewport but no part of 23
    QCOMPARE(v.is_any_piece_within_zoom(14, 13), false);
    QCOMPARE(v.is_any_piece_within_zoom(14, 14), false);
    QCOMPARE(v.is_any_piece_within_zoom(14, 22), false);
    QCOMPARE(v.is_any_piece_within_zoom(14, 23), false);
    
    QCOMPARE(v.is_any_piece_within_zoom(15, 13), false);
    QCOMPARE(v.is_any_piece_within_zoom(15, 14), true);
    QCOMPARE(v.is_any_piece_within_zoom(15, 22), true);
    QCOMPARE(v.is_any_piece_within_zoom(15, 23), false);

    QCOMPARE(v.is_any_piece_within_zoom(27, 13), false);
    QCOMPARE(v.is_any_piece_within_zoom(27, 14), true);
    QCOMPARE(v.is_any_piece_within_zoom(27, 22), true);
    QCOMPARE(v.is_any_piece_within_zoom(27, 23), false);

    QCOMPARE(v.is_any_piece_within_zoom(28, 13), false);
    QCOMPARE(v.is_any_piece_within_zoom(28, 14), false);
    QCOMPARE(v.is_any_piece_within_zoom(28, 22), false);
    QCOMPARE(v.is_any_piece_within_zoom(28, 23), false);
}