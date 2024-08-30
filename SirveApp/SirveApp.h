#pragma once

#include <QtWidgets/QMainWindow>

#include "osm_reader.h"
#include "ABIR_Reader.h"
#include "video_display.h"
#include "histogram_plotter.h"
#include "frame_player.h"
#include "engineeringdata.h"
#include "plot_engineering_data.h"
#include "video_container.h"
#include "video_details.h"
#include "process_file.h"
#include "non_uniformity_correction.h"
#include "annotation_info.h"
#include "annotation_list_dialog.h"
#include "custom_input_dialog.h"
#include "calibration_data.h"
#include "non_uniformity_correction_external_file.h"
#include "config.h"
#include "support/jtime.h"
#include "support/qthelpers.h"
#include "workspace.h"
#include "Data_Structures.h"
#include "popout_dialog.h"
#include "tracks.h"
#include "track_management_widget.h"
#include "data_export.h"
#include "color_correction.h"
#include "windows.h"
#include "SirveApp.h"
#include "image_processing.h"
#include "auto_tracking.h"

#include <qlabel.h>
#include <qgridlayout.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <qthread.h>
#include <qobject.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qinputdialog.h>
#include <qdialogbuttonbox.h>
#include <qprogressdialog.h>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qclipboard.h>
#include <QThread>
#include <QButtonGroup.h>
#include <QRadioButton.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qgroupbox>
#include <qmenu.h>
#include <qmenubar.h>
#include <QStringList>
#include <QToolBox>
#include <QProgressBar>
#include "directory_picker.h"

class SirveApp : public QMainWindow
{
	Q_OBJECT

public:

    SirveApp(QWidget *parent = Q_NULLPTR);
    ~SirveApp();

	//Variables
	ProcessFile file_processor;
	OSMReader osm_reader;
	std::vector<Frame> osm_frames;
	AbpFileMetadata abp_file_metadata;
    Workspace *workspace;

	QWidget *main_widget;
	QGridLayout *engineering_plot_layout;

	QAction *menu_add_banner, *menu_add_primary_data, *menu_sensor_boresight, *menu_osm, *menu_change_color_tracker, *menu_change_color_banner, *menu_change_color_map, *menu_annotate;
	QAction *menu_plot_all_data, *menu_plot_primary, *menu_plot_frame_marker, *menu_plot_edit_banner;

	QThread thread_video, thread_timer;

	FramePlayer *playback_controller;
	QMenu *menu, *plot_menu;

	/* --------------------------------------------------------------------------------------------
	Qt Elements for user interface
	----------------------------------------------------------------------------------------------- */

	QString orange_styleSheet = "color: black; background-color: rgba(245, 200, 125, 255); font-weight: bold;";

	QString bold_large_styleSheet = "color: black; font-weight: bold; font-size: 12px";

    QString orange_button_styleSheet = "color: black; background-color: #fbb31a; font-weight: bold;";

	QTabWidget* tab_menu, * tab_plots;
	QDateTimeEdit* dt_epoch;
	QLabel * lbl_file_name, *lbl_lift_value, *lbl_gain_value, *lbl_max_frames, *lbl_fps, *lbl_current_epoch, *lbl_adaptive_noise_suppression, *lbl_bad_pixel_color, *lbl_current_workspace_folder;

	QLabel *lbl_adaptive_noise_suppression_status, *lbl_fixed_suppression, *lbl_bad_pixel_count, * lbl_create_track_message;
	QLabel *lbl_min_count_val, *lbl_max_count_val, *label_lift, *label_gain;
    QLabel *lbl_progress_status, *lbl_processing_description, *lbl_min_scale_value, *lbl_max_scale_value;
    QScrollArea *scrollarea_processing_description;
	QLineEdit* txt_lift_sigma, * txt_gain_sigma, *txt_frame_stack_Nframes;
	QSlider* slider_lift, * slider_gain, * slider_video;

	QLineEdit* txt_start_frame, *txt_stop_frame, *txt_moving_median_N, *txt_bad_pixel_start_frame, *txt_bad_pixel_stop_frame, *txt_ANS_number_frames, *txt_ANS_offset_frames, * txt_FNS_start_frame, * txt_FNS_stop_frame;
	QPushButton* btn_get_frames, * btn_load_osm, * btn_copy_directory, * btn_apply_epoch, * btn_reset_color_correction, * btn_ANS, * btn_FNS,
		* btn_calibration_dialog, * btn_deinterlace, * btn_deinterlace_current_frame, * btn_play, * btn_slow_back, * btn_fast_forward, * btn_prev_frame, * btn_next_frame, * btn_video_menu,
		* btn_pause, * btn_reverse, * btn_frame_save, * btn_frame_record, * btn_save_plot, * btn_plot_menu, * btn_zoom, *btn_calculate_radiance,
		* btn_workspace_load, * btn_workspace_save, * btn_undo_step, * btn_popout_video, * btn_popout_histogram, * btn_popout_engineering, * btn_replace_bad_pixels,
        * btn_import_tracks, * btn_create_track, * btn_finish_create_track, *btn_center_on_tracks, 
        * btn_center_on_brightest, *btn_frame_stack, *btn_RPCP, *btn_cancel_operation, *btn_auto_track_target;

