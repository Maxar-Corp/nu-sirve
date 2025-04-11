cmake_minimum_required(VERSION 3.22)

project(Sirve
        DESCRIPTION "A tool used for visual and scientific analysis of data from EO/IR sensors."
        HOMEPAGE_URL "www.maxar.com"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SIRVE_INSTALL_VC_REDIST "Install Visual C++ Redistributable with the application" OFF)
set(QT_VERSION_MAJOR 5 CACHE STRING "Qt version to use")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
endif()

if(QT_VERSION_MAJOR VERSION_EQUAL 6)
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/6.6.0/msvc2019_64")
else ()
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/5.15.2/msvc2019_64")
endif()

set(QT_NAME "Qt${QT_VERSION_MAJOR}")

list(PREPEND CMAKE_PREFIX_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/6.6.0/msvc2019_64"
        "${CMAKE_CURRENT_SOURCE_DIR}/opencv/build/x64/vc16/lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/fftw-3.3.5-dll64"
        "${CMAKE_CURRENT_SOURCE_DIR}/armadillo-12.2.0/examples/lib_win64"
        "${CMAKE_CURRENT_SOURCE_DIR}/armadillo-12.2.0"
        "${CMAKE_CURRENT_SOURCE_DIR}/JKQtPlotter/${QT_NAME}/${CMAKE_BUILD_TYPE}/lib/cmake"
)

list(PREPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

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

find_package(${QT_NAME} COMPONENTS Core Gui Widgets Charts REQUIRED)

if(WIN32)
    # TODO: This is unnecessary on for Qt6, since windeployqt is included as Qt6::windeployqt
    # Build the windeployqt command
    if(QT_VERSION_MAJOR VERSION_LESS 6)
        set(WINDEPLOYQT_EXEC "${CMAKE_CURRENT_SOURCE_DIR}/5.15.2/msvc2019_64/bin/windeployqt.exe")
    else()
        set(WINDEPLOYQT_EXEC Qt6::windeployqt)
    endif()

    set(CMAKE_INSTALL_BINDIR .)
    set(CMAKE_INSTALL_LIBDIR .)
    set(CMAKE_INSTALL_DATAROOTDIR .)
endif()

add_subdirectory(SirveApp)
