#include <QtTest>

#include "bad_pixels.h"

class TestBadPixels: public QObject {
    Q_OBJECT
public:
    TestBadPixels();
private slots:
    void test_identify_dead_pixels();
    void test_replace_bad_pixels_center();
    void test_replace_bad_pixels_edges();
    void test_replace_bad_pixels_overlapping();
private:
    std::vector<std::vector<uint16_t>> generate_test_input_with_dead_pixels();
};