	QCheckBox * chk_auto_lift_gain, * chk_relative_histogram, * chk_plot_primary_data, * chk_plot_show_line, * chk_plot_full_data, * chk_hide_shadow, * chk_FNS_external_file;
	QGroupBox * grpbox_auto_lift_gain, *grpbox_image_controls, *grpbox_colormap, *grpbox_overlay_controls, *grpbox_bad_pixels_correction, *grpbox_FNS_processing, *grpbox_ANS_processing, *grpbox_image_shift, *grpbox_status_area, *grpbox_image_processing;
    QGroupBox *grpbox_load_frames_area, *grpbox_progressbar_area;
    QProgressBar * progress_bar_main;

	QComboBox* cmb_deinterlace_options, * cmb_plot_yaxis, * cmb_plot_xaxis, *cmb_color_maps, * cmb_processing_states, * cmb_bad_pixels_type, * cmb_outlier_processing_type, *cmb_outlier_processing_sensitivity, *cmb_bad_pixel_color, *cmb_shadow_threshold;
    QComboBox * cmb_OSM_track_IDs, * cmb_manual_track_IDs, *cmb_track_centering_priority;
	QFrame* frame_video_player, *frame_histogram_rel, *frame_histogram_abs;
	QFrame* frame_plots;
	QRadioButton* rad_decimal, * rad_linear, * rad_scientific, * rad_log, *rad_scale_by_frame, *rad_scale_by_cube;
	QButtonGroup *data_plot_yformat, *data_plot_yloglinear;

	QCheckBox* chk_show_tracks, *chk_sensor_track_data, *chk_show_time, *chk_highlight_bad_pixels, *chk_deinterlace_confirmation;
	QComboBox* cmb_text_color, *cmb_tracker_color, *cmb_primary_tracker_color;
	QPushButton* btn_change_banner_text, * btn_add_annotations, *btn_delete_state, *btn_accumulator;

    QStackedWidget *stck_noise_suppresssion_methods;
 
    AnnotationListDialog *annotate_gui;

    QStatusBar *status_bar;
    QLabel *lbl_goto_frame, *lbl_status_start_frame, *lbl_status_stop_frame, *lbl_loaded_frames, *lbl_workspace_name, *lbl_workspace_name_field, *lbl_current_workspace_folder_field;
    QCheckBox *chk_bad_pixels_from_original;
    QLineEdit *txt_goto_frame, *txt_auto_track_start_frame, *txt_auto_track_stop_frame, *txt_accumulator_weight;

	/* --------------------------------------------------------------------------------------------
	----------------------------------------------------------------------------------------------- */

	VideoDisplay *video_display;
	EngineeringPlots *data_plots;
	EngineeringData *eng_data;
	TrackInformation *track_info;
	TrackManagementWidget *tm_widget;
	bool record_video;

    void SetupUi();
    // QWidget* SetupFileImportTab();
    QWidget* SetupColorCorrectionTab();
    QWidget* SetupTracksTab();
    QWidget* SetupProcessingTab();
    void SetupVideoFrame();
    void SetupPlotFrame();
    void setupConnections();

    void ToggleVideoPlaybackOptions(bool input);
    bool VerifyFrameSelection(int min_frame, int max_frame);
    void UpdateEpochString(QString new_epoch_string);
    void DisplayOriginalEpoch(QString new_epoch_string);
    QString CreateEpochString(std::vector<double> new_epoch);

    bool osmDataLoaded;
    void UpdatePlots();
    void UpdateGuiPostDataLoad(bool status);
    void UpdateGuiPostFrameRangeLoad(bool status);

	signals:
        void changeBanner(QString banner_text);
        void changeBannerColor(QString color);
        void changeTrackerColor(QString color);
        void directorySelected(QString directory);
        void updateVideoDisplayPinpointControls(bool status);

	public slots:

        void SetLiftAndGain(double lift, double gain);
        void HandleHistogramClick(double x0, double x1);
        void HandleAutoLiftGainCheck(int state);
        void HandleLiftSliderToggled();
        void HandleGainSliderToggled();

        void SaveWorkspace();
        void LoadWorkspace();
        void ImportTracks();
        void HandleTrackRemoval(int track_id);
        void HandleManualTrackRecoloring(int track_id, QColor color);

        void HandleAbpFileSelected();
        bool ValidateAbpFiles(QString path_to_image_file);
        void UiLoadAbirData();
        void ExecuteAdaptiveNoiseSuppression();
        void ExecuteDeinterlace();
        void ExecuteDeinterlaceCurrent();
        void ExecuteCenterOnTracks();
        void ExecuteCenterOnBrightest();
        void ExecuteFrameStacking();
        void ExecuteFixedNoiseSuppression();
        void ExecuteRPCPNoiseSuppression();
        void ExecuteAccumulatorNoiseSuppression();
        void ExecuteAutoTracking();
        void HandleFrameNumberChangeInput();

