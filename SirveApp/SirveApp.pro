TEMPLATE = app
INCLUDEPATH += .

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#CONFIG += c++17 \
#            console
CONFIG += c++17

HEADERS +=  abir_reader.h \
            abpnuc_reader.h \
            annotation_edit_dialog.h \
            annotation_list_dialog.h \
            annotation_stencil.h \
            auto_tracking.h\
            binary_reader.h \
            calibration_data.h \
            chart_metadata.h \
            classification.h \
            clickable_chartview.h \
            color_correction.h \
            color_map.h \
            color_scheme.h \
            config.h \
            constants.h \
            custom_input_dialog.h \
            data_export.h \
            data_structures.h \
            engineering_data.h \
            enhanced_label.h \
            enums.h \
            fixed_noise_correction_external_file.h \
            frame_player.h \
            histogram_plotter.h \
            image_processing.h \
            location_input.h \
            new_location.h \
            osm_reader.h \
            plot_designer.h \
            playback_controller.h \
            plot_engineering_data.h \
            plot_palette.h \
            popout_dialog.h \
            process_file.h \
            processing_state.h \
            Section.h \
            shared_tracking_functions.h\
            SirveApp.h \
            quantity.h \
            single_check_list.h \
            state_manager.h \
            track_management_widget.h \
            tracks.h \
            video_details.h \
            video_display.h \
            video_display_zoom.h \
            wait_cursor.h \
            video_player.h \
            workspace.h \
            support/az_el_calculation.h \
            support/earth.h \
            support/euler.h \
            support/jtime.h \
            support/qt_elements_with_ids.h \
            support/qthelpers.h

FORMS += location_gui.ui new_location.ui

SOURCES +=  abir_reader.cpp \
            abpnuc_reader.cpp \
            annotation_edit_dialog.cpp \
            annotation_info.cpp \
            annotation_list_dialog.cpp \
            annotation_stencil.cpp \
            auto_tracking.cpp\
            binary_reader.cpp \
            calibration_data.cpp \
            classification.cpp \
            clickable_chartview.cpp \
            color_correction.cpp \
            color_map.cpp \
            color_scheme.cpp \
            config.cpp \
            custom_input_dialog.cpp \
            data_export.cpp \
            engineering_data.cpp \
            enhanced_label.cpp \
            enums.cpp \
            fixed_noise_correction_external_file.cpp \
            histogram_plotter.cpp \
            image_processing.cpp \
            location_input.cpp \
            main.cpp \
            new_location.cpp \
            osm_reader.cpp \
            plot_designer.cpp \
            playback_controller.cpp \
            plot_engineering_data.cpp \
            plot_palette.cpp \
            popout_dialog.cpp \
            process_file.cpp \
            processing_state.cpp \
            Section.cpp \
            shared_tracking_functions.cpp\
            SirveApp.cpp \
            quantity.cpp \
            single_check_list.cpp \
            state_manager.cpp \
            track_management_widget.cpp \
            tracks.cpp \
            video_display.cpp \
            video_display_zoom.cpp \
            wait_cursor.cpp \
            video_player.cpp \
            workspace.cpp \
            support/az_el_calculation.cpp \
            support/earth.cpp \
            support/euler.cpp \
            support/jtime.cpp \
            support/qt_elements_with_ids.cpp \
            support/qthelpers.cpp

QT += charts widgets

LIBS += -lshell32

LIBS += "-L../6.6.0/msvc2019_64/lib/"

include(..\jkqtPlotter\lib\jkqtplotter.pri)
include(..\jkqtPlotter\lib\jkqtcommon.pri)
include(..\jkqtPlotter\lib\jkqtfastplotter.pri)

include(..\jkqtPlotter\lib\jkqtcommon_statistics_and_math.pri)
include(..\jkqtPlotter\lib\jkqtmath.pri)
include(..\jkqtPlotter\lib\jkqtmathtext.pri)

include(..\jkqtPlotter\examples\libexampletools\libexampletools.pri)

#To copy all the required files alongside the exe, run `nmake install`
dlls.files = "../6.6.0/msvc2019_64/bin/Qt6Charts.dll" \
            "../6.6.0/msvc2019_64/bin/Qt6Core.dll" \
            "../6.6.0/msvc2019_64/bin/Qt6Gui.dll" \
            "../6.6.0/msvc2019_64/bin/Qt6Widgets.dll" \
            "../6.6.0/msvc2019_64/bin/Qt6OpenGL.dll" \
            "../6.6.0/msvc2019_64/bin/Qt6OpenGLWidgets.dll" \
            "../6.6.0/msvc2019_64/bin/Qt6PrintSupport.dll" \
            "../6.6.0/msvc2019_64/bin/Qt6Svg.dll" \
            "../6.6.0/msvc2019_64/bin/Qt6Xml.dll" \
            "../fftw-3.3.5-dll64/libfftw3-3.dll" \
            "../fftw-3.3.5-dll64/libfftw3f-3.dll" \
            "../fftw-3.3.5-dll64/libfftw3l-3.dll" \
            "../opencv/build/x64/vc16/bin/opencv_world470.dll" \
            "../armadillo-12.2.0/examples/lib_win64/libopenblas.dll"

test {
    message(Building for test)

    TARGET = run_all_tests
    DESTDIR = ../tests

    QT += testlib

    HEADERS -= SirveApp.h

    SOURCES -= main.cpp \
        SirveApp.cpp

    HEADERS += \
        testing/test_example.h \
        testing/test_az_el_calculation.h \
        testing/test_bad_pixels.h \
        testing/test_video_display_zoom.h

    SOURCES += \
        testing/run_all_tests.cpp \
        testing/test_az_el_calculation.cpp \
        testing/test_example.cpp \
        testing/test_bad_pixels.cpp \
        testing/test_video_display_zoom.cpp

    dlls.files += "../6.6.0/msvc2019_64/bin/Qt5Test.dll"
}
else {
    message(Building normally)

    TARGET = SirveApp
    DESTDIR = ../release

    qwindows.files = "../6.6.0/msvc2019_64/plugins/qwindows.dll"
    qwindows.path = $${DESTDIR}/platforms
    INSTALLS += qwindows

    icons.files = "icons/*"
    icons.path = $${DESTDIR}/icons
    INSTALLS += icons

    configuration.files = "config/*"
    configuration.path = $${DESTDIR}/config
    INSTALLS += configuration
}

dlls.path = $${DESTDIR}
INSTALLS += dlls

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../armadillo-12.2.0/examples/lib_win64/ -llibopenblas
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../armadillo-12.2.0/examples/lib_win64/ -llibopenblas

INCLUDEPATH += $$PWD/../armadillo-12.2.0/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../opencv/build/x64/vc16/lib/ -lopencv_world470
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../opencv/build/x64/vc16/lib/ -lopencv_world470d

INCLUDEPATH += $$PWD/../opencv/build/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../fftw-3.3.5-dll64/ -llibfftw3-3
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../fftw-3.3.5-dll64/ -llibfftw3-3

INCLUDEPATH += $$PWD/../fftw-3.3.5-dll64
DEPENDPATH += $$PWD/../fftw-3.3.5-dll64

INCLUDEPATH += $$PWD\..\jkqtPlotter\lib \
                $$PWD\..\jkqtPlotter\examples\libexampletools

HEADERS += $$PWD\..\jkqtPlotter\examples\libexampletools\jkqtpexampleapplication.h

RESOURCES += \
    SirveApp.qrc
