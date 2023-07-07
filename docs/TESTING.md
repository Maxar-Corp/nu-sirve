# Testing

SIRVE was historically written without any automated testing. New features and scientific compute capabilities going forward should be covered by automated tests. At present, tests in SIRVE are written and executed using the Qt QTest framework.

## Compiling/Building for Tests

The unit tests are compiled and built similarly to the main application - namely, via directives in the SirveApp.pro file. A "test" block exists that knows how to build the files required for testing. To compile/build for test, instead of running qmake normally, run:

..\5.15.2\msvc2019_64\bin\qmake.exe "CONFIG += test"

This will create Makefiles that are geared to building the unit tests. Then, simply running `nmake` (and if needed, `nmake install` at least once) will create a `..\tests` directory with the `run_all_tests.exe` executable in it. This executable can be run to execute all tests.