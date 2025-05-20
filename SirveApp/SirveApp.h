#ifndef SIRVEAPP_H
#define SIRVEAPP_H

#include <QtWidgets/QMainWindow>

#include "enums.h"
#include "constants.h"
#include "abir_reader.h"

#include "classification.h"
#include "color_map.h"

#include "osm_reader.h"
#include "video_display.h"
#include "video_player.h"
#include "histogram_plotter.h"
#include "engineering_data.h"
#include "plot_engineering_data.h"

#include "plot_palette.h"
//#include "video_container.h"
//#include "video_details.h"

#include "process_file.h"
#include "annotation_list_dialog.h"
#include "custom_input_dialog.h"
#include "calibration_data.h"
#include "non_uniformity_correction_external_file.h"
#include "config.h"
#include "workspace.h"
#include "data_structures.h"
#include "popout_dialog.h"
#include "tracks.h"
#include "track_management_widget.h"
#include "image_processing.h"
#include "auto_tracking.h"

#include <QVBoxLayout>
#include <QRadioButton>
#include <QGroupBox>
#include <QMenu>
#include <QProgressBar>

class SirveApp : public QMainWindow
{
    Q_OBJECT

public:

    SirveApp(QWidget *parent = Q_NULLPTR);

    /* --------------------------------------------------------------------------------------------
        Key component & supporting status variables
    ----------------------------------------------------------------------------------------------- */
	ABIRFrames::Ptr abir_frames;
    QString abpimage_file_base_name;
    AbpFileMetadata abp_file_metadata;
    QPointer<AutoTracking> auto_tracking;
    std::vector<Classification> classification_list;
    //EngineeringPlots *data_plots;
    QPointer<EngineeringData> eng_data;
    QPointer<QWidget> main_widget;
    bool osmDataLoaded;
    std::vector<Frame> osm_frames;
    OSMReader osm_reader;
    bool record_video;
    QSize screenResolution;
    QPointer<TrackManagementWidget> tm_widget;
    TrackInformation *track_info = nullptr;
    Workspace *workspace = nullptr;
    bool yAxisChanged = false;
    ConfigValues config_values;

    const int leftWidgetStartingSize = 600;
    const int centralWidgetStartingSize = 690;
    const int rightWidgetStartingSize = 600;

    //QWidget *main_widget;
    QGridLayout *engineering_plot_layout;

    /* --------------------------------------------------------------------------------------------
    Qt Elements for user interface
    ----------------------------------------------------------------------------------------------- */

    QString orange_styleSheet = "color: black; background-color: rgba(245, 200, 125, 255); font-weight: bold;";
    QString bold_large_styleSheet = "color: black; font-weight: bold; font-size: 12px";
    QString orange_button_styleSheet = "color: black; background-color: #fbb31a; font-weight: bold;";

/*-----------------------------------------------------------------------------
	----------------------------------------------------------------------------------------------- */

    PlotPalette *plot_palette;

    QPointer<QTabWidget> tab_menu, tab_plots;
    QPointer<QDateTimeEdit> dt_epoch;
    QPointer<QLabel>  lbl_file_name, lbl_lift_value, lbl_gain_value, lbl_max_frames, lbl_current_epoch,
        lbl_adaptive_noise_suppression, lbl_bad_pixel_color, lbl_current_workspace_folder;

    QPointer<QLabel> lbl_adaptive_noise_suppression_status, lbl_fixed_suppression, lbl_bad_pixel_count,
                     lbl_create_track_message;
    QPointer<QLabel> lbl_min_count_val, lbl_max_count_val, label_lift, label_gain;
    QPointer<QLabel> lbl_progress_status, lbl_processing_description, lbl_min_scale_value, lbl_max_scale_value;
    QPointer<QScrollArea> scrollarea_processing_description;
    QPointer<QLineEdit> txt_lift_sigma, txt_gain_sigma, txt_frame_stack_Nframes, txt_accumulator_offset;
    QPointer<QSlider> slider_lift, slider_gain;

