#pragma once
#ifndef VIDEO_DISPLAY_ZOOM_H
#define VIDEO_DISPLAY_ZOOM_H

#include <QRect>
#include <vector>

/**
 * \brief Manages the zoom state of a video display.
 */
class VideoDisplayZoomManager
{
public:
    VideoDisplayZoomManager(int x_pixels=0, int y_pixels=0);

    const std::vector<QRect>& GetZoomList() const noexcept;
    const std::vector<QRectF>& GetAbsoluteZoomList() const noexcept;
    void ZoomImage(const QRect& area);
    QPoint GetPositionWithinZoom(int x0, int y0) const;
    QPoint GetPositionWithinZoom(const QPoint& pt) const;
    void UndoZoom();

    bool IsCurrentlyZoomed() const noexcept;
    bool IsCurrentlyZoomed(int x0, int y0) const noexcept;

private:
    int image_x = 0, image_y = 0;
    std::vector<QRect> zoom_list;
    std::vector<QRectF> absolute_zoom_list;
};

#endif
