#pragma once
#ifndef VIDEO_DISPLAY_ZOOM_H
#define VIDEO_DISPLAY_ZOOM_H

#include <vector>
#include <cmath>

#include <QRect>

struct absolute_location {
    double x;
    double y;
};

struct absolute_zoom_info {
	double x, y, width, height;
};

class VideoDisplayZoomManager {
    public:
        VideoDisplayZoomManager(int x_pixels, int y_pixels);
        ~VideoDisplayZoomManager();

	    std::vector<QRect> zoom_list;
	    std::vector<absolute_zoom_info> absolute_zoom_list;

        bool is_any_piece_within_zoom(int x0, int y0);
        std::vector<int> get_position_within_zoom(int x0, int y0);
        void zoom_image(QRect area);
	    void unzoom();

        bool is_currently_zoomed();

    private:
        int image_x, image_y;
};

#endif