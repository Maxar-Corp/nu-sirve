#include "test_video_display_zoom.h"
#include "constants.h"

TestVideoDisplayZoom::TestVideoDisplayZoom()
{}

void TestVideoDisplayZoom::test_is_currently_zoomed()
{
    VideoDisplayZoomManager v(SirveAppConstants::VideoDisplayWidth, SirveAppConstants::VideoDisplayHeight);
    QCOMPARE(v.is_currently_zoomed(), false);

    v.ZoomImage(QRect(1, 1, 10, 10));
    QCOMPARE(v.is_currently_zoomed(), true);

    v.ZoomImage(QRect(1, 1, 10, 10));
    QCOMPARE(v.is_currently_zoomed(), true);
    
    v.UndoZoom();
    QCOMPARE(v.is_currently_zoomed(), true);

    v.ZoomImage(QRect(1, 1, 10, 10));
    QCOMPARE(v.is_currently_zoomed(), true);

    v.UndoZoom();
    QCOMPARE(v.is_currently_zoomed(), true);

    v.UndoZoom();
    QCOMPARE(v.is_currently_zoomed(), false);
}

void TestVideoDisplayZoom::test_zoom_and_unzoom()
{
    VideoDisplayZoomManager v(SirveAppConstants::VideoDisplayWidth, SirveAppConstants::VideoDisplayHeight);
    v.ZoomImage(QRect(320, 240, 320, 240)); //the bottom right quadrant
    v.ZoomImage(QRect(320, 240, 320, 240)); //the bottom right quadrant of ^

    //Ensure the last zoom is the final 1/4 of the image in both directions
    QCOMPARE(v.absolute_zoom_list.size(), 3);
    QCOMPARE(v.absolute_zoom_list[2].x, 480);
    QCOMPARE(v.absolute_zoom_list[2].width, 160);
    QCOMPARE(v.absolute_zoom_list[2].y, 360);
    QCOMPARE(v.absolute_zoom_list[2].height, 120);

    v.UndoZoom();

    //Ensure the last zoom is 1/2 of the image in both directions starting at the midpoint
    QCOMPARE(v.absolute_zoom_list.size(), 2);
    QCOMPARE(v.absolute_zoom_list[1].x, 320);
    QCOMPARE(v.absolute_zoom_list[1].width, 320);
    QCOMPARE(v.absolute_zoom_list[1].y, 240);
    QCOMPARE(v.absolute_zoom_list[1].height, 240);

    v.UndoZoom();
    //Ensure the "zoom" level is the full image
    QCOMPARE(v.absolute_zoom_list.size(), 1);
    QCOMPARE(v.absolute_zoom_list[0].x, 0);
    QCOMPARE(v.absolute_zoom_list[0].width, SirveAppConstants::VideoDisplayWidth);
    QCOMPARE(v.absolute_zoom_list[0].y, 0);
    QCOMPARE(v.absolute_zoom_list[0].height, SirveAppConstants::VideoDisplayHeight);
}

void TestVideoDisplayZoom::test_get_position_within_zoom()
{
    VideoDisplayZoomManager v(SirveAppConstants::VideoDisplayWidth, SirveAppConstants::VideoDisplayHeight);

    //Ensure pixels are at the right position in an unzoomed image
    std::vector<int> result = v.get_position_within_zoom(SirveAppConstants::VideoDisplayWidth, SirveAppConstants::VideoDisplayHeight);
    QCOMPARE(result[0], 480);
    QCOMPARE(result[1], 360);

    v.ZoomImage(QRect(320, 240, 320, 240)); //the bottom right quadrant of the window
    
    //Ensure a pixel in the middle of the zoom window is correctly found
    result = v.get_position_within_zoom(480, 360); //the center of the bottom right quadrant
    QCOMPARE(result[0], 320);
    QCOMPARE(result[1], 240);

    //Ensuring the edges/corners work
    result = v.get_position_within_zoom(320, 240);
    QCOMPARE(result[0], 0);
    QCOMPARE(result[1], 0);
    result = v.get_position_within_zoom(SirveAppConstants::VideoDisplayWidth, SirveAppConstants::VideoDisplayHeight);
    QCOMPARE(result[0], SirveAppConstants::VideoDisplayWidth);
    QCOMPARE(result[1], SirveAppConstants::VideoDisplayHeight);
    
    //Ensure points outside the zoom window aren't returned
    result = v.get_position_within_zoom(160, 120); //a point in the top left quadrant
    QCOMPARE(result[0], -1);
    QCOMPARE(result[1], -1);

    
    v.ZoomImage(QRect(320, 240, 320, 240)); //the bottom right quadrant of the remaining window

    //Ensure the same 4 tests still pass (middle, corner, corner, outside) with additional zoom
    result = v.get_position_within_zoom(560, 420);
    QCOMPARE(result[0], 320);
    QCOMPARE(result[1], 240);
    result = v.get_position_within_zoom(SirveAppConstants::VideoDisplayHeight, 360);
    QCOMPARE(result[0], 0);
    QCOMPARE(result[1], 0);
    result = v.get_position_within_zoom(SirveAppConstants::VideoDisplayWidth, SirveAppConstants::VideoDisplayHeight);
    QCOMPARE(result[0], SirveAppConstants::VideoDisplayWidth);
    QCOMPARE(result[1], 480);
    result = v.get_position_within_zoom(320, 240);
    QCOMPARE(result[0], -1);
    QCOMPARE(result[1], -1);
}

