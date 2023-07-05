#include <QtTest>

#include "bad_pixel_identification.h"

class TestBadPixelIdentification: public QObject {
    Q_OBJECT
public:
    TestBadPixelIdentification();
private slots:
    void test_get_dead_pixel_mask();
private:
    std::vector<std::vector<uint16_t>> generate_test_input_with_dead_pixels();
};