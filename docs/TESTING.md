# Testing

SIRVE was historically not covered by tests, but tests are slowly being introduced to cover key features and capabilities. At present, tests in SIRVE are written and executed using the Qt QTest framework.

## Adding Tests

Tests are added by creating a new class that derives from QObject, adding it to the QTest::qExec runners in `run_all_tests.cpp`, and then creating methods on that test class with asserts, e.g. QCOMPARE or QVERIFY.

## Compiling/Building for Tests

The unit tests are compiled and built similarly to the main application - namely, via directives in the SirveApp.pro file. A "test" block exists that knows how to build the files required for testing. To compile/build for test, instead of running qmake normally, run:

..\5.15.2\msvc2019_64\bin\qmake.exe "CONFIG += test"

This will create Makefiles that are geared to building the unit tests. Then, simply running `nmake` (and if needed, `nmake install` at least once) will create a `..\tests` directory with the `run_all_tests.exe` executable in it. This executable can be run to execute all tests.