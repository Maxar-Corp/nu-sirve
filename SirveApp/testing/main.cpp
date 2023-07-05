#include <QTest>

#include "test_example.h"
#include "test_bad_pixel_identification.h"

int main()
{
    int status = 0;
    status |= QTest::qExec(new TestExample);
    status |= QTest::qExec(new TestBadPixelIdentification);

    return status;
}