    QPointer<QLineEdit> txt_pixel_buffer, txt_start_frame, txt_stop_frame, txt_moving_median_N,
        txt_bad_pixel_start_frame, txt_bad_pixel_stop_frame, txt_ANS_number_frames, txt_ANS_offset_frames,
        txt_FNS_start_frame,  txt_FNS_stop_frame;
    QPointer<QPushButton> btn_get_frames, btn_load_osm, btn_copy_directory, btn_apply_epoch, btn_reset_color_correction,
        btn_ANS, btn_FNS, btn_calibration_dialog, btn_deinterlace, btn_deinterlace_current_frame, btn_video_menu,
        btn_save_plot, btn_plot_menu,  btn_workspace_load, btn_workspace_save, btn_undo_step, btn_popout_histogram,
        btn_popout_engineering, btn_replace_bad_pixels, btn_import_tracks, btn_create_track, btn_finish_create_track,
        btn_center_on_tracks, btn_center_on_brightest, btn_frame_stack, btn_RPCP, btn_cancel_operation,
        btn_auto_track_target;

    QPointer<QCheckBox>  chk_auto_lift_gain, chk_relative_histogram, chk_plot_primary_data, chk_plot_show_line,
        chk_plot_full_data, chk_hide_shadow, chk_FNS_external_file;
    QPointer<QGroupBox>  grpbox_auto_lift_gain, grpbox_image_controls, grpbox_colormap, grpbox_overlay_controls,
        grpbox_bad_pixels_correction, grpbox_FNS_processing, grpbox_ANS_processing, grpbox_image_shift,
        grpbox_status_area, grpbox_image_processing;
    QPointer<QGroupBox> grpbox_load_frames_area, grpbox_progressbar_area, plot_groupbox;
    QPointer<QProgressBar>  progress_bar_main;

    QPointer<QComboBox> cmb_deinterlace_options, cmb_plot_yaxis, cmb_plot_xaxis, cmb_color_maps, cmb_processing_states,
        cmb_bad_pixels_type, cmb_outlier_processing_type, cmb_outlier_processing_sensitivity, cmb_bad_pixel_color,
        cmb_shadow_threshold;
    QPointer<QComboBox>  cmb_OSM_track_IDs, cmb_manual_track_IDs, cmb_track_centering_priority;
    QPointer<QFrame> frame_video_player, frame_histogram_rel, frame_histogram_abs;
    QPointer<QFrame> frame_plots;
    QPointer<QRadioButton> rad_scientific, rad_log, rad_scale_by_frame, rad_scale_by_cube,
        rad_autotrack_filter_none, rad_autotrack_filter_gaussian, rad_autotrack_filter_median,
        rad_autotrack_filter_nlmeans;
    QPointer<QButtonGroup> data_plot_yformat, data_plot_yloglinear;
    QPointer<QRadioButton>  rad_autotrack_feature_weighted_centroid,  rad_autotrack_feature_centroid,
        rad_autotrack_feature_peak;
    QPointer<QCheckBox> chk_show_OSM_tracks, chk_sensor_track_data, chk_show_time, chk_highlight_bad_pixels,
        chk_deinterlace_confirmation;
    QPointer<QComboBox> cmb_text_color, cmb_OSM_track_color, cmb_primary_tracker_color, cmb_autotrack_threshold;
    QPointer<QPushButton> btn_change_banner_text, btn_add_annotations, btn_delete_state, btn_accumulator;

    QPointer<QMenu> menu, plot_menu;
    QPointer<QAction> menu_add_banner, menu_add_primary_data, menu_sensor_boresight, menu_osm,
        menu_change_color_tracker, menu_change_color_banner, menu_change_color_map, menu_annotate;
    QPointer<QAction> menu_plot_all_data, menu_plot_primary, menu_plot_frame_marker, menu_plot_edit_banner,
        action_show_calibration_dialog, action_enable_binary_export;

    QPointer<QStackedWidget> stck_noise_suppresssion_methods;
    QPointer<AnnotationListDialog> annotation_dialog;

    QPointer<QStatusBar> status_bar;
    QPointer<QLabel> lbl_status_start_frame, lbl_status_stop_frame, lbl_loaded_frames,
        lbl_workspace_name, lbl_workspace_name_field, lbl_current_workspace_folder_field;
    QPointer<QCheckBox> chk_bad_pixels_from_original;
    QPointer<QLineEdit> txt_auto_track_start_frame, txt_auto_track_stop_frame, txt_accumulator_weight;

    QPointer<QComboBox> cursor_color;
    QPointer<QSplitter> splitter;

    // this was put here in order to tweak its icon when disabling deinterlacing on D:
    QToolBox *toolbox_image_processing;

    /* --------------------------------------------------------------------------------------------
        Setup & Basic Frontend Operations
    ----------------------------------------------------------------------------------------------- */

