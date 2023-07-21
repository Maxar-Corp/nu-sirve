#include <QtTest>

#include "bad_pixels.h"

class TestBadPixels: public QObject {
    Q_OBJECT
public:
    TestBadPixels();
private slots:
    void test_identify_dead_pixels();
private:
    std::vector<std::vector<uint16_t>> generate_test_input_with_dead_pixels();
};