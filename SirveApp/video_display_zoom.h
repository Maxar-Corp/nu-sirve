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
    VideoDisplayZoomManager(int width=0, int height=0);

    void Clear(int width, int height);
    const std::vector<QRect>& GetZoomList() const noexcept;
    const std::vector<QRectF>& GetAbsoluteZoomList() const noexcept;
    void ZoomImage(const QRect& area);
    QPoint GetPositionWithinZoom(int x0, int y0) const;
    QPoint GetPositionWithinZoom(const QPoint& pt) const;
    void UndoZoom();

    bool IsCurrentlyZoomed() const noexcept;
    bool IsCurrentlyZoomed(int x0, int y0) const noexcept;

    bool ZoomListExists();

private:
    int image_width_ = 0, image_height_ = 0;
    std::vector<QRect> zoom_list_;
    std::vector<QRectF> absolute_zoom_list_;
};

#endif