    static QString CreateEpochString(const std::vector<double>& new_epoch);
    void    DisplayOriginalEpoch(const QString& new_epoch_string);

    static SirveApp* GetMainWindow();
    const QVector<QRgb>& GetStartingColorTable() const;
    StateManager& GetStateManager();
    const StateManager& GetStateManager() const;
    QPoint  GetWindowPosition() const;
    QSize   GetWindowSize() const;

    QWidget*    SetupColorCorrectionTab();
    void        SetupConnections();
    void        SetupPlotFrame();
    QWidget*    SetupProcessingTab();
    QWidget*    SetupTracksTab();
    void        SetupUi();
    void        SetupVideoFrame();

    void UpdateEpochString(const QString& new_epoch_string);
    void UpdateGuiPostDataLoad(bool status);
    void UpdateGuiPostFrameRangeLoad(bool status);
    void UpdatePlots(EngineeringPlot *engineering_plot);
    bool VerifyFrameSelection(int min_frame, int max_frame) const;
    void UpdateGlobalFrameVector();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    ABPFileType abp_file_type = ABPFileType::ABP_B;

    QPointer<StateManager> state_manager_;
    QPointer<VideoPlayer> video_player_;

    ColorMap video_colors;
    QPointer<ColorMapDisplay> color_map_display;

    QPointer<QVBoxLayout> histogram_abs_layout, vlayout_tab_histogram;
    QPointer<QClipboard> clipboard;

    QPointer<ProcessFile> file_processor;

    bool in_edit_mode = false;

    QPointer<PopoutDialog> popout_histogram;
    QPointer<PopoutDialog> popout_engineering;

    QPointer<HistogramLinePlot> histogram_plot;

    int currently_editing_or_creating_track_id;

    QPointer<QMenu> file_menu, menu_workspace, menu_export, menu_settings, menu_about;
    QPointer<QAction> action_about, action_close, action_set_timing_offset, action_change_workspace_directory,
         action_load_OSM, action_load_OSM_D, action_load_frames;
    QPointer<QAction> action_load_workspace, action_save_workspace, action_export_current_frame,
        action_export_frame_range, action_export_tracking_data;
    QPointer<QAction> action_export_all_frames;

    CalibrationData calibration_model;

    void AllocateAbirData(int min_frame, int max_frame);
    void AnnotateVideo();

    void ApplyAccumulatorNoiseSuppression(double weight, int offset, bool hide_shadow_choice, int shadow_sigma_thresh, int source_state_idx);
    void ApplyAdaptiveNoiseSuppression(int relative_start_frame, int num_frames, int processing_state_idx);
    void ApplyDeinterlacing(int processing_state_idx);
    void ApplyDeinterlacingCurrent();
    void ApplyFixedNoiseSuppression(const QString& image_path, const QString& file_path, unsigned int frame0, unsigned int min_frame, unsigned int max_frame, int processing_state_idx);
    void ApplyRPCPNoiseSuppression(int processing_state_idx);

    void CenterOnBrightest(std::vector<std::vector<int>> & brightest_centered_offsets, int processing_state_idx);
    void CenterOnOffsets(const QString& trackFeaturePriority, int track_id, const std::vector<std::vector<int>> & track_centered_offsets, boolean find_any_tracks, int source_state_idx);
    void CenterOnTracks(const QString& trackFeaturePriority, int OSM_track_id, int manual_track_id, std::vector<std::vector<int>> & track_centered_offsets,boolean findAnyTrack, int processing_state_idx);

    bool CheckCurrentStateisNoiseSuppressed(int source_state_idx) const;
    void CloseProgressArea();

    static int ConvertFrameNumberTextToInt(const QString& input);
    void CreateMenuActions();
    ImageProcessing* CreateImageProcessor();
    void DeleteAbirData();
    void DeleteState();

    void EditBadPixelColor();
    void EditBannerColor();
    void EditBannerText();
    void EditCursorColor();
    void EditClassificationText(int plot_tab_index, QString current_value);
    void EditColorMap();
    void EditOSMTrackColor();
    void EditPlotText();

    void EnableBinaryExport();

    void ExitTrackCreationMode();

    void ExportAllFrames();
    void ExportFrame();
    void ExportFrameRange();
    void ExportPlotData();

    void FrameStacking(int num_frames, int processing_state_idx);

    static void GetAboutTimeStamp();
    static double GetAvailableMemoryRatio(int num_frames, ABPFileType file_type);
    static int GetCurrentColorIndex(const QVector<QString>& colors, const QColor& input_color);
    static std::vector<unsigned int> GetUniqueIntegerVector(std::vector<unsigned int> A);

