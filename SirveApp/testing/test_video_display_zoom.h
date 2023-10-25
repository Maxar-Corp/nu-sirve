#include <QtTest>

#include "video_display_zoom.h"

class TestVideoDisplayZoom: public QObject {
    Q_OBJECT
public:
    TestVideoDisplayZoom();
private slots:
    void test_is_currently_zoomed();
    void test_zoom_and_unzoom();
    void test_get_position_within_zoom();
    void test_is_any_piece_within_zoom();
    void test_zoom_resets_up_and_left_if_over_the_edge();
};