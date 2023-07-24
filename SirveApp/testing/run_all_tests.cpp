#include <QTest>

#include "test_example.h"
#include "test_bad_pixels.h"

int main()
{
    int status = 0;
    status |= QTest::qExec(new TestExample);
    status |= QTest::qExec(new TestBadPixels);

    return status;
}