    void HandleAutoTrackStartChangeInput();
    void HandleBadPixelRawToggle();
    void HandleBadPixelReplacement();
    void HandleCreateTrackClick();
    void HandleExternalFileToggle();
    void HandleFinishCreateTrackClick();
    void HandleOutlierProcessingChange();
    //void HandlePopoutEngineeringClick(bool checked);
    void HandlePopoutHistogramClick(bool checked);
    void HandleYAxisChange();

    void LoadAbirData(int start_frame, int stop_frame);
    void LoadOsmData();
;
    void OpenPopoutHistogramPlot();
    void OpenProgressArea(const QString& message, int N);
    void PrepareForTrackCreation(int track_id);
    static void ProvideInformationAbout();
    void ReplaceBadPixels(std::vector<unsigned int> pixels_to_replace, int source_state_ind);
    void ResetEngineeringDataAndSliderGUIs();

signals:
    void changeBanner(QString banner_text);
    void changeBannerColor(QString color);
    void changeTrackerColor(QString color);
    void directorySelected(QString directory);
    void itemDataSelected(QVariant data);
    void updateVideoDisplayPinpointControls(bool status);

public slots:

    void ApplyEpochTime();
    void ApplyFixedNoiseSuppressionFromExternalFile();
    void ChangeWorkspaceDirectory();
    void CloseWindow();

    void ColorTrack(int track_id, const QColor& initial_color);

    void ExecuteAccumulatorNoiseSuppression();
    void ExecuteAdaptiveNoiseSuppression();
    void ExecuteAutoTracking();
    void ExecuteCenterOnBrightest();
    void ExecuteCenterOnTracks();
    void ExecuteDeinterlace();
    void ExecuteDeinterlaceCurrent();
    void ExecuteFixedNoiseSuppression();
    void ExecuteFrameStacking();
    void ExecuteRPCPNoiseSuppression();
    // void HandleAbpFileSelected();
    // void HandleAbpDFileSelected();
    void HandleAbpFileSelected();
    void HandleAnnotationDialogClosed();
    void HandleAutoLiftGainCheck(int state);
    void HandleFrameChange();
    void HandleGainSliderToggled();
    void HandleHideManualTrackId(int track_id);
    void HandleHistogramClick(double x0, double x1);
    void HandleLiftSliderToggled();
    void HandleManualTrackRecoloring(int track_id, const QColor& color);
    void HandleNewProcessingState(const QString& state_name, const QString& combobox_state_name, int index);
    void HandleOsmTracksToggle();

    void HandleParamsSelected(QString plotTitle, const std::vector<Quantity> &quantities);
    void HandlePlayerStateChanged(bool status);
    void HandlePlotFocusChanged(int tab_index);
    void HandlePlotFullDataToggle();
    void HandlePlotPrimaryOnlyToggle();
    //void HandlePopoutEngineeringClosed();

    void HandlePopoutHistogramClosed();
    void HandleProcessingNewStateSelected();
    void HandleProcessingStateRemoval(ProcessingMethod method, int index);
    void HandleProcessingStatesCleared();
    void HandleProgressUpdate(int percent);
    void HandleRelativeHistogramToggle(bool input);
    void HandleShowManualTrackId(int track_id, const QColor& color);
    void HandleTrackRemoval(int track_id);
    void HandleZoomAfterSlider();

    void ImportTracks();
    void LoadWorkspace();

    void OpenPopoutEngineeringPlot(int tab_index, QString plotTitle, std::vector<Quantity> quantities);
    void ClosePopoutEngineeringPlot();

    void ReceiveNewBadPixels(const std::vector<unsigned int>& new_pixels);
    void ReceiveNewGoodPixels(const std::vector<unsigned int>& pixels);
    void ReceiveProgressBarUpdate(int percent) const;
    void ResetColorCorrection();

    void SaveWorkspace();

    void SetDataTimingOffset();
    void SetLiftAndGain(double lift, double gain);
    void ShowCalibrationDialog();

    void UiLoadAbirData();
    bool ValidateAbpFiles(const QString& path_to_image_file);

    void onThresholdComboBoxIndexChanged(int index) {
        QVariant data = cmb_autotrack_threshold->itemData(index);
        emit itemDataSelected(data); // Emit signal to pass data to another class
    }
};

#endif // SIRVEAPP_H