void TestVideoDisplayZoom::test_is_any_piece_within_zoom()
{
    VideoDisplayZoomManager v(SirveAppConstants::VideoDisplayWidth, SirveAppConstants::VideoDisplayHeight);

    //hyper zoom into a randomly chosen weird box in the top left
    v.ZoomImage(QRect(10, 10, 19, 11));
    //The area selected for zooming gets changed to the aspect ratio to guard against, e.g., ...
    // ... distorting the viewport by selecting a very vertical rectangle
    //So for 19/11, the width/height get reset to be 20/15
    //Now the zoom window is from:
    // 10 to 30 in the X direction with each "original" pixel now being 640/20 = 32 true pixels wide
    // 10 to 25 in the Y direction with each "original" pixel now being 480/17 = 32 true pixels high
    
    v.ZoomImage(QRect(80, 120, 320, 240));
    //Now, we zoom into a box that is exactly half the width and half the height of that viewport
    //I'll start at 1/8 of the image (x) and thus end at 5/8 of the image (x) in order to get non-integer numbers for testing
    //I'll also choose just 1/4 of the image (y) and thus 3/4 of the image (y) for the same reason
    //In theory, the new view window stretches from "original" pixel:
    // X direction (~12.5, ~22.5): Math explanation: 10 + 1/8 * 20 , 10 + 5/8 * 20
    // Y direction: (~13.75, ~21.25): Math explanation: 10 + 1/4 * 15 , 10 + 3/4 * 15

    //Reminder that the a pixel of N,M stretches at its bottom right corner to hit N+1,M+1, therefore:
    //On the left side, X direction - no part of column/pixel 11 is in the viewport but part of 12 is
    //On the right side, X direction - part of column/pixel 22 is in the viewport but no part of 23
    //On the top side, Y direction - no part of row/pixel 12 is in the viewport but part of 13 is
    //On the bottom side, Y direction - part of row/pixel 21 is in the viewport but no part of 22
    QCOMPARE(v.is_any_piece_within_zoom(11, 12), false);
    QCOMPARE(v.is_any_piece_within_zoom(11, 13), false);
    QCOMPARE(v.is_any_piece_within_zoom(11, 21), false);
    QCOMPARE(v.is_any_piece_within_zoom(11, 22), false);
    
    QCOMPARE(v.is_any_piece_within_zoom(12, 12), false);
    QCOMPARE(v.is_any_piece_within_zoom(12, 13), true);
    QCOMPARE(v.is_any_piece_within_zoom(12, 21), true);
    QCOMPARE(v.is_any_piece_within_zoom(12, 22), false);

    QCOMPARE(v.is_any_piece_within_zoom(22, 12), false);
    QCOMPARE(v.is_any_piece_within_zoom(22, 13), true);
    QCOMPARE(v.is_any_piece_within_zoom(22, 21), true);
    QCOMPARE(v.is_any_piece_within_zoom(22, 22), false);

    QCOMPARE(v.is_any_piece_within_zoom(23, 12), false);
    QCOMPARE(v.is_any_piece_within_zoom(23, 13), false);
    QCOMPARE(v.is_any_piece_within_zoom(23, 21), false);
    QCOMPARE(v.is_any_piece_within_zoom(23, 22), false);
}

void TestVideoDisplayZoom::test_zoom_resets_up_and_left_if_over_the_edge()
{
    VideoDisplayZoomManager v( SirveAppConstants::VideoDisplayWidth, SirveAppConstants::VideoDisplayHeight);

    //Remember that the zoom levels are 0-indexed, so the furthest right 10 pixels are 629-639
    //Try to zoom into a box that extends out of the viewport
    v.ZoomImage(QRect(629, 469, 20, 15));

    //Assert that the additional 10 (x) pixels and 5 (y) pixels cause the zoom box to shift up and left
    QCOMPARE(v.absolute_zoom_list[1].x, 619);
    QCOMPARE(v.absolute_zoom_list[1].y, 464);
}
