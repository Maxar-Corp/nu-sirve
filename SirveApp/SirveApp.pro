TEMPLATE = app
INCLUDEPATH += .

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

CONFIG += console

HEADERS += abir_reader.h \
            abpnuc_reader.h \
            annotation_edit_dialog.h \
            annotation_list_dialog.h \
            bad_pixels.h \
            binary_file_reader.h \
            calibration_data.h \
            clickable_chartview.h \
            color_correction.h \
            color_map.h \
            color_scheme.h \
            config.h \
            custom_input_dialog.h \
            Data_Structures.h \
            data_export.h \
            deinterlace.h \
            deinterlace_type.h \
            directory_picker.h \
            engineering_data.h \
            enhanced_label.h \
            histogram_plotter.h \
            location_input.h \
            new_location.h \
            noise_suppression.h \
            non_uniformity_correction_external_file.h \
            non_uniformity_correction.h \
            osm_reader.h \
            playback.h \
            plot_engineering_data.h \
            popout_dialog.h \
            process_file.h \
            processing_state.h \
            Section.h \
            SirveApp.h \
            track_management_widget.h \
            tracks.h \
            video_container.h \
            video_details.h \
            video_display.h \
            video_display_zoom.h \
            workspace.h \
            support/az_el_calculation.h \
            support/earth.h \
            support/euler.h \
            support/jtime.h \
            support/qt_elements_with_ids.h \
            support/qthelpers.h

FORMS += location_gui.ui new_location.ui

SOURCES += abir_reader.cpp \
            abpnuc_reader.cpp \
            annotation_edit_dialog.cpp \
            annotation_info.cpp \
            annotation_list_dialog.cpp \
            bad_pixels.cpp \
            binary_file_reader.cpp \
            calibration_data.cpp \
            clickable_chartview.cpp \
            color_correction.cpp \
            color_map.cpp \
            color_scheme.cpp \
            config.cpp \
            custom_input_dialog.cpp \
            data_export.cpp \
            deinterlace.cpp \
            directory_picker.cpp \
            engineering_data.cpp \
            enhanced_label.cpp \
            histogram_plotter.cpp \
            location_input.cpp \
            main.cpp \
            new_location.cpp \
            noise_suppression.cpp \
            non_uniformity_correction_external_file.cpp \
            non_uniformity_correction.cpp \
            osm_reader.cpp \
            playback.cpp \
            plot_engineering_data.cpp \
            popout_dialog.cpp \
            process_file.cpp \
            processing_state.cpp \
            Section.cpp \
            SirveApp.cpp \
            track_management_widget.cpp \
            tracks.cpp \
            video_container.cpp \
            video_display.cpp \
            video_display_zoom.cpp \
            workspace.cpp \
            support/az_el_calculation.cpp \
            support/earth.cpp \
            support/euler.cpp \
            support/jtime.cpp \
            support/qt_elements_with_ids.cpp \
            support/qthelpers.cpp

QT += charts widgets

LIBS += "-L../5.15.2/msvc2019_64/lib/"

#To copy all the required files alongside the exe, run `nmake install`
dlls.files = "../5.15.2/msvc2019_64/bin/Qt5Charts.dll" \
            "../5.15.2/msvc2019_64/bin/Qt5Core.dll" \
            "../5.15.2/msvc2019_64/bin/Qt5Gui.dll" \
            "../5.15.2/msvc2019_64/bin/Qt5Widgets.dll" \
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

    dlls.files += "../5.15.2/msvc2019_64/bin/Qt5Test.dll"
}
else {
    message(Building normally)

    TARGET = SirveApp
    DESTDIR = ../release

    qwindows.files = "../5.15.2/msvc2019_64/plugins/platforms/qwindows.dll"
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
