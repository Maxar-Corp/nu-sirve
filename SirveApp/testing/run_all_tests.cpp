#include <QTest>

#include "test_example.h"
#include "test_bad_pixels.h"
#include "test_az_el_calculation.h"
#include "test_video_display_zoom.h"

int main()
{
    int status = 0;
    status |= QTest::qExec(new TestExample);
    status |= QTest::qExec(new TestBadPixels);
    status |= QTest::qExec(new TestVideoDisplayZoom);
    status |= QTest::qExec(new TestAzElCalculation);

    return status;
}