        void StartStopVideoRecording();
        void HandleZoomOnVideoToggle();
        // void HandleCalculationOnVideoToggle();
        void HandleProcessingNewStateSelected();
        void ClearZoomAndCalculationButtons();

        void UpdateFps();
        void ResetColorCorrection();

        void HandlePlotFullDataToggle();
        void HandlePlotPrimaryOnlyToggle();
        void HandlePlotCurrentFrameMarkerToggle();
   
        void SetDataTimingOffset();
        void ChangeWorkspaceDirectory();
        void CloseWindow();

        void SavePlot();
        void SaveFrame();
        void HandleRelativeHistogramToggle(bool input);
        void ApplyEpochTime();
        void ApplyFixedNoiseSuppressionFromExternalFile();
        void ReceiveNewBadPixels(std::vector<unsigned int> new_pixels);
        void ReceiveNewGoodPixels(std::vector<unsigned int> pixels);

        void HandleFrameChange();
        void HandleOsmTracksToggle();
        void HandleNewProcessingState(QString state_name, QString combobox_state_name, int index);
        void HandlePlayerStateChanged(bool status);
        void HandleProcessingStateRemoval(ProcessingMethod method, int index);
        void HandlePopoutVideoClosed();
        void HandlePopoutHistogramClosed();
        void HandlePopoutEngineeringClosed();
        void HandleZoomAfterSlider();
        
        void SirveApp::HandleProcessingStatesCleared();
        void SirveApp::HandleWorkspaceDirChanged(QString workspaceDirectory);

private:
	ColorMap video_colors;
	ColorMapDisplay *color_map_display;

	QVBoxLayout *histogram_abs_layout, *vlayout_tab_histogram;
	QClipboard *clipboard;

	PopoutDialog *popout_video;
	PopoutDialog *popout_histogram;
	PopoutDialog *popout_engineering;

	HistogramLinePlot *histogram_plot;

	ConfigValues config_values;

	int currently_editing_or_creating_track_id;

    void CreateMenuActions();
    void EditColorMap();
    void EditBannerText();
    void EditPlotText();
    void ExportPlotData();
    void EditBannerColor();
    void EditTrackerColor();
	void edit_bad_pixel_color();
	void handle_outlier_processing_change();

    void AnnotateVideo();

    void HandlePopoutVideoClick(bool checked);
    void OpenPopoutVideoDisplay();

    void HandlePopoutHistogramClick(bool checked);
    void OpenPopoutHistogramPlot();

    void HandlePopoutEngineeringClick(bool checked);
    void OpenPopoutEngineeringPlot();

    void ResizeUi();

	QMenu *file_menu, *menu_workspace, *menu_export, *menu_settings;
    QAction *action_close, *action_set_timing_offset, *action_change_workspace_directory, *action_load_OSM, * action_load_frames, *action_load_workspace, *action_save_workspace, *action_export_current_frame, *action_export_frame_range;
    QAction * action_export_all_frames;

    int GetCurrentColorIndex(QVector<QString> colors, QColor input_color);
    int ConvertFrameNumberTextToInt(QString input);
	CalibrationData calibration_model;

    void LoadOsmData();
    void LoadAbirData(int start_frame, int stop_frame);

    void HandleBadPixelReplacement();
    void ReplaceBadPixels(std::vector<unsigned int> & pixels_to_replace,int source_state_ind);
    
    void ApplyFixedNoiseSuppression(QString image_path, QString file_path, unsigned int frame0, unsigned int min_frame, unsigned int max_frame, int processing_state_idx);
    void ApplyAdaptiveNoiseSuppression(int relative_start_frame, int num_frames, bool hide_shadow_choice, int shadow_sigma_thresh, int processing_state_idx);
    void ApplyRPCPNoiseSuppression(int processing_state_idx);
    void ApplyDeinterlacing(int processing_state_idx);
    void ApplyAccumulatorNoiseSuppression(double weight, int source_state_idx);
    void ApplyDeinterlacingCurrent();
    void CenterOnTracks(QString trackTypePriority, int track_id, std::vector<std::vector<int>> & track_centered_offsets,boolean findAnyTrack, int processing_state_idx);
    void CenterOnBrightest(std::vector<std::vector<int>> & brightest_centered_offsets, int processing_state_idx);
    void FrameStacking(int num_frames, int processing_state_idx);
    void ExportFrame();
    void ExportFrameRange();
    void ExportAllFrames();
 
    void EnableEngineeringPlotOptions();
    void ExitTrackCreationMode();
    void HandleCreateTrackClick();
    void HandleFinishCreateTrackClick();
    void PrepareForTrackCreation(int track_id);

    void HandleFrameNumberChange(unsigned int new_frame_number);

    void UpdateGlobalFrameVector();

    void DeleteState();

    QString abpimage_file_base_name;
};
