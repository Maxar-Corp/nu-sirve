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
#include "noise_suppression.h"
#include "deinterlace.h"
#include "deinterlace_type.h"
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
#include "bad_pixels.h"
#include "tracks.h"
#include "track_management_widget.h"
#include "data_export.h"
#include "color_correction.h"
#include "windows.h"
#include "SirveApp.h"
#include "image_processing.h"

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
	// QString dark_green_button_styleSheet = "color: white; background-color: #1a3533; font-weight: bold;";
	// QString olive_green_button_styleSheet = "color: white; background-color: #555121; font-weight: bold;";
	// QString dark_blue_button_styleSheet = "color: white; background-color: #0b2139; font-weight: bold;";
	// QString dark_orange_button_styleSheet = "color: white; background-color: #743203; font-weight: bold;";
	// QString track_button_styleSheet = "color: white; background-color: #002147; font-weight: bold;";
	// QString dark_red_stop_styleSheet = "color: white; background-color: #331a1a; font-weight: bold;";

	QString orange_styleSheet = "color: black; background-color: #fbb31a; font-weight: bold;";

	QString bold_large_styleSheet = "color: black; font-weight: bold; font-size: 12px";


	QTabWidget* tab_menu, * tab_plots;
	QDateTimeEdit* dt_epoch;
	QLabel * lbl_file_name, *lbl_lift_value, *lbl_gain_value, *lbl_max_frames, *lbl_fps, *lbl_current_epoch, *lbl_adaptive_noise_suppression, *lbl_bad_pixel_color, *lbl_current_workspace_folder;

	QLabel *lbl_adaptive_noise_suppression_status, *lbl_fixed_suppression, *lbl_bad_pixel_count, * lbl_create_track_message, * lbl_bad_pixel_type,  * lbl_bad_pixel_sensitivity,  * lbl_bad_pixel_method, *lbl_moving_median_window_length;
	QLabel *lbl_bad_pixel_start_frame, *lbl_bad_pixel_stop_frame, *lbl_ANS_number_frames, *lbl_ANS_offset_frames, *lbl_FNS_start_frame, * lbl_FNS_stop_frame, * lbl_ANS_shadow_threshold, *lbl_min_count_val, *lbl_max_count_val, *label_lift, *label_gain;

	QLineEdit* txt_lift_sigma, * txt_gain_sigma;
	QSlider* slider_lift, * slider_gain, * slider_video;

	QLineEdit* txt_start_frame, * txt_end_frame, * txt_moving_median_N, *txt_bad_pixel_start_frame, *txt_bad_pixel_end_frame, *txt_ANS_number_frames, *txt_ANS_offset_frames, * txt_FNS_start_frame, * txt_FNS_end_frame;
	QPushButton* btn_get_frames, * btn_load_osm, * btn_copy_directory, * btn_apply_epoch, * btn_reset_color_correction, * btn_ANS, * btn_FNS,
		* btn_calibration_dialog, * btn_deinterlace, * btn_play, * btn_slow_back, * btn_fast_forward, * btn_prev_frame, * btn_next_frame, * btn_video_menu,
		* btn_pause, * btn_reverse, * btn_frame_save, * btn_frame_record, * btn_save_plot, * btn_plot_menu, * btn_zoom, *btn_calculate_radiance,
		* btn_workspace_load, * btn_workspace_save, * btn_undo_step, * btn_popout_video, * btn_popout_histogram, * btn_popout_engineering, * btn_bad_pixel_identification,

        * btn_import_tracks, * btn_create_track, * btn_finish_create_track, *btn_change_workspace_directory, *btn_center_on_osm, *btn_center_on_manual;

	QCheckBox * chk_auto_lift_gain, * chk_relative_histogram, * chk_plot_primary_data, * chk_plot_show_line, * chk_plot_full_data, * chk_hide_shadow, * chk_FNS_external_file;
	QGroupBox * grpbox_auto_lift_gain, *grpbox_image_controls, *grpbox_colormap, *grpbox_overlay_controls, *grpbox_bad_pixels_correction, *grpbox_image_processing, *grpbox_FNS_processing, *grpbox_ANS_processing, *grpbox_Image_Shift;

	QComboBox* cmb_deinterlace_options, * cmb_plot_yaxis, * cmb_plot_xaxis, *cmb_color_maps, * cmb_workspace_name, * cmb_processing_states, * cmb_bad_pixels_type, * cmb_outlier_processing_type, *cmb_outlier_processing_sensitivity, *cmb_bad_pixel_color, *cmb_shadow_threshold;
    QComboBox * cmb_OSM_track_IDs, * cmb_manual_track_IDs;
	QFrame* frame_video_player, *frame_histogram_rel, *frame_histogram_abs;
	QFrame* frame_plots;
	QRadioButton* rad_decimal, * rad_linear, * rad_scientific, * rad_log;
	QButtonGroup *data_plot_yformat, *data_plot_yloglinear;

	QCheckBox* chk_show_tracks, *chk_sensor_track_data, *chk_show_time, *chk_highlight_bad_pixels;
	QComboBox* cmb_text_color, *cmb_tracker_color, *cmb_primary_tracker_color;
	QPushButton* btn_change_banner_text, * btn_add_annotations;

	/* --------------------------------------------------------------------------------------------
	----------------------------------------------------------------------------------------------- */

	VideoDisplay *video_display;
	EngineeringPlots *data_plots;
	EngineeringData *eng_data;
	TrackInformation *track_info;
	TrackManagementWidget *tm_widget;
	bool record_video;

    void SetupUi();
    QWidget* SetupFileImportTab();
    QWidget* SetupColorCorrectionTab();
    QWidget* SetupWorkspaceTab();
    QWidget* SetupProcessingTab();
    void SetupVideoFrame();
    void SetupPlotFrame();
    void setupConnections();

    void ToggleVideoPlaybackOptions(bool input);
    bool VerifyFrameSelection(int min_frame, int max_frame);
    void UpdateEpochString(QString new_epoch_string);
    void DisplayOriginalEpoch(QString new_epoch_string);
    QString CreateEpochString(std::vector<double> new_epoch);

	signals:
        void changeBanner(QString banner_text);
        void changeBannerColor(QString color);
        void changeTrackerColor(QString color);
        void directorySelected(QString directory);

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
        void ExecuteNoiseSuppression();
        void ExecuteDeinterlace();
        void ExecuteCenterOnOSM();
        void ExecuteCenterOnManual();
        void ExecuteNonUniformityCorrectionSelectionOption();

        void StartStopVideoRecording();
        void HandleZoomOnVideoToggle();
        void HandleCalculationOnVideoToggle();
        void ClearZoomAndCalculationButtons();

        void UpdateFps();
        void ResetColorCorrection();

        void HandlePlotFullDataToggle();
        void HandlePlotPrimaryOnlyToggle();
        void HandlePlotCurrentFrameMarkerToggle();
   
        void ShowCalibrationDialog();

        void SetDataTimingOffset();
        void ChangeWorkspaceDirectory();
        void CloseWindow();

        void SavePlot();
        void SaveFrame();
        void CopyOsmDirectory();
        void HandleRelativeHistogramToggle(bool input);
        void ApplyEpochTime();
        void ApplyFixedNoiseSuppressionFromExternalFile();
        void ReceiveNewBadPixels(std::vector<unsigned int> new_pixels);
        void ReceiveNewGoodPixels(std::vector<unsigned int> pixels);

        void HandleFrameChange();
        void HandleOsmTracksToggle();
        void HandleNewProcessingState(QString state_name, int index);
        void HandleProcessingStateRemoval(ProcessingMethod method, int index);
        void HandlePopoutVideoClosed();
        void HandlePopoutHistogramClosed();
        void HandlePopoutEngineeringClosed();

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
    void UpdatePlots();
    void AnnotateVideo();

    void HandlePopoutVideoClick(bool checked);
    void OpenPopoutVideoDisplay();

    void HandlePopoutHistogramClick(bool checked);
    void OpenPopoutHistogramPlot();

    void HandlePopoutEngineeringClick(bool checked);
    void OpenPopoutEngineeringPlot();

    void ResizeUi();

	QMenu *menu_file, *menu_settings;
    QAction *action_close, *action_set_timing_offset, *action_change_workspace_directory;

    int GetCurrentColorIndex(QVector<QString> colors, QColor input_color);
    int ConvertFrameNumberTextToInt(QString input);
	CalibrationData calibration_model;

    void LoadOsmData();
    void LoadAbirData(int start_frame, int end_frame);

    void HandleBadPixelReplacement();
    void ReplaceBadPixels(std::vector<unsigned int> & pixels_to_replace);

    void ApplyFixedNoiseCorrection(int start_frame, int num_frames, QString hide_shadow_choice);
    void ApplyAdaptiveNoiseCorrection(int relative_start_frame, int num_frames, QString hide_shadow_choice, int shadow_sigma_thresh);
    void ApplyDeinterlacing(DeinterlaceType deinterlace_method_type);
    void CenterOnOSM(int track_id, std::vector<std::vector<int>> & OSM_centered_offsets, int processing_state_idx);
    void CenterOnManual(int track_id, std::vector<std::vector<int>> & manual_centered_offsets, int processing_state_idx);
    void ApplyFixedNoiseSuppression(QString image_path, QString file_path, unsigned int min_frame, unsigned int max_frame);

    void EnableEngineeringPlotOptions();
    void ExitTrackCreationMode();
    void HandleCreateTrackClick();
    void HandleFinishCreateTrackClick();
    void PrepareForTrackCreation(int track_id);

    void HandleFrameNumberChange(unsigned int new_frame_number);

    void UpdateGlobalFrameVector();
};
