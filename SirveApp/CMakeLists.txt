set(HEADERS
        abir_reader.h
        abpnuc_reader.h
        abp_version_dlg.h
        annotation_edit_dialog.h
        annotation_list_dialog.h
        annotation_stencil.h
        auto_tracking.h
        binary_reader.h
        calibration_data.h
        chart_metadata.h
        classification.h
        clickable_chartview.h
        color_correction.h
        color_map.h
        color_scheme.h
        config.h
        constants.h
        custom_input_dialog.h
        data_export.h
        data_structures.h
        engineering_data.h
        enhanced_label.h
        enums.h
        playback_controller.h
        histogram_plotter.h
        image_processing.h
        location_input.h
        new_location.h
        non_uniformity_correction_external_file.h
        osm_reader.h
        plot_engineering_data.h
        popout_dialog.h
        process_file.h
        processing_state.h
        Section.h
        shared_tracking_functions.h
        SirveApp.h
        track_management_widget.h
        tracks.h
        video_container.h
        video_details.h
        video_display.h
        video_display_zoom.h
        wait_cursor.h
        workspace.h
        support/az_el_calculation.h
        support/earth.h
        support/euler.h
        support/jtime.h
        support/qt_elements_with_ids.h
        support/qthelpers.h
)

set(SOURCES
        abir_reader.cpp
        abpnuc_reader.cpp
        abp_version_dlg.cpp
        annotation_edit_dialog.cpp
        annotation_info.cpp
        annotation_list_dialog.cpp
        annotation_stencil.cpp
        auto_tracking.cpp
        binary_reader.cpp
        calibration_data.cpp
        classification.cpp
        clickable_chartview.cpp
        color_correction.cpp
        color_map.cpp
        color_scheme.cpp
        config.cpp
        custom_input_dialog.cpp
        data_export.cpp
        engineering_data.cpp
        enhanced_label.cpp
        playback_controller.cpp
        histogram_plotter.cpp
        image_processing.cpp
        location_input.cpp
        main.cpp
        new_location.cpp
        non_uniformity_correction_external_file.cpp
        osm_reader.cpp
        plot_engineering_data.cpp
        popout_dialog.cpp
        process_file.cpp
        processing_state.cpp
        Section.cpp
        shared_tracking_functions.cpp
        SirveApp.cpp
        track_management_widget.cpp
        tracks.cpp
        video_container.cpp
        video_display.cpp
        video_display_zoom.cpp
        wait_cursor.cpp
        workspace.cpp
        support/az_el_calculation.cpp
        support/earth.cpp
        support/euler.cpp
        support/jtime.cpp
        support/qt_elements_with_ids.cpp
        support/qthelpers.cpp
)

set(UI_SOURCES location_gui.ui new_location.ui abp_version.ui)
set(RESOURCES SirveApp.qrc)

# TODO: AUTOMOC doesn't work, investigate
# qt5_wrap_cpp(MOC_SOURCES ${HEADERS})

add_executable(SirveApp ${SOURCES} ${HEADERS} ${MOC_SOURCES} ${UI_SOURCES} ${RESOURCES})

if(WIN32)
    set_target_properties(SirveApp PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

target_link_libraries(SirveApp
        PRIVATE
            Qt5::Core
            Qt5::Gui
            Qt5::Widgets
            Qt5::Charts
            FFTW::fftw3-3
            BLAS::BLAS
            ${OpenCV_LIBS}
)

target_include_directories(SirveApp PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
    "${ARMADILLO_INCLUDE_DIRS}"
    "${OpenCV_INCLUDE_DIRS}"
)

set_target_properties(SirveApp PROPERTIES
    AUTOGEN_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
    # TODO: AUTOMOC doesn't work, investigate
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

add_custom_command(TARGET SirveApp POST_BUILD
    COMMENT "Copying resources..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/icons" "$<TARGET_FILE_DIR:SirveApp>/icons"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/config" "$<TARGET_FILE_DIR:SirveApp>/config"
)

if(WIN32)
    install(TARGETS SirveApp)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/icons" TYPE DATA)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/config" TYPE DATA)

    # OpenBLAS is not auto-detected
    cmake_path(REPLACE_EXTENSION BLAS_openblas_LIBRARY ".dll" OUTPUT_VARIABLE OPENBLAS_DLL)

    # Copy runtime dependencies to the output directory
    add_custom_command(TARGET SirveApp POST_BUILD
        COMMENT "Copying runtime dependencies..."
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:SirveApp> $<TARGET_RUNTIME_DLLS:SirveApp>
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:SirveApp> "${OPENBLAS_DLL}"
        COMMAND_EXPAND_LISTS
    )

    install(FILES "${OPENBLAS_DLL}" $<TARGET_RUNTIME_DLLS:SirveApp> TYPE BIN)

    # Build the windeployqt command
    set(WINDEPLOYQT_COMMAND "${QT5_WINDEPLOYQT}" "--no-translations" "--no-system-d3d-compiler" "--no-opengl-sw")

    if(NOT SIRVE_INSTALL_VC_REDIST)
        list(APPEND WINDEPLOYQT_COMMAND "--no-compiler-runtime" "--no-libraries")
    endif()

    if("${CMAKE_BUILD_TYPE}" STREQUAL "" OR "${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
        list(APPEND WINDEPLOYQT_COMMAND "--release")
    elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        list(APPEND WINDEPLOYQT_COMMAND "--debug")
    endif()

    list(APPEND WINDEPLOYQT_COMMAND "$<TARGET_FILE:SirveApp>")

    # Copy Qt plugins to the output directory and save the manifest
    add_custom_command(TARGET SirveApp POST_BUILD
        COMMENT "Copying Qt plugins..."
        COMMAND ${WINDEPLOYQT_COMMAND} --json "$<TARGET_FILE:SirveApp>" > "plugins_manifest.json"
        COMMAND_EXPAND_LISTS
    )

    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/platforms" TYPE BIN)

    if(SERVE_INSTALL_VC_REDIST)
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/vc_redist.x64.exe" TYPE BIN)
    endif()

    # TODO: These do not appear to be necessary, maybe remove them?
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/iconengines" TYPE BIN)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imageformats" TYPE BIN)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/styles" TYPE BIN)
endif()
