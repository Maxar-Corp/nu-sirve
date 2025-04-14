cmake_minimum_required(VERSION 3.22)

project(Sirve
        DESCRIPTION "A tool used for visual and scientific analysis of data from EO/IR sensors."
        HOMEPAGE_URL "www.maxar.com"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SIRVE_INSTALL_VC_REDIST "Install Visual C++ Redistributable with the application" OFF)

set(Qt6_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/6.6.0/msvc2019_64/" CACHE PATH "Qt6 installation directory")
set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/opencv/build/x64/vc16/lib" CACHE PATH "OpenCV installation directory")
set(FFTW_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/fftw-3.3.5-dll64" CACHE PATH "FFTW installation directory")
set(ARMADILLO_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/armadillo-12.2.0/include" CACHE PATH "Armadillo include directory")
set(BLAS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/armadillo-12.2.0/examples/lib_win64" CACHE PATH "OpenBLAS installation directory")

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(WIN32)
    find_package(OpenCV COMPONENTS world REQUIRED)
else()
    find_package(OpenCV COMPONENTS core imgproc video videoio highgui photo REQUIRED)
endif()

find_package(FFTW COMPONENTS REQUIRED)

set(BLA_VENDOR OpenBLAS)
find_package(BLAS)

set(OPENBLAS_PROVIDES_LAPACK ON)
find_package(Armadillo REQUIRED)

set(Qt6_DIR "${Qt6_ROOT}/lib/cmake/Qt6" CACHE PATH "Qt6 cmake directory")
mark_as_advanced(Qt6_DIR)

find_package(Qt6 COMPONENTS Core Gui Widgets Charts REQUIRED)
if(WIN32)
    # TODO: This is unnecessary on for Qt6, since windeployqt is included as Qt6::windeployqt
    set(QT6_WINDEPLOYQT "${Qt6_ROOT}/bin/windeployqt.exe")

    set(CMAKE_INSTALL_BINDIR .)
    set(CMAKE_INSTALL_LIBDIR .)
    set(CMAKE_INSTALL_DATAROOTDIR .)
endif()


include_directories("${CMAKE_CURRENT_SOURCE_DIR}/jkqtPlotter/lib")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/jkqtPlotter/examples/libexampletools")
include_directories("${Qt6_ROOT}\\include\\QtPrintSupport\\")

add_subdirectory(jkqtPlotter)
add_subdirectory(SirveApp)
