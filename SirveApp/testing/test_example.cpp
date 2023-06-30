#include "test_example.h"

TestExample::TestExample()
{
    
}

void TestExample::testOne()
{
    int actual = 2 + 5;

    int expected = 7;
    
    QCOMPARE(actual, expected);
}