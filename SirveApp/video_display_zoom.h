#pragma once
#ifndef VIDEO_DISPLAY_ZOOM_H
#define VIDEO_DISPLAY_ZOOM_H

#include <vector>
#include <cmath>
#include <QRect>

struct AbsoluteLocation {
    double x;
    double y;
};

struct AbsoluteZoomInfo {
	double x, y, width, height;
};

class VideoDisplayZoomManager {
    public:
        VideoDisplayZoomManager(int x_pixels, int y_pixels);
        ~VideoDisplayZoomManager();

	    std::vector<QRect> zoom_list;
        std::vector<AbsoluteZoomInfo> absolute_zoom_list;

        void ZoomImage(QRect area);
        std::vector<int> GetPositionWithinZoom(int x0, int y0);
        void UndoZoom();

        bool is_currently_zoomed();
        bool is_any_piece_within_zoom(int x0, int y0);

    private:
        int image_x, image_y;
};

#endif
