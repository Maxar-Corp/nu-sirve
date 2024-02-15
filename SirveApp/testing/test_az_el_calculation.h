#include <QtTest>

#include "support/az_el_calculation.h"

class TestAzElCalculation: public QObject {
    Q_OBJECT
public:
    TestAzElCalculation();
private slots:
    void test_azimuth_elevation_calculation();
};