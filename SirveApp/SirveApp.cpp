#include "SirveApp.h"

SirveApp::SirveApp(QWidget *parent)
    : QMainWindow(parent)
{
    config_values = configReaderWriter::ExtractWorkspaceConfigValues();

    workspace = new Workspace(config_values.workspace_folder);

    // establish object that will hold video and connect it to the playback thread
    color_map_display = new ColorMapDisplay(video_colors.maps[0].colors, 0, 1);
    video_display = new VideoDisplay(video_colors.maps[0].colors);
    video_display->moveToThread(&thread_video);

    histogram_plot = new HistogramLinePlot();

    SetupUi();
    popout_video = new PopoutDialog();
    popout_histogram = new PopoutDialog();
    popout_engineering = new PopoutDialog();

    //---------------------------------------------------------------------------

    // establish object to control playback timer and move to a new thread
    playback_controller = new FramePlayer(1);
    playback_controller->moveToThread(&thread_timer);


    //---------------------------------------------------------------------------
    // setup container to store all videos
    eng_data = NULL;

    // default recording video to false
    record_video = false;

    // links chart with frame where it will be contained
    QVBoxLayout *histogram_rel_layout = new QVBoxLayout();
    histogram_rel_layout->addWidget(histogram_plot->rel_chart_view);
    frame_histogram_rel->setLayout(histogram_rel_layout);

    // links chart with frame where it will be contained
    histogram_abs_layout = new QVBoxLayout();
    histogram_abs_layout->addWidget(histogram_plot->abs_chart_view);
    frame_histogram_abs->setLayout(histogram_abs_layout);

    // establish connections to all qwidgets
    setupConnections();

    HandleRelativeHistogramToggle(false);
    ToggleVideoPlaybackOptions(false);
    EnableEngineeringPlotOptions();

    CreateMenuActions();

    this->resize(0, 0);
}

SirveApp::~SirveApp() {
    delete video_display;
    delete playback_controller;
    delete eng_data;
    delete data_plots;
    thread_video.terminate();
    thread_timer.terminate();
}

void SirveApp::SetupUi() {

    QGridLayout* main_layout = new QGridLayout();

    // Define main widgets in UI
    tab_menu = new QTabWidget();
    frame_video_player = new QFrame();
    tab_plots = new QTabWidget();

    tab_menu->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    // ------------------------------------------------------------------------
    // Define complete tab widget
    // ------------------------------------------------------------------------
    tab_menu->addTab(SetupProcessingTab(), "Processing");
    tab_menu->addTab(SetupColorCorrectionTab(), "Color/Overlays");
    tab_menu->addTab(SetupTracksTab(), "Tracks");

    QSizePolicy fixed_width;
    fixed_width.setHorizontalPolicy(QSizePolicy::Minimum);
    fixed_width.setVerticalPolicy(QSizePolicy::Preferred);
    tab_menu->setSizePolicy(fixed_width);

    QSizePolicy fixed_width_video;
    fixed_width_video.setHorizontalPolicy(QSizePolicy::Fixed);
    fixed_width_video.setVerticalPolicy(QSizePolicy::Fixed);
    frame_video_player->setSizePolicy(fixed_width_video);
    frame_video_player->setFixedHeight(750);
    frame_video_player->setFixedWidth(800);

    // ------------------------------------------------------------------------
    // Adds all elements to main UI

    SetupVideoFrame();
    SetupPlotFrame();

    QGroupBox *grpbox_status_area = new QGroupBox();
    // grpbox_status_area->setStyleSheet("background-color: rgba(245, 200, 125, 100);");
    QGridLayout *grid_status_area = new QGridLayout();
    grpbox_status_area->setLayout(grid_status_area);
    cmb_processing_states = new QComboBox();
    btn_undo_step = new QPushButton("Undo One Step");
    btn_undo_step->setFixedWidth(110);
    QLabel *lbl_processing_state = new QLabel("Processing State:");
    lbl_processing_state->setFixedWidth(110);
    lbl_processing_description = new QLabel("");
    lbl_processing_description->setFixedHeight(50);
    lbl_processing_description->setWordWrap(true);
    lbl_processing_description->setStyleSheet("border: 1px solid gray; border-color: rgb(245, 200, 125); border-width: 1px;");
    lbl_file_name = new QLabel("File Name:");
    txt_file_name = new QLineEdit("");
    txt_file_name->setReadOnly(true);
    lbl_max_frames = new QLabel("Max Frames: ");
    txt_max_frames = new QLineEdit("");
    txt_max_frames->setReadOnly(true);
    QLabel* label_start_frame = new QLabel("Start Frame:");
    QLabel* label_stop_frame = new QLabel("Stop Frame:");
    txt_start_frame = new QLineEdit("0");
    txt_start_frame->setAlignment(Qt::AlignHCenter);
    txt_end_frame = new QLineEdit();
    txt_end_frame->setAlignment(Qt::AlignHCenter);
    btn_get_frames = new QPushButton("Load Frames");
    lbl_current_workspace_folder = new QLabel("Workspace Folder: ");
    lbl_current_workspace_folder->setWordWrap(false);
    txt_current_workspace_folder = new QLineEdit();
    txt_current_workspace_folder->setReadOnly(true);
    txt_current_workspace_folder->setText(config_values.workspace_folder);
    QLabel *lbl_workspace_name = new QLabel("Workspace:");
    txt_workspace_name = new QLineEdit("");
    txt_workspace_name->setReadOnly(true);
    // grid_status_area->addWidget(lbl_processing_state,0,0,1,1);
    // grid_status_area->addWidget(cmb_processing_states,0,1,1,7);
    // grid_status_area->addWidget(btn_undo_step,0,8,1,1);
    grid_status_area->addWidget(lbl_processing_description,1,0,1,-1);
    grid_status_area->addWidget(lbl_file_name,2,0,1,1);
    grid_status_area->addWidget(txt_file_name,2,1,1,-1);
    grid_status_area->addWidget(lbl_current_workspace_folder,3,0,1,1);
    grid_status_area->addWidget(txt_current_workspace_folder,3,1,1,-1);
    grid_status_area->addWidget(lbl_workspace_name,4,0,1,-1);
    grid_status_area->addWidget(txt_workspace_name,4,1,1,-1);
    grid_status_area->addWidget(lbl_max_frames,5,0,1,1);
    grid_status_area->addWidget(txt_max_frames,5,1,1,1);
    grid_status_area->addWidget(label_start_frame,5,2,1,1);
    grid_status_area->addWidget(txt_start_frame,5,4,1,1);
    grid_status_area->addWidget(label_stop_frame,5,5,1,1);
    grid_status_area->addWidget(txt_end_frame,5,6,1,1);
    grid_status_area->addWidget(btn_get_frames,5,8,1,1);
    grpbox_status_area->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    // grpbox_status_area->setStyleSheet("background-color:rgb(247,246,246); border-width: 2px; border-color: rgb(0, 0, 125); border-radius: 6px;");
    // grpbox_status_area->setStyleSheet("border: 0px solid gray; border-color: rgb(245, 200, 125); border-width: 0px;");

    QGroupBox *grpbox_state_control_area = new QGroupBox();
    grpbox_state_control_area->setStyleSheet("border: 0px solid gray; border-color: rgb(245, 200, 125); border-width: 0px;");
    QGridLayout *grid_state_control_area = new QGridLayout();
    grpbox_state_control_area->setLayout(grid_state_control_area);
    grid_state_control_area->addWidget(lbl_processing_state,0,0,1,1);
    grid_state_control_area->addWidget(cmb_processing_states,0,1,1,7);
    grid_state_control_area->addWidget(btn_undo_step,0,8,1,1);

    QGroupBox *grpbox_progressbar_area = new QGroupBox();
    QGridLayout *grid_progressbar_area = new QGridLayout();
    grpbox_progressbar_area->setLayout(grid_progressbar_area);
    grpbox_progressbar_area->setStyleSheet("border: 0px solid gray; border-color: rgb(245, 200, 125); border-width: 0px;");
    lbl_progress_status = new QLabel(" ");
    // lbl_progress_status->setStyleSheet("border: 1px solid gray; border-color: rgb(245, 200, 125); border-width: 1px;");
    progress_bar_main = new QProgressBar();
    btn_cancel_operation = new QPushButton("Cancel");
    btn_cancel_operation->setFixedWidth(75);
    grid_progressbar_area->addWidget(lbl_progress_status,0,0,1,3);
    grid_progressbar_area->addWidget(progress_bar_main,0,3,1,8);
    grid_progressbar_area->addWidget(btn_cancel_operation,0,11,1,1);

    main_layout->addWidget(tab_menu,0,0,1,1);
    main_layout->addWidget(frame_video_player,0,1,2,1);
    main_layout->addWidget(tab_plots,0,2,2,1);
    main_layout->addWidget(grpbox_progressbar_area,2,1,1,2);
    main_layout->addWidget(grpbox_state_control_area,2,0,1,1);
    main_layout->addWidget(grpbox_status_area,1,0,1,1);
    QFrame* frame_main = new QFrame();
    frame_main->setLayout(main_layout);

    // ------------------------------------------------------------------------
    // initialize ui elements

    tab_menu->setCurrentIndex(0);
    tab_menu->setTabEnabled(0, false);
    tab_menu->setTabEnabled(1, false);
    tab_menu->setTabEnabled(2, false);

    txt_start_frame->setEnabled(false);
    txt_end_frame->setEnabled(false);
    btn_get_frames->setEnabled(false);

    dt_epoch->setEnabled(false);
    btn_apply_epoch->setEnabled(false);

    rad_decimal->setChecked(true);
    rad_linear->setChecked(true);

    grpbox_auto_lift_gain->setEnabled(false);

    cmb_processing_states->setEnabled(false);

    btn_import_tracks->setEnabled(false);
    btn_create_track->setEnabled(false);
    chk_auto_lift_gain->setChecked(true);
    btn_reset_color_correction->setEnabled(false);
    grpbox_auto_lift_gain->setEnabled(true);
    // ------------------------------------------------------------------------

    this->setCentralWidget(frame_main);
    this->show();
}

QWidget* SirveApp::SetupColorCorrectionTab()
{

    color_map_display->setMinimumHeight(20);
    // color_map_display->setMaximumWidth(500);
    // color_map_display->setMinimumWidth(500);
    QWidget* widget_tab_color = new QWidget(tab_menu);
    QVBoxLayout* vlayout_tab_color = new QVBoxLayout(widget_tab_color);

    grpbox_image_controls = new QGroupBox("Image Controls");
    grpbox_image_controls->setStyleSheet(bold_large_styleSheet);
    grpbox_image_controls->setFixedWidth(700);
    QGridLayout* grid_grpbox_image_controls = new QGridLayout(grpbox_image_controls);

    label_lift = new QLabel("Dark \nSet Point");
    label_lift->setToolTip("Dark Set Point pushes the image darker");
    label_gain = new QLabel("Light \nSet Point");
    label_gain->setToolTip("Light Set Point pushes the image lighter");
    lbl_lift_value = new QLabel("0.0");
    lbl_lift_value->setFixedWidth(50);
    lbl_gain_value = new QLabel("1.0");
    lbl_gain_value->setFixedWidth(50);
    lbl_gain_value->setFixedWidth(50);
    slider_lift = new QSlider();
    slider_lift->setOrientation(Qt::Horizontal);
    slider_lift->setMinimum(0);
    slider_lift->setMaximum(1000);
    slider_lift->setSingleStep(1);
    slider_lift->setPageStep(10);
    slider_lift->setValue(0);
    slider_lift->setTickPosition(QSlider::TicksAbove);
    slider_lift->setTickInterval(100);
    slider_lift->setEnabled(false);

    slider_gain = new QSlider();
    slider_gain->setOrientation(Qt::Horizontal);
    slider_gain->setMinimum(0);
    slider_gain->setMaximum(1000);
    slider_gain->setSingleStep(1);
    slider_gain->setPageStep(10);
    slider_gain->setValue(1000);
    lbl_max_scale_value = new QLabel("High");
    lbl_max_scale_value->setStyleSheet("color:rgb(81,72,65);");
    QLabel* lbl_colormap = new QLabel("Set Colormap:");
    slider_gain->setTickPosition(QSlider::TicksAbove);
    slider_gain->setTickInterval(100);
    slider_gain->setEnabled(false);
    chk_auto_lift_gain = new QCheckBox("Auto\nLift/Gain");
    // chk_relative_histogram = new QCheckBox("Relative Histogram");
    btn_reset_color_correction = new QPushButton("Reset Set Points");
    lbl_min_count_val = new QLabel("Light Set Pt:");
    lbl_min_count_val->setStyleSheet("color: black; background-color: rgba(245, 200, 125, 255); font-weight: bold;");
    lbl_min_count_val->setFixedWidth(130);
    lbl_min_count_val->setFixedHeight(30);
    lbl_max_count_val = new QLabel("Light Set Pt:");
    lbl_max_count_val->setStyleSheet("color: black; background-color: rgba(245, 200, 125, 255); font-weight: bold;");
    lbl_max_count_val->setFixedHeight(30);
    lbl_min_scale_value = new QLabel("Low");
    lbl_min_scale_value->setFixedWidth(125);
    lbl_min_scale_value->setStyleSheet("color:rgb(81,72,65);");;
    cmb_color_maps = new QComboBox();
    int number_maps = video_colors.maps.size();
    for (int i = 0; i < number_maps; i++)
        cmb_color_maps->addItem(video_colors.maps[i].name);
    QGroupBox *grpbox_scale_options = new QGroupBox();
    QGridLayout *grid_grpbox_scale_options = new QGridLayout();
    grpbox_scale_options->setLayout(grid_grpbox_scale_options);
    grpbox_scale_options->setStyleSheet("border: 0px solid gray; border-width: 0px;");
    rad_scale_by_frame = new QRadioButton("Scale by frame maximum");
    rad_scale_by_frame->setAutoExclusive(true);
    rad_scale_by_frame->setChecked(true);
    rad_scale_by_cube = new QRadioButton("Scale by cube maximum");
    rad_scale_by_cube->setAutoExclusive(true);
    rad_scale_by_cube->setChecked(false);
    connect(rad_scale_by_frame, &QCheckBox::toggled, this, &SirveApp::UpdateGlobalFrameVector);
    connect(rad_scale_by_cube, &QCheckBox::toggled, this, &SirveApp::UpdateGlobalFrameVector);
    grid_grpbox_scale_options->addWidget(rad_scale_by_frame,0,0,1,1);
    grid_grpbox_scale_options->addWidget(rad_scale_by_cube,0,1,1,1);

    grpbox_auto_lift_gain = new QGroupBox("Auto Lift/Gain Options");
    QGridLayout* grid_grpbox_lift_controls = new QGridLayout(grpbox_auto_lift_gain);
    QDoubleValidator* ensure_double = new QDoubleValidator(widget_tab_color);
    QLabel* lbl_auto_lift = new QLabel("Lift (sigma below mean)");
    txt_lift_sigma = new QLineEdit("3");
    txt_lift_sigma->setValidator(ensure_double);
    txt_lift_sigma->setFixedWidth(50);
    QLabel* lbl_auto_gain = new QLabel("Gain (sigma above mean)");
    txt_gain_sigma = new QLineEdit("3");
    txt_gain_sigma->setValidator(ensure_double);
    txt_gain_sigma->setFixedWidth(50);
    grid_grpbox_lift_controls->addWidget(lbl_auto_lift,0,0,1,1);
    grid_grpbox_lift_controls->addWidget(txt_lift_sigma,0,1,1,1);
    grid_grpbox_lift_controls->addWidget(lbl_auto_gain,0,2,1,1);
    grid_grpbox_lift_controls->addWidget(txt_gain_sigma,0,3,1,1);

    grid_grpbox_image_controls->addWidget(chk_auto_lift_gain,0,0, 1, 2);
    grid_grpbox_image_controls->addWidget(grpbox_auto_lift_gain, 0, 1, 1, -1);
    grid_grpbox_image_controls->addWidget(label_lift, 1, 0, 1, 1);
    grid_grpbox_image_controls->addWidget(slider_lift, 1, 1, 1, 4);
    grid_grpbox_image_controls->addWidget(lbl_lift_value, 1, 5, 1, 1);
    grid_grpbox_image_controls->addWidget(label_gain, 2, 0, 1, 1);
    grid_grpbox_image_controls->addWidget(slider_gain, 2, 1, 1, 4);
    grid_grpbox_image_controls->addWidget(lbl_gain_value, 2, 5, 1, 1);
    grid_grpbox_image_controls->addWidget(lbl_colormap, 3, 0, 1, 1);
    grid_grpbox_image_controls->addWidget(cmb_color_maps, 3, 1, 1, 1);
    grid_grpbox_image_controls->addWidget(grpbox_scale_options,3,2,1,4);
    grid_grpbox_image_controls->addWidget(lbl_min_scale_value, 4, 0, 1, 1);
    grid_grpbox_image_controls->addWidget(color_map_display, 4, 1, 1, 4);
    grid_grpbox_image_controls->addWidget(lbl_max_scale_value, 4, 5, 1 ,1);
    grid_grpbox_image_controls->addWidget(btn_reset_color_correction, 5, 2, 1, 1);
    grid_grpbox_image_controls->addWidget(lbl_min_count_val,5, 1, 1 ,1);
    grid_grpbox_image_controls->addWidget(lbl_max_count_val, 5, 4, 1, 1);


    vlayout_tab_color->addWidget(grpbox_image_controls);

    grpbox_overlay_controls = new QGroupBox("Overlay Controls");
    grpbox_overlay_controls->setStyleSheet(bold_large_styleSheet);
    QGridLayout* grid_overlay_controls = new QGridLayout(grpbox_overlay_controls);

    chk_show_tracks = new QCheckBox("Show OSM Tracks");
    chk_show_tracks->setChecked(true);
    video_display->ToggleOsmTracks(true);
    chk_sensor_track_data = new QCheckBox("Show Sensor Info");
    chk_show_time = new QCheckBox("Show Zulu Time");
    btn_change_banner_text = new QPushButton("Change Banner Text");
    btn_add_annotations = new QPushButton("Add/Edit Annotations");
    QStringList colors = ColorScheme::get_track_colors();
    QLabel* lbl_text_color = new QLabel("Set Text Color:");

    cmb_tracker_color = new QComboBox();
    cmb_text_color = new QComboBox();
    cmb_tracker_color->addItems(colors);
    cmb_text_color->addItems(colors);
    cmb_tracker_color->setEnabled(true);

    grid_overlay_controls->addWidget(chk_show_tracks,0,0);
    grid_overlay_controls->addWidget(cmb_tracker_color,0,1);
    grid_overlay_controls->addWidget(lbl_text_color,1,0);
    grid_overlay_controls->addWidget(cmb_text_color,1,1);
    grid_overlay_controls->addWidget(chk_sensor_track_data,0,2);
    grid_overlay_controls->addWidget(chk_show_time,1,2);
    grid_overlay_controls->addWidget(btn_change_banner_text,0,3);
    grid_overlay_controls->addWidget(btn_add_annotations,1,3);

    vlayout_tab_color->addWidget(grpbox_overlay_controls);

    QGroupBox *grpbox_epoch_area = new QGroupBox();
    QGridLayout *grid_epoch_area = new QGridLayout();
    grpbox_epoch_area->setLayout(grid_epoch_area);
    QLabel* label_epoch = new QLabel("Epoch");
    QLabel* label_date_format = new QLabel("Format is:    YYYY/MM/DD HH:MM:SS");
    dt_epoch = new QDateTimeEdit(QDateTime(QDate(2001, 01, 01), QTime(0, 0, 0, 0)));
    dt_epoch->setDisplayFormat("yyyy/MM/dd hh:mm:ss.zzz");
    dt_epoch->setAlignment(Qt::AlignHCenter);
    lbl_current_epoch = new QLabel("Applied Epoch: ");
    btn_apply_epoch = new QPushButton("Apply Epoch");
    grid_epoch_area->addWidget(label_epoch, 0, 0, 1, 1);
    grid_epoch_area->addWidget(dt_epoch, 0, 1, 1, 1);
    grid_epoch_area->addWidget(label_date_format, 1,0,1,2);
    grid_epoch_area->addWidget(lbl_current_epoch, 2,0,1,2);
    grid_epoch_area->addWidget(btn_apply_epoch, 2, 1,1,1);
    vlayout_tab_color->addWidget(grpbox_epoch_area);

    vlayout_tab_color->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout

    return widget_tab_color;
}

QWidget* SirveApp::SetupProcessingTab() {

    QWidget* widget_tab_processing = new QWidget(tab_menu);
    QVBoxLayout* vlayout_tab_processing = new QVBoxLayout(widget_tab_processing);

    QStringList colors = ColorScheme::get_track_colors();
    // ------------------------------------------------------------------------
    grpbox_bad_pixels_correction = new QGroupBox();
    grpbox_bad_pixels_correction->setStyleSheet(bold_large_styleSheet);
    grpbox_bad_pixels_correction->setFixedWidth(700);
    grpbox_bad_pixels_correction->setFixedHeight(200);
    QGridLayout* grid_bad_pixels = new QGridLayout(grpbox_bad_pixels_correction);

    lbl_bad_pixel_count = new QLabel("No Bad Pixels Replaced.");
    lbl_bad_pixel_count->setStyleSheet("background-color: rgb(200,200,200);");
    lbl_bad_pixel_count->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    lbl_bad_pixel_type = new QLabel("Replace Pixels:");
    cmb_bad_pixels_type = new QComboBox();
    cmb_bad_pixels_type->setFixedWidth(150);
    cmb_bad_pixels_type->addItem("All Bad Pixels");
    cmb_bad_pixels_type->addItem("Dead/Bad Scale Only");
    cmb_bad_pixels_type->addItem("Outlier Only");
    lbl_bad_pixel_method = new QLabel("Method:");
    cmb_outlier_processing_type = new QComboBox();
    cmb_outlier_processing_type->setFixedWidth(150);
    cmb_outlier_processing_type->addItem("Median");
    cmb_outlier_processing_type->addItem("Moving Median");
    connect(cmb_outlier_processing_type, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &SirveApp::handle_outlier_processing_change);
    lbl_bad_pixel_start_frame = new QLabel("Sample Start:");
    txt_bad_pixel_start_frame = new QLineEdit("1");
    txt_bad_pixel_start_frame->setFixedWidth(60);
    lbl_bad_pixel_stop_frame = new QLabel("Sample Stop:");
    txt_bad_pixel_end_frame = new QLineEdit("500");
    txt_bad_pixel_end_frame->setFixedWidth(60);
    lbl_moving_median_window_length = new QLabel("Window Length:");
    txt_moving_median_N = new QLineEdit("30");
    txt_moving_median_N->setFixedWidth(60);
    txt_moving_median_N->setEnabled(false);
    lbl_bad_pixel_sensitivity = new QLabel("Sensitivity:");
    cmb_outlier_processing_sensitivity = new QComboBox();
    cmb_outlier_processing_sensitivity->addItem("Low 6 sigma");
    cmb_outlier_processing_sensitivity->addItem("Medium 5 sigma");
    cmb_outlier_processing_sensitivity->addItem("High 4 sigma");
    cmb_outlier_processing_sensitivity->addItem("Max 3 sigma");
    chk_highlight_bad_pixels = new QCheckBox("Highlight Bad Pixels");
    lbl_bad_pixel_color = new QLabel("Color:");
    cmb_bad_pixel_color = new QComboBox();
    cmb_bad_pixel_color->setFixedWidth(100);
    cmb_bad_pixel_color->addItems(colors);
    cmb_bad_pixel_color->setCurrentIndex(2);
    connect(cmb_bad_pixel_color, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &SirveApp::edit_bad_pixel_color);
    btn_bad_pixel_identification = new QPushButton("Replace Bad Pixels");
    connect(btn_bad_pixel_identification, &QPushButton::clicked, this, &SirveApp::HandleBadPixelReplacement);
    QSpacerItem *spacerItem = new QSpacerItem(50,1);

    grid_bad_pixels->addWidget(lbl_bad_pixel_count, 0, 0, 1, -1);
    grid_bad_pixels->addWidget(lbl_bad_pixel_type, 1, 0, 1, 1);
    grid_bad_pixels->addWidget(cmb_bad_pixels_type, 1, 1, 1, 2);
    grid_bad_pixels->addWidget(lbl_bad_pixel_method, 2, 0, 1, 1);
    grid_bad_pixels->addWidget(cmb_outlier_processing_type, 2, 1, 1, 1);
    grid_bad_pixels->addWidget(lbl_bad_pixel_start_frame, 3, 0, 1, 1);
    grid_bad_pixels->addWidget(txt_bad_pixel_start_frame, 3, 1, 1, 1);
    grid_bad_pixels->addWidget(lbl_bad_pixel_stop_frame, 4, 0, 1, 1);
    grid_bad_pixels->addWidget(txt_bad_pixel_end_frame, 4, 1, 1, 1);
    grid_bad_pixels->addWidget(lbl_moving_median_window_length, 5, 0, 1, 1);
    grid_bad_pixels->addWidget(txt_moving_median_N, 5, 1, 1, 1);
    // grid_bad_pixels->addItem(spacerItem,5,2);
    grid_bad_pixels->addWidget(lbl_bad_pixel_sensitivity, 2, 3, 1, 1);
    grid_bad_pixels->addWidget(cmb_outlier_processing_sensitivity, 2, 4, 1, 1);
    grid_bad_pixels->addWidget(chk_highlight_bad_pixels, 4, 4, 1, 1);
    grid_bad_pixels->addWidget(lbl_bad_pixel_color, 3, 3, 1, 1);
    grid_bad_pixels->addWidget(cmb_bad_pixel_color,3, 4, 1,1);
    grid_bad_pixels->addWidget(btn_bad_pixel_identification, 5, 4, 1, 1);
    grid_bad_pixels->addItem(spacerItem,5,5);

    // ------------------------------------------------------------------------

    QGroupBox *grpbox_image_processing = new QGroupBox();
    QToolBox *toolbox_noise_suppresssion_methods = new QToolBox();
    toolbox_noise_suppresssion_methods->setStyleSheet(sub_toolbox_StyleSheet);
    QGridLayout *grid_image_processing = new QGridLayout(grpbox_image_processing);
    grid_image_processing->addWidget(toolbox_noise_suppresssion_methods,1,0,1,6);
    grpbox_FNS_processing = new QGroupBox("");
    // grpbox_FNS_processing->setStyleSheet("border: 1px solid gray; border-color: rgb(245, 200, 125); border-width: 1px;");
    QGridLayout* grid_FNS_processing = new QGridLayout(grpbox_FNS_processing);
    lbl_fixed_suppression = new QLabel("No Frames Selected");
    lbl_fixed_suppression->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    lbl_fixed_suppression->setStyleSheet("background-color: rgb(200,200,200);");
    chk_FNS_external_file = new QCheckBox("External File");
    chk_FNS_external_file->setFixedWidth(150);
    lbl_FNS_start_frame = new QLabel("Start:");
    lbl_FNS_start_frame->setFixedWidth(50);
    txt_FNS_start_frame = new QLineEdit("1");
    txt_FNS_start_frame->setFixedWidth(60);
    lbl_FNS_stop_frame = new QLabel("Stop:");
    lbl_FNS_stop_frame->setFixedWidth(50);
    txt_FNS_end_frame = new QLineEdit("50");
    txt_FNS_end_frame->setFixedWidth(60);
    btn_FNS = new QPushButton("Fixed Background Noise Suppression");
    btn_FNS->setFixedWidth(275);
    grid_FNS_processing->addWidget(lbl_fixed_suppression, 0, 0, 1, -1);
    grid_FNS_processing->addWidget(chk_FNS_external_file, 1, 0, 1, 2);
    grid_FNS_processing->addWidget(lbl_FNS_start_frame, 2, 0, 1, 1);
    grid_FNS_processing->addWidget(txt_FNS_start_frame, 2, 1, 1, 1);
    grid_FNS_processing->addWidget(lbl_FNS_stop_frame, 3, 0, 1, 1);
    grid_FNS_processing->addWidget(txt_FNS_end_frame, 3, 1, 1, 1);
    grid_FNS_processing->addWidget(btn_FNS,3,2,1,2);

    // ------------------------------------------------------------------------
    grpbox_ANS_processing = new QGroupBox("");
    // grpbox_ANS_processing->setStyleSheet("border: 1px solid gray; border-color: rgb(245, 200, 125); border-width: 1px;");
    QGridLayout* grid_ANS_processing = new QGridLayout(grpbox_ANS_processing);
    lbl_adaptive_noise_suppression_status = new QLabel("No Frames Setup");
    lbl_adaptive_noise_suppression_status->setStyleSheet("background-color:rgb(200,200,200);");
    lbl_ANS_offset_frames = new QLabel("Offset:");
    lbl_ANS_offset_frames->setFixedWidth(60);
    txt_ANS_offset_frames = new QLineEdit("-30");
    txt_ANS_offset_frames->setFixedWidth(60);
    lbl_ANS_number_frames = new QLabel("Number:");
    lbl_ANS_number_frames->setFixedWidth(60);
    txt_ANS_number_frames = new QLineEdit("5");
    txt_ANS_number_frames->setFixedWidth(60);
    chk_hide_shadow = new QCheckBox("Hide Shadow");
    chk_hide_shadow->setFixedWidth(110);
    chk_hide_shadow->setChecked(true);
    lbl_ANS_shadow_threshold = new QLabel("Shadow Threshold:");
    lbl_ANS_shadow_threshold->setFixedWidth(130);
    cmb_shadow_threshold = new QComboBox();
    cmb_shadow_threshold->setFixedWidth(100);
    cmb_shadow_threshold->addItem("3 sigma");
    cmb_shadow_threshold->addItem("2 sigma");
    cmb_shadow_threshold->addItem("1 sigma");
    btn_ANS = new QPushButton("Adaptive Noise Suppression");
    btn_ANS->setFixedWidth(200);
    grid_ANS_processing->addWidget(lbl_adaptive_noise_suppression_status, 0, 0, 1, -1);
    grid_ANS_processing->addWidget(lbl_ANS_offset_frames, 1, 0, 1, 1);
    grid_ANS_processing->addWidget(txt_ANS_offset_frames, 1, 1, 1, 1);
    grid_ANS_processing->addWidget(lbl_ANS_number_frames, 2, 0, 1, 1);
    grid_ANS_processing->addWidget(txt_ANS_number_frames, 2, 1, 1, 1);
    grid_ANS_processing->addWidget(chk_hide_shadow, 3, 0, 1, 1);
    grid_ANS_processing->addWidget(lbl_ANS_shadow_threshold, 3, 1, 1, 1);
    grid_ANS_processing->addWidget(cmb_shadow_threshold, 3, 2, 1, 1);
    grid_ANS_processing->addWidget(btn_ANS, 3, 4, 1, 2);
    QGroupBox *grpbox_RPCP_processing = new QGroupBox("");
    grpbox_RPCP_processing->setFlat(true);
    grpbox_RPCP_processing->setStyleSheet("border-width: 0px;");
    QGridLayout* grid_RPCP_processing = new QGridLayout(grpbox_RPCP_processing);
    btn_RPCP = new QPushButton("RPCP Noise Suppression");
    btn_RPCP->setFixedWidth(200);
    connect(btn_RPCP, &QPushButton::clicked, this, &SirveApp::ExecuteRPCPNoiseSuppression);
    grid_RPCP_processing->addWidget(btn_RPCP, 0, 0, 1, 1);
    // ------------------------------------------------------------------------
    QGroupBox * grpbox_deinterlacing = new QGroupBox("");
    grpbox_deinterlacing->setFixedHeight(100);
    // grpbox_deinterlacing->setStyleSheet("border: 1px solid gray; border-color: rgb(245, 200, 125); border-width: 1px;");
    QGridLayout* grid_deinterlacing = new QGridLayout(grpbox_deinterlacing);
    btn_deinterlace = new QPushButton("Deinterlace");
    btn_deinterlace->setFixedWidth(150);
    connect(btn_deinterlace, &QPushButton::clicked, this, &SirveApp::ExecuteDeinterlace);
    btn_deinterlace_current_frame = new QPushButton("Deinterlace Current Frame");
    btn_deinterlace_current_frame->setFixedWidth(175);
    connect(btn_deinterlace_current_frame, &QPushButton::clicked, this, &SirveApp::ExecuteDeinterlaceCurrent);
    chk_deinterlace_confirmation = new QCheckBox("Require Confirmation");
    chk_deinterlace_confirmation->setChecked(true);

    grid_deinterlacing->addWidget(btn_deinterlace,0,0,1,1);
    grid_deinterlacing->addWidget(btn_deinterlace_current_frame,0,1,1,1);
    grid_deinterlacing->addWidget(chk_deinterlace_confirmation,1,1,1,1);

    QToolBox *toolbox_image_processing = new QToolBox();
    toolbox_image_processing->addItem(grpbox_bad_pixels_correction,QString("Bad Pixel Correction"));
    toolbox_image_processing->addItem(grpbox_image_processing,QString("Noise Suppression"));
    toolbox_image_processing->addItem(grpbox_deinterlacing,QString("Deinterlacing"));
    toolbox_noise_suppresssion_methods->addItem(grpbox_FNS_processing,QString("Fixed Background Noise Suppresssion"));
    toolbox_noise_suppresssion_methods->addItem(grpbox_ANS_processing,QString("Adaptive Background Noise Suppresssion"));
    toolbox_noise_suppresssion_methods->addItem(grpbox_RPCP_processing,QString("RPCP Noise Suppresssion"));
    // ------------------------------------------------------------------------
    grpbox_Image_Shift = new QGroupBox();
    QSizePolicy grpbox_size_policy;
    QGridLayout* grid_Image_Shift = new QGridLayout(grpbox_Image_Shift);

    QLabel* lbl_OSM_track_ID = new QLabel("OSM Track ID:");
    lbl_OSM_track_ID->setFixedWidth(125);
    QLabel* lbl_manual_track_ID = new QLabel("Manual Track ID:");
    QLabel* lbl_track_centering_priority = new QLabel("Centering Priority:");
    QLabel* lbl_frame_stack_Nframes = new QLabel("Number of Frames:");
    txt_frame_stack_Nframes = new QLineEdit("5");
    txt_frame_stack_Nframes->setFixedWidth(50);
    cmb_OSM_track_IDs = new QComboBox();
    cmb_OSM_track_IDs->setFixedWidth(75);
    cmb_OSM_track_IDs->setCurrentIndex(0);
    cmb_manual_track_IDs = new QComboBox();
    cmb_manual_track_IDs->setCurrentIndex(0);
    cmb_manual_track_IDs->setFixedWidth(75);
    cmb_track_centering_priority = new QComboBox();
    cmb_track_centering_priority->setFixedWidth(150);
    cmb_track_centering_priority->addItem(QString("OSM"));
    cmb_track_centering_priority->addItem(QString("Manual"));
    cmb_track_centering_priority->addItem(QString("OSM then Manual"));
    cmb_track_centering_priority->addItem(QString("Manual then OSM"));
    btn_center_on_tracks = new QPushButton("Center on Tracks");
    btn_center_on_tracks->setFixedWidth(130);
    connect(btn_center_on_tracks, &QPushButton::clicked, this, &SirveApp::ExecuteCenterOnTracks);
    btn_center_on_brightest = new QPushButton("Center on Brightest");
    connect(btn_center_on_brightest, &QPushButton::clicked, this, &SirveApp::ExecuteCenterOnBrightest);
    btn_frame_stack = new QPushButton("Frame Stack");
    btn_frame_stack->setFixedWidth(150);
    connect(btn_frame_stack, &QPushButton::clicked, this, &SirveApp::ExecuteFrameStacking);

    grid_Image_Shift->addWidget(lbl_track_centering_priority,0,0,1,1);
    grid_Image_Shift->addWidget(cmb_track_centering_priority,0,1,1,2);
    grid_Image_Shift->addWidget(lbl_OSM_track_ID,1,0,1,1);
    grid_Image_Shift->addWidget(cmb_OSM_track_IDs,1,1,1,1);
    grid_Image_Shift->addItem(spacerItem,1,2);
    grid_Image_Shift->addWidget(lbl_manual_track_ID,2,0,1,1);
    grid_Image_Shift->addWidget(cmb_manual_track_IDs,2,1,1,1);
    grid_Image_Shift->addWidget(btn_center_on_tracks,1,3,1,1);
    grid_Image_Shift->addWidget(btn_center_on_brightest,2,3,1,1);
    grid_Image_Shift->addWidget(lbl_frame_stack_Nframes,3,0,1,1);
    grid_Image_Shift->addWidget(txt_frame_stack_Nframes,3,1,1,1);
    grid_Image_Shift->addWidget(btn_frame_stack,3,3,1,1);
    grid_Image_Shift->addItem(spacerItem,3,3);
    grid_Image_Shift->addItem(spacerItem,3,4);
    // // ------------------------------------------------------------------------

    toolbox_image_processing->addItem(grpbox_Image_Shift,QString("Image Stabilization"));
    vlayout_tab_processing->addWidget(toolbox_image_processing);
    // // ------------------------------------------------------------------------

    vlayout_tab_processing->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout

    return widget_tab_processing;
}

QWidget* SirveApp::SetupTracksTab(){

    QWidget* widget_tab_tracks = new QWidget(tab_menu);
    QVBoxLayout* vlayout_tab_workspace = new QVBoxLayout(widget_tab_tracks);
    QLabel *lbl_track = new QLabel("Manual Track Management");
    lbl_create_track_message = new QLabel("");
    btn_create_track = new QPushButton("Create Track");
    btn_finish_create_track = new QPushButton("Finish");
    btn_finish_create_track->setHidden(true);
    btn_import_tracks = new QPushButton("Import Tracks");

    QGridLayout* grid_workspace = new QGridLayout();

    grid_workspace->addWidget(lbl_track, 0, 0, 1, -1, Qt::AlignCenter);
    grid_workspace->addWidget(lbl_create_track_message, 1, 1, 1, 1);
    grid_workspace->addWidget(btn_create_track, 1, 0, 1, 1);
    grid_workspace->addWidget(btn_finish_create_track, 1, 0, 1, 1);
    grid_workspace->addWidget(btn_import_tracks, 2, 0, 1, 1);

    tm_widget = new TrackManagementWidget(widget_tab_tracks);
    QScrollArea *track_management_scroll_area = new QScrollArea();
    track_management_scroll_area->setWidgetResizable( true );
    track_management_scroll_area->setWidget(tm_widget);
    track_management_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    grid_workspace->addWidget(track_management_scroll_area, 3, 0, -1, -1);

    vlayout_tab_workspace->addLayout(grid_workspace);
    vlayout_tab_workspace->insertStretch(-1, 0);
    return widget_tab_tracks;
}

void SirveApp::SetupVideoFrame(){

    frame_video_player->setFrameShape(QFrame::Box);
    QVBoxLayout* vlayout_frame_video = new QVBoxLayout(frame_video_player);

    // ------------------------------------------------------------------------
    vlayout_frame_video->addLayout(video_display->video_display_layout);

    // ------------------------------------------------------------------------

    lbl_fps = new QLabel("fps");
    lbl_fps->setAlignment(Qt::AlignRight);

    // ------------------------------------------------------------------------

    slider_video = new QSlider();
    slider_video->setOrientation(Qt::Horizontal);

    vlayout_frame_video->addWidget(slider_video);

    // ------------------------------------------------------------------------

    int button_video_width = 40;
    int button_video_height = 40;

    //Add icons to video playback buttons
    QPixmap play_image("icons/play.png");
    QIcon play_icon(play_image);
    btn_play = new QPushButton();
    btn_play->resize(button_video_width, button_video_height);
    btn_play->setIcon(play_icon);
    btn_play->setProperty("id", "play");
    btn_play->setToolTip("Play Video");

    QPixmap pause_image("icons/pause.png");
    QIcon pause_icon(pause_image);
    btn_pause = new QPushButton();
    btn_pause->resize(button_video_width, button_video_height);
    btn_pause->setIcon(pause_icon);
    btn_pause->setProperty("id", "pause");
    btn_pause->setToolTip("Pause Video");

    QPixmap reverse_image("icons/reverse.png");
    QIcon reverse_icon(reverse_image);
    btn_reverse = new QPushButton();
    btn_reverse->resize(button_video_width, button_video_height);
    btn_reverse->setIcon(reverse_icon);
    btn_reverse->setProperty("id", "reverse");
    btn_reverse->setToolTip("Reverse Video");

    QPixmap speed_up_image("icons/chevron-double-up.png");
    QIcon speed_up_icon(speed_up_image);
    btn_fast_forward = new QPushButton();
    btn_fast_forward->resize(button_video_width, button_video_height);
    btn_fast_forward->setIcon(speed_up_icon);
    btn_fast_forward->setToolTip("Increase FPS");

    QPixmap next_frame_image("icons/skip-next.png");
    QIcon next_frame_icon(next_frame_image);
    btn_next_frame = new QPushButton();
    btn_next_frame->resize(button_video_width, button_video_height);
    btn_next_frame->setIcon(next_frame_icon);
    btn_next_frame->setProperty("id", "next");
    btn_next_frame->setToolTip("Next Frame");

    QPixmap slow_down_image("icons/chevron-double-down.png");
    QIcon slow_down_icon(slow_down_image);
    btn_slow_back = new QPushButton();
    btn_slow_back->resize(button_video_width, button_video_height);
    btn_slow_back->setIcon(slow_down_icon);

    btn_slow_back->setToolTip("Decrease FPS");

    QPixmap prev_frame_image("icons/skip-previous.png");
    QIcon prev_frame_icon(prev_frame_image);
    btn_prev_frame = new QPushButton();
    btn_prev_frame->resize(button_video_width, button_video_height);
    btn_prev_frame->setIcon(prev_frame_icon);
    btn_prev_frame->setProperty("id", "previous");
    btn_prev_frame->setToolTip("Previous Frame");

    QPixmap record_frame("icons/record.png");
    QIcon record_frame_icon(record_frame);
    btn_frame_record = new QPushButton();
    btn_frame_record->resize(button_video_width, button_video_height);
    btn_frame_record->setIcon(record_frame_icon);
    btn_frame_record->setToolTip("Record Video");

    QPixmap save_frame("icons/content-save.png");
    QIcon save_frame_icon(save_frame);
    btn_frame_save = new QPushButton();
    btn_frame_save->resize(button_video_width, button_video_height);
    btn_frame_save->setIcon(save_frame_icon);
    btn_frame_save->setToolTip("Save Frame");

    QPixmap zoom_image("icons/magnify.png");
    QIcon zoom_icon(zoom_image);
    btn_zoom = new QPushButton();
    btn_zoom->resize(button_video_width, button_video_height);
    btn_zoom->setIcon(zoom_icon);
    btn_zoom->setCheckable(true);

    QPixmap signal_image("icons/signal.png");
    QIcon signal_icon(signal_image);
    btn_calculate_radiance = new QPushButton();
    btn_calculate_radiance->resize(button_video_width, button_video_height);
    btn_calculate_radiance->setIcon(signal_icon);
    btn_calculate_radiance->setCheckable(true);

    QPixmap expand_image("icons/expand.png");
    QIcon expand_icon(expand_image);
    btn_popout_video = new QPushButton();
    btn_popout_video->resize(button_video_width, button_video_height);
    btn_popout_video->setIcon(expand_icon);
    btn_popout_video->setCheckable(true);

    QHBoxLayout* hlayout_video_buttons = new QHBoxLayout();

    hlayout_video_buttons->addWidget(btn_frame_save);
    hlayout_video_buttons->addWidget(btn_frame_record);
    hlayout_video_buttons->addWidget(btn_zoom);
    hlayout_video_buttons->addWidget(btn_calculate_radiance);
    hlayout_video_buttons->addWidget(btn_popout_video);
    hlayout_video_buttons->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout
    hlayout_video_buttons->addWidget(btn_prev_frame);
    hlayout_video_buttons->addWidget(btn_reverse);
    hlayout_video_buttons->addWidget(btn_pause);
    hlayout_video_buttons->addWidget(btn_play);
    hlayout_video_buttons->addWidget(btn_next_frame);
    hlayout_video_buttons->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout
    hlayout_video_buttons->addWidget(lbl_fps);
    hlayout_video_buttons->addWidget(btn_fast_forward);
    hlayout_video_buttons->addWidget(btn_slow_back);

    vlayout_frame_video->addLayout(hlayout_video_buttons);

}

void SirveApp::SetupPlotFrame() {

    tab_plots->setTabPosition(QTabWidget::North);

    // ------------------------------------------------------------------------

    QWidget* widget_tab_histogram = new QWidget();

    vlayout_tab_histogram = new QVBoxLayout(widget_tab_histogram);

    frame_histogram_rel = new QFrame();
    frame_histogram_rel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    frame_histogram_abs = new QFrame();
    frame_histogram_abs->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

    btn_popout_histogram = new QPushButton("Push to Popout Absolute Histogram");
    btn_popout_histogram->resize(40, 40);
    btn_popout_histogram->setCheckable(true);
    vlayout_tab_histogram->addWidget(btn_popout_histogram);
    chk_relative_histogram = new QCheckBox("Relative Histogram");
    vlayout_tab_histogram->addWidget(frame_histogram_abs);
    vlayout_tab_histogram->addWidget(frame_histogram_rel);
    vlayout_tab_histogram->addWidget(chk_relative_histogram);

    // ------------------------------------------------------------------------

    frame_plots = new QFrame();
    QLabel* label_x_axis_option = new QLabel("X-Axis");
    QLabel* label_y_axis_option = new QLabel("Y-Axis");
    QGroupBox* plot_groupbox = new QGroupBox("Y-Axis Options");

    // create and group radial boxes
    rad_decimal = new QRadioButton("Decimal");
    rad_scientific = new QRadioButton("Scientific");
    rad_log = new QRadioButton("Log");
    rad_linear = new QRadioButton("Linear");

    //QButtonGroup data_plot_yformat, data_plot_yloglinear;
    data_plot_yformat = new QButtonGroup();
    data_plot_yloglinear = new QButtonGroup();
    data_plot_yformat->addButton(rad_decimal);
    data_plot_yformat->addButton(rad_scientific);
    data_plot_yloglinear->addButton(rad_log);
    data_plot_yloglinear->addButton(rad_linear);

    // create comboboxes and add options
    cmb_plot_xaxis = new QComboBox();
    cmb_plot_yaxis = new QComboBox();

    // create buttons in the plot controls
    btn_save_plot = new QPushButton();
    QPixmap save_frame("icons/content-save.png");
    QIcon save_frame_icon(save_frame);
    btn_save_plot->setIcon(save_frame_icon);
    btn_save_plot->setToolTip("Save Plot");

    btn_plot_menu = new QPushButton();
    QPixmap menu_image("icons/menu.png");
    QIcon menu_icon(menu_image);
    btn_plot_menu->setIcon(menu_icon);

    // establish layout of y-axis options
    QGridLayout* grid_plots_tab_color_groupbox = new QGridLayout(plot_groupbox);
    grid_plots_tab_color_groupbox->addWidget(rad_linear, 0, 0);
    grid_plots_tab_color_groupbox->addWidget(rad_log, 1, 0);
    grid_plots_tab_color_groupbox->addWidget(rad_decimal, 0, 1);
    grid_plots_tab_color_groupbox->addWidget(rad_scientific, 1, 1);

    // set layout for combo boxes
    QVBoxLayout* vlayout_y_axis_options = new QVBoxLayout();
    vlayout_y_axis_options->addWidget(label_y_axis_option);
    vlayout_y_axis_options->addWidget(cmb_plot_yaxis);
    vlayout_y_axis_options->setAlignment(Qt::AlignTop);

    QVBoxLayout* vlayout_x_axis_options = new QVBoxLayout();
    vlayout_x_axis_options->addWidget(label_x_axis_option);
    vlayout_x_axis_options->addWidget(cmb_plot_xaxis);
    vlayout_x_axis_options->setAlignment(Qt::AlignTop);

    QHBoxLayout* hlayout_buttons = new QHBoxLayout();
    hlayout_buttons->addWidget(btn_plot_menu);
    hlayout_buttons->addWidget(btn_save_plot);
    hlayout_buttons->setAlignment(Qt::AlignTop);

    // set layout for everything below the plot
    QHBoxLayout* hlayout_widget_plots_tab_color_control = new QHBoxLayout();
    hlayout_widget_plots_tab_color_control->addLayout(vlayout_y_axis_options);
    hlayout_widget_plots_tab_color_control->addLayout(vlayout_x_axis_options);
    hlayout_widget_plots_tab_color_control->addWidget(plot_groupbox);
    hlayout_widget_plots_tab_color_control->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout
    hlayout_widget_plots_tab_color_control->addLayout(hlayout_buttons);

    // set layout for engineering plots tab
    QWidget* widget_plots_tab_color = new QWidget();
    QVBoxLayout* vlayout_widget_plots_tab_color = new QVBoxLayout(widget_plots_tab_color);

    vlayout_widget_plots_tab_color->addWidget(frame_plots);
    vlayout_widget_plots_tab_color->addLayout(hlayout_widget_plots_tab_color_control);

    // ------------------------------------------------------------------------
    // Add all to tab widget

    // set ratio of first to second elements on engineering plot tab
    vlayout_widget_plots_tab_color->setStretch(0, 7);
    vlayout_widget_plots_tab_color->setStretch(1, 1);

    tab_plots->addTab(widget_tab_histogram, "Histogram");
    tab_plots->addTab(widget_plots_tab_color, "Plots");

    //directoryPicker = new DirectoryPicker(this);
}

void SirveApp::setupConnections() {


    //---------------------------------------------------------------------------

    connect(&video_display->container, &VideoContainer::updateDisplayVideo, this, &SirveApp::HandleFrameChange);
    connect(btn_undo_step, &QPushButton::clicked, &video_display->container, &VideoContainer::PopProcessingState);
    connect(playback_controller, &FramePlayer::frameSelected, this, &SirveApp::HandleFrameNumberChange);

    connect(&video_display->container, &VideoContainer::stateAdded, this, &SirveApp::HandleNewProcessingState);
    connect(&video_display->container, &VideoContainer::stateRemoved, this, &SirveApp::HandleProcessingStateRemoval);
    connect(&video_display->container, &VideoContainer::statesCleared, this, &SirveApp::HandleProcessingStatesCleared);

    connect(cmb_processing_states, qOverload<int>(&QComboBox::currentIndexChanged), &video_display->container, &VideoContainer::SelectState);
    connect(cmb_processing_states, qOverload<int>(&QComboBox::currentIndexChanged), this, &SirveApp::HandleProcessingNewStateSelected);

    connect(histogram_plot, &HistogramLinePlot::clickDragHistogram, this, &SirveApp::HandleHistogramClick);

    connect(video_display, &VideoDisplay::addNewBadPixels, this, &SirveApp::ReceiveNewBadPixels);
    connect(video_display, &VideoDisplay::removeBadPixels, this, &SirveApp::ReceiveNewGoodPixels);

    //---------------------------------------------------------------------------

    // connect(tab_menu, &QTabWidget::currentChanged, this, &SirveApp::HandlePlotDisplayAutoChange);
    connect(chk_relative_histogram, &QCheckBox::toggled, this, &SirveApp::HandleRelativeHistogramToggle);

    //---------------------------------------------------------------------------
    // Link color correction sliders to changing color correction values
    connect(slider_gain, &QSlider::valueChanged, this, &SirveApp::HandleGainSliderToggled);
    connect(slider_lift, &QSlider::valueChanged, this, &SirveApp::HandleLiftSliderToggled);

    connect(btn_reset_color_correction, &QPushButton::clicked, this, &SirveApp::ResetColorCorrection);

    connect(chk_auto_lift_gain, &QCheckBox::stateChanged, this, &SirveApp::HandleAutoLiftGainCheck);
    connect(txt_lift_sigma, &QLineEdit::editingFinished, this, &SirveApp::UpdateGlobalFrameVector);
    connect(txt_gain_sigma, &QLineEdit::editingFinished, this, &SirveApp::UpdateGlobalFrameVector);
    //---------------------------------------------------------------------------

    connect(chk_show_tracks, &QCheckBox::stateChanged, this, &SirveApp::HandleOsmTracksToggle);
    connect(cmb_tracker_color, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::EditTrackerColor);

    connect(chk_sensor_track_data, &QCheckBox::stateChanged, video_display, &VideoDisplay::HandleSensorBoresightDataCheck);
    connect(chk_show_time, &QCheckBox::stateChanged, video_display, &VideoDisplay::HandleFrameTimeToggle);
    connect(cmb_color_maps, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::EditColorMap);
    connect(cmb_text_color, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::EditBannerColor);

    connect(btn_add_annotations, &QPushButton::clicked, this, &SirveApp::AnnotateVideo);
    connect(btn_change_banner_text, &QPushButton::clicked, this, &SirveApp::EditBannerText);

    //---------------------------------------------------------------------------

    // Link horizontal slider to playback controller
    connect(playback_controller, &FramePlayer::frameSelected, slider_video, &QSlider::setValue);
    connect(slider_video, &QSlider::valueChanged, playback_controller, &FramePlayer::set_current_frame_number);

    //---------------------------------------------------------------------------

    // Link playback to play controls
    connect(btn_play, &QPushButton::clicked, playback_controller, &FramePlayer::StartTimer);
    connect(btn_pause, &QPushButton::clicked, playback_controller, &FramePlayer::StopTimer);

    connect(btn_reverse, &QPushButton::clicked, playback_controller, &FramePlayer::ReverseTimer);

    connect(btn_fast_forward, &QPushButton::clicked, playback_controller, &FramePlayer::IncreaseTimerInterval);
    connect(btn_slow_back, &QPushButton::clicked, playback_controller, &FramePlayer::DecreaseTimerInterval);
    connect(btn_next_frame, &QPushButton::clicked, playback_controller, &FramePlayer::GotoNextFrame);
    connect(video_display, &VideoDisplay::advanceFrame, playback_controller, &FramePlayer::CustomAdvanceFrame);
    connect(btn_prev_frame, &QPushButton::clicked, playback_controller, &FramePlayer::GotoPrevFrame);
    connect(btn_frame_record, &QPushButton::clicked, this, &SirveApp::StartStopVideoRecording);

    connect(btn_fast_forward, &QPushButton::clicked, this, &SirveApp::UpdateFps);
    connect(btn_slow_back, &QPushButton::clicked, this, &SirveApp::UpdateFps);

    connect(btn_zoom, &QPushButton::clicked, this, &SirveApp::HandleZoomOnVideoToggle);
    connect(btn_calculate_radiance, &QPushButton::clicked, this, &SirveApp::HandleCalculationOnVideoToggle);
    connect(video_display, &VideoDisplay::clearMouseButtons, this, &SirveApp::ClearZoomAndCalculationButtons);

    connect(btn_popout_video, &QPushButton::clicked, this, &SirveApp::HandlePopoutVideoClick);

    //---------------------------------------------------------------------------

    //Link buttons to functions
    connect(btn_get_frames, &QPushButton::clicked, this, &SirveApp::UiLoadAbirData);
    connect(txt_end_frame, &QLineEdit::returnPressed, this, &SirveApp::UiLoadAbirData);

    connect(chk_highlight_bad_pixels, &QPushButton::clicked, video_display, &VideoDisplay::HighlightBadPixels);

    connect(btn_FNS, &QPushButton::clicked, this, &SirveApp::ExecuteFixedNoiseSuppression);

    connect(btn_ANS, &QPushButton::clicked, this, &SirveApp::ExecuteAdaptiveNoiseSuppression);

    //---------------------------------------------------------------------------
    connect(btn_import_tracks, &QPushButton::clicked, this, &SirveApp::ImportTracks);
    connect(btn_create_track, &QPushButton::clicked, this, &SirveApp::HandleCreateTrackClick);
    connect(btn_finish_create_track, &QPushButton::clicked, this, &SirveApp::HandleFinishCreateTrackClick);
    connect(video_display, &VideoDisplay::finishTrackCreation, this, &SirveApp::HandleFinishCreateTrackClick);

    connect(tm_widget, &TrackManagementWidget::displayTrack, video_display, &VideoDisplay::ShowManualTrackId);
    connect(tm_widget, &TrackManagementWidget::hideTrack, video_display, &VideoDisplay::HideManualTrackId);
    connect(tm_widget, &TrackManagementWidget::deleteTrack, this, &SirveApp::HandleTrackRemoval);
    connect(tm_widget, &TrackManagementWidget::recolorTrack, this, &SirveApp::HandleManualTrackRecoloring);

    // Connect epoch button click to function
    connect(btn_apply_epoch, &QPushButton::clicked, this, &SirveApp::ApplyEpochTime);

    //Enable saving frame
    connect(btn_frame_save, &QPushButton::clicked, this, &SirveApp::SaveFrame);

    //---------------------------------------------------------------------------
    // Connect x-axis and y-axis changes to functions
    connect(cmb_plot_yaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::UpdatePlots);
    connect(cmb_plot_xaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::UpdatePlots);

    // Connect save button functions
    connect(btn_save_plot, &QPushButton::clicked, this, &SirveApp::SavePlot);

    //---------------------------------------------------------------------------
    // connect the plot radial buttons to adjust plot

    connect(rad_log, &QRadioButton::toggled, this, &SirveApp::UpdatePlots);
    connect(rad_decimal, &QRadioButton::toggled, this, &SirveApp::UpdatePlots);
    connect(rad_linear, &QRadioButton::toggled, this, &SirveApp::UpdatePlots);
    connect(rad_scientific, &QRadioButton::toggled, this, &SirveApp::UpdatePlots);

    //---------------------------------------------------------------------------
    connect(btn_popout_histogram, &QPushButton::clicked, this, &SirveApp::HandlePopoutHistogramClick);
}

void SirveApp::ImportTracks()
{
    QString base_track_folder = config_values.workspace_folder;
    QString file_selection = QFileDialog::getOpenFileName(this, ("Open Track File"), base_track_folder, ("Track File (*.csv)"));

    int compare = QString::compare(file_selection, "", Qt::CaseInsensitive);
    if (compare == 0) {
        QtHelpers::LaunchMessageBox(QString("Issue Finding File"), "No track file was selected.");
        return;
    }

    TrackFileReadResult result = track_info->ReadTracksFromFile(file_selection);

    if (QString::compare(result.error_string, "", Qt::CaseInsensitive) != 0)
    {
        QtHelpers::LaunchMessageBox("Issue Reading Tracks", result.error_string);
        return;
    }

    if (result.track_ids.find(currently_editing_or_creating_track_id) != result.track_ids.end())
    {
        QtHelpers::LaunchMessageBox("Forbidden", "You are not allowed to import a track with the same manual track ID that is currently being created or edited.");
        return;
    }

    std::set<int> previous_manual_track_ids = track_info->get_manual_track_ids();
    for ( int track_id : result.track_ids )
    {
        if (previous_manual_track_ids.find(track_id) != previous_manual_track_ids.end())
        {
            QtHelpers::LaunchMessageBox("Warning", "Warning: Overwriting track ID: " + QString::number(track_id));
        }
        video_display->AddManualTrackIdToShowLater(track_id);
        tm_widget->AddTrackControl(track_id);
        cmb_manual_track_IDs->addItem(QString::number(track_id));
    }

    track_info->AddManualTracks(result.frames);

    int index0 = data_plots->index_sub_plot_xmin;
    int index1 = data_plots->index_sub_plot_xmax + 1;
    video_display->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
    data_plots->UpdateManualPlottingTrackFrames(track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
    UpdatePlots();
}

void SirveApp::HandleCreateTrackClick()
{
    bool ok;
    int track_id = QInputDialog::getInt(this, tr("Select New Track Identifier"), tr("Track ID:"), -1, 1, 1000000, 1, &ok);
    if (!ok || track_id < 0)
    {
        return;
    }

    std::set<int> previous_manual_track_ids = track_info->get_manual_track_ids();
    if (previous_manual_track_ids.find(track_id) != previous_manual_track_ids.end())
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Confirm Track Overwriting", "The manual track ID you have chosen already exists. You can edit this track without saving, but finalizing this track will overwrite it. Are you sure you want to proceed with editing the existing manual track?");
        if (response == QMessageBox::Yes)
        {
            std::vector<std::optional<TrackDetails>> existing_track_details = track_info->CopyManualTrack(track_id);
            PrepareForTrackCreation(track_id);
            video_display->EnterTrackCreationMode(existing_track_details);
        }
    }
    else
    {
        std::vector<std::optional<TrackDetails>> empty_track_details = track_info->GetEmptyTrack();
        PrepareForTrackCreation(track_id);
        video_display->EnterTrackCreationMode(empty_track_details);
    }
}

void SirveApp::PrepareForTrackCreation(int track_id)
{
    currently_editing_or_creating_track_id = track_id;
    btn_create_track->setHidden(true);
    btn_finish_create_track->setHidden(false);
    lbl_create_track_message->setText("Editing Track: " + QString::number(currently_editing_or_creating_track_id));
    tab_menu->setTabEnabled(0, false);
    if (popout_video->isVisible())
    {
        popout_video->close();
    }
}

void SirveApp::HandleFinishCreateTrackClick()
{
    const std::vector<std::optional<TrackDetails>> & created_track_details = video_display->GetCreatedTrackDetails();
    bool any_contents = false;
    for (int i = 0; i < created_track_details.size(); i++)
    {
        if (created_track_details[i].has_value())
        {
            any_contents = true;
            break;
        }
    }
    if (!any_contents)
    {
        QtHelpers::LaunchMessageBox("Empty Track", "The manual track being edited is empty. The manual track will be discarded.");
        ExitTrackCreationMode();
        return;
    }

    auto response = QtHelpers::LaunchYesNoMessageBox("Finish Track Creation", "This action will finalize track creation. Pressing \"Yes\" will save the track, \"No\" will cancel track editing, and \"Cancel\" will return to track editing mode. Are you finished editing the track?", true);

    if (response == QMessageBox::Cancel)
    {
        return;
    }

    if (response == QMessageBox::Yes)
    {
        QString base_track_folder = config_values.workspace_folder;;
        QString new_track_file_name = QFileDialog::getSaveFileName(this, "Select a new file to save the track into", base_track_folder, "CSV (*.csv)");
        if (new_track_file_name.isEmpty())
        {
            QtHelpers::LaunchMessageBox("Returning to Track Creation", "An invalid or empty file was chosen. To prevent data loss, edited tracks must be saved to disk to finish track creation. Returning to track editing mode.");
            return;
        }

        tm_widget->AddTrackControl(currently_editing_or_creating_track_id);
        video_display->AddManualTrackIdToShowLater(currently_editing_or_creating_track_id);
        track_info->AddCreatedManualTrack(currently_editing_or_creating_track_id, created_track_details, new_track_file_name);

        int index0 = data_plots->index_sub_plot_xmin;
        int index1 = data_plots->index_sub_plot_xmax + 1;
        video_display->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
        data_plots->UpdateManualPlottingTrackFrames(track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
        UpdatePlots();
        cmb_manual_track_IDs->addItem(QString::number(currently_editing_or_creating_track_id));
    }

    ExitTrackCreationMode();
}

void SirveApp::ExitTrackCreationMode()
{
    btn_finish_create_track->setHidden(true);
    btn_create_track->setHidden(false);
    lbl_create_track_message->setText("");
    currently_editing_or_creating_track_id = -1;
    tab_menu->setTabEnabled(0, true);
    tab_menu->setTabEnabled(2, true);
    video_display->ExitTrackCreationMode();
}

void SirveApp::HandleTrackRemoval(int track_id)
{
    int ind_delete = cmb_manual_track_IDs->findText(QString::number(track_id));
    cmb_manual_track_IDs->removeItem(ind_delete);
    tm_widget->RemoveTrackControl(track_id);
    track_info->RemoveManualTrack(track_id);
    int index0 = data_plots->index_sub_plot_xmin;
    int index1 = data_plots->index_sub_plot_xmax + 1;
    video_display->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
    video_display->DeleteManualTrack(track_id);
    data_plots->UpdateManualPlottingTrackFrames(track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
    UpdatePlots();
}

void SirveApp::HandleManualTrackRecoloring(int track_id, QColor new_color)
{
    video_display->RecolorManualTrack(track_id, new_color);
    data_plots->Recolor_manual_track(track_id, new_color);
    UpdatePlots(); //Note: Engineering_Plots does not yet control its own graphical updates like VideoDisplay
}

void SirveApp::SaveWorkspace()
{
    if (abp_file_metadata.image_path == "" || video_display->container.get_processing_states().size() == 0) {
        QtHelpers::LaunchMessageBox(QString("Issue Saving Workspace"), "No frames are loaded, unable to save workspace.");
    }
    else {
        QString current_workspace_name = txt_workspace_name->text();

        QDate today = QDate::currentDate();
        QTime currentTime = QTime::currentTime();;
        QString formattedDate = today.toString("MM-dd-yyyy") + "_" + currentTime.toString("HHmm");
        QString start_frame = QString::number(data_plots->index_sub_plot_xmin + 1);
        QString stop_frame = QString::number(data_plots->index_sub_plot_xmax + 1);
        QString initial_name = abpimage_file_base_name + "_" + start_frame + "-"+ stop_frame + "_" + formattedDate;

        QString suggested_name = current_workspace_name.length() > 0 ? current_workspace_name : initial_name;
        QString workspace_name = QFileDialog::getSaveFileName(this, tr("Workspace Name"), suggested_name, tr("Workspace Files *.json"));

        QFileInfo fileInfo(workspace_name);
        txt_current_workspace_folder->setText(fileInfo.path());
        workspace->SaveState(fileInfo.fileName(), config_values.workspace_folder, abp_file_metadata.image_path, data_plots->index_sub_plot_xmin + 1, data_plots->index_sub_plot_xmax + 1, video_display->container.get_processing_states(), video_display->annotation_list);
        txt_workspace_name->setText(workspace_name);
    }
}

void SirveApp::LoadWorkspace()
{
     QString current_workspace_name = QFileDialog::getOpenFileName(this, tr("Select Workspace"), config_values.workspace_folder,  tr("Images (*.json)"));
    int compare = QString::compare(current_workspace_name, "", Qt::CaseInsensitive);
    if (compare != 0){
        WorkspaceValues workspace_vals = workspace->LoadState(current_workspace_name);

        int compare = QString::compare(workspace_vals.image_path, "", Qt::CaseInsensitive);
        if (compare == 0) {
            QtHelpers::LaunchMessageBox(QString("Issue Loading Workspace"), "The workspace is empty.");
            return;
        }
        QFileInfo fileInfo(current_workspace_name);
        QString filePath = fileInfo.path();

        txt_current_workspace_folder->setText(filePath);
        txt_workspace_name->setText(current_workspace_name);
        bool validated = ValidateAbpFiles(workspace_vals.image_path);
        if (validated) {
            LoadOsmData();
            cmb_text_color->setCurrentIndex(2);
        }

        if (workspace_vals.start_frame == 0 || workspace_vals.end_frame == 0)
        {
            return;
        }
        else
        {
            LoadAbirData(workspace_vals.start_frame, workspace_vals.end_frame);
        }

        processingState original = workspace_vals.all_states[0];
        if (original.replaced_pixels.size() > 0)
        {
            std::vector<unsigned int> bad_pixels = original.replaced_pixels;
            ReplaceBadPixels(bad_pixels);
        }

        for (auto i = 1; i < workspace_vals.all_states.size(); i++)
        {
            processingState current_state = workspace_vals.all_states[i];

            switch (current_state.method)
            {
                case ProcessingMethod::RPCP_noise_suppression:
                {
                    int source_state_ID = current_state.source_state_ID ;
                    ApplyRPCPNoiseSuppression(source_state_ID);
                    break;
                }

                case ProcessingMethod::adaptive_noise_suppression:
                {
                    bool hide_shadow_choice = current_state.ANS_hide_shadow;
                    int source_state_ID = current_state.source_state_ID ;
                    ApplyAdaptiveNoiseSuppression(current_state.ANS_relative_start_frame, current_state.ANS_num_frames, hide_shadow_choice, current_state.ANS_shadow_threshold, source_state_ID);
                    break;
                }
                case ProcessingMethod::deinterlace:{
                    int min_frame = ConvertFrameNumberTextToInt(txt_start_frame->text());
                    int max_frame = ConvertFrameNumberTextToInt(txt_end_frame->text());
                    std::vector<TrackFrame> osmFrames = track_info->get_osm_frames(min_frame - 1, max_frame);
                    int source_state_ID = current_state.source_state_ID ;
                    ApplyDeinterlacing(current_state.deint_type, source_state_ID);
                    break;
                }
                case ProcessingMethod::fixed_noise_suppression:{
                    int source_state_ID = current_state.source_state_ID ;
                    ApplyFixedNoiseSuppression(workspace_vals.image_path, current_state.FNS_file_path, current_state.FNS_start_frame, current_state.FNS_stop_frame, source_state_ID);
                    break;
                }
                case ProcessingMethod::center_on_OSM:{
                    QString trackTypePriority = "OSM";
                    int source_state_ID = current_state.source_state_ID ;
                    CenterOnTracks(trackTypePriority,current_state.track_id, current_state.offsets,current_state.find_any_tracks, source_state_ID);
                    break;
                }
                case ProcessingMethod::center_on_manual:{
                    QString trackTypePriority = "manual";
                    int source_state_ID = current_state.source_state_ID ;
                    CenterOnTracks(trackTypePriority,current_state.track_id, current_state.offsets,current_state.find_any_tracks, source_state_ID);
                    break;
                }
                case ProcessingMethod::center_on_brightest:{
                    int source_state_ID = current_state.source_state_ID ;
                    CenterOnBrightest(current_state.offsets,source_state_ID);
                    break;
                }
                case ProcessingMethod::frame_stacking:{
                    int source_state_ID = current_state.source_state_ID ;
                    FrameStacking(current_state.frame_stack_num_frames, source_state_ID);
                    break;
                }
                default:
                    QtHelpers::LaunchMessageBox(QString("Unexpected Workspace Behavior"), "Unexpected processing method in workspace, unable to proceed.");
            }
        }

        for (auto i = 0; i < workspace_vals.annotations.size(); i++)
        {
            AnnotationInfo anno = workspace_vals.annotations[i];
            video_display->annotation_list.push_back(anno);
        }
    }
}

void SirveApp::HandleAbpFileSelected()
{
    QString file_selection = QFileDialog::getOpenFileName(this, ("Open File"), "", ("Image File(*.abpimage)"));
    int compare = QString::compare(file_selection, "", Qt::CaseInsensitive);
    if (compare == 0) {
        QtHelpers::LaunchMessageBox(QString("Issue Finding File"), "No file was selected.");
        return;
    }

    bool validated = ValidateAbpFiles(file_selection);
    if (validated) {
        LoadOsmData();
        txt_start_frame->setStyleSheet(orange_styleSheet);
        txt_end_frame->setStyleSheet(orange_styleSheet);
        QFileInfo fileInfo(file_selection);
        abpimage_file_base_name = fileInfo.baseName();
    }
};

bool SirveApp::ValidateAbpFiles(QString path_to_image_file)
{
    AbpFileMetadata possible_abp_file_metadata = file_processor.LocateAbpFiles(path_to_image_file);

    if (!possible_abp_file_metadata.error_msg.isEmpty())
    {
        if (eng_data != NULL) {
            // if eng_data already initialized, allow user to re-select frames
            txt_start_frame->setEnabled(true);
            txt_end_frame->setEnabled(true);
            btn_get_frames->setEnabled(true);
        }
        else{
            txt_start_frame->setEnabled(false);
            txt_end_frame->setEnabled(false);
            btn_get_frames->setEnabled(false);
        }

        QtHelpers::LaunchMessageBox(QString("Issue Finding File"), possible_abp_file_metadata.error_msg);

        return false;
    }

    abp_file_metadata = possible_abp_file_metadata;

    return true;
};

void SirveApp::LoadOsmData()
{
    osm_frames = osm_reader.ReadOsmFileData(abp_file_metadata.osm_path);
    if (osm_frames.size() == 0)
    {
        QtHelpers::LaunchMessageBox(QString("Error loading OSM file"), QString("Error reading OSM file. Close program and open logs for details."));
        return;
    }

    txt_file_name->setText(abp_file_metadata.file_name);
    txt_file_name->setToolTip(abp_file_metadata.directory_path);

    txt_start_frame->setEnabled(true);
    txt_end_frame->setEnabled(true);
    btn_get_frames->setEnabled(true);

    QString osm_max_frames = QString::number(osm_frames.size());
    txt_start_frame->setText(QString("1"));
    txt_end_frame->setText(osm_max_frames);

    txt_max_frames->setText(osm_max_frames);

    SetLiftAndGain(0, 1);

    if (eng_data != NULL)
    {
        slider_video->setValue(0);
        ToggleVideoPlaybackOptions(false);

        // Reset video frame
        playback_controller->StopTimer();
        ResetColorCorrection();

        // delete objects with existing data within them
        delete eng_data;
        delete track_info;
        delete data_plots;
        delete engineering_plot_layout;

        video_display->container.ClearProcessingStates();
        video_display->RemoveFrame();
        histogram_plot->RemoveHistogramPlots();

        tab_menu->setTabEnabled(0, false);
        tab_menu->setTabEnabled(1, false);
        tab_menu->setTabEnabled(2, false);
        cmb_processing_states->setEnabled(false);
        txt_start_frame->setStyleSheet(orange_styleSheet);
        txt_end_frame->setStyleSheet(orange_styleSheet);
    }

    eng_data = new EngineeringData(osm_frames);
    track_info = new TrackInformation(osm_frames);
    data_plots = new EngineeringPlots(osm_frames);

    connect(btn_pause, &QPushButton::clicked, data_plots, &EngineeringPlots::HandlePlayerButtonClick);
    connect(btn_play, &QPushButton::clicked, data_plots, &EngineeringPlots::HandlePlayerButtonClick);
    connect(btn_reverse, &QPushButton::clicked, data_plots, &EngineeringPlots::HandlePlayerButtonClick);
    connect(btn_next_frame, &QPushButton::clicked, data_plots, &EngineeringPlots::HandlePlayerButtonClick);
    connect(btn_prev_frame, &QPushButton::clicked, data_plots, &EngineeringPlots::HandlePlayerButtonClick);

    size_t num_tracks = track_info->get_track_count();
    if (num_tracks == 0)
    {
        QtHelpers::LaunchMessageBox(QString("No Tracking Data"), "No tracking data was found within the file. No data will be plotted.");
    }

    data_plots->past_midnight = eng_data->get_seconds_from_midnight();
    data_plots->past_epoch = eng_data->get_seconds_from_epoch();

    data_plots->set_plotting_track_frames(track_info->get_osm_plotting_track_frames(), track_info->get_track_count());

    //--------------------------------------------------------------------------------
    // Enable setting of epoch
    dt_epoch->setEnabled(true);
    btn_apply_epoch->setEnabled(true);

    std::vector<double> epoch0 = eng_data->get_epoch(osm_frames);
    std::vector<double> epoch_min = eng_data->get_adj_epoch(-2, osm_frames);
    std::vector<double> epoch_max = eng_data->get_adj_epoch(2, osm_frames);
    UpdateEpochString(CreateEpochString(epoch0));
    DisplayOriginalEpoch(CreateEpochString(epoch0));

    QDate new_date(epoch0[0], epoch0[1], epoch0[2]);
    QDate min_date(epoch_min[0], epoch_min[1], epoch_min[2]);
    QDate max_date(epoch_max[0], epoch_max[1], epoch_max[2]);

    dt_epoch->setDate(new_date);
    dt_epoch->setMinimumDate(min_date);
    dt_epoch->setMaximumDate(max_date);

    dt_epoch->setTime(QTime(epoch0[3], epoch0[4], epoch0[5]));

    //--------------------------------------------------------------------------------

    engineering_plot_layout = new QGridLayout();
    btn_popout_engineering = new QPushButton("Push to Popout Plots");
    btn_popout_engineering->resize(40, 40);
    btn_popout_engineering->setCheckable(true);
    connect(btn_popout_engineering, &QPushButton::clicked, this, &SirveApp::HandlePopoutEngineeringClick);
    engineering_plot_layout->addWidget(btn_popout_engineering);
    engineering_plot_layout->addWidget(data_plots->chart_view);
    frame_plots->setLayout(engineering_plot_layout);

    btn_calculate_radiance->setChecked(false);
    btn_calculate_radiance->setEnabled(false);
    chk_highlight_bad_pixels->setChecked(false);
    chk_highlight_bad_pixels->setEnabled(false);

    btn_create_track->setEnabled(false);
    btn_import_tracks->setEnabled(false);

    CalibrationData temp;
    calibration_model = temp;

    // Reset settings on video playback to defaults
    chk_show_tracks->setChecked(true);
    chk_show_time->setChecked(false);
    chk_sensor_track_data->setChecked(false);
    cmb_text_color->setCurrentIndex(0);
    video_display->InitializeToggles();

    // Reset setting engineering plot defaults
    menu_plot_all_data->setIconVisibleInMenu(true);

    menu_plot_primary->setIconVisibleInMenu(false);

    menu_plot_frame_marker->setIconVisibleInMenu(false);

    EnableEngineeringPlotOptions();
    data_plots->SetPlotTitle(QString("EDIT CLASSIFICATION"));

    return;
}

void SirveApp::UiLoadAbirData()
{
    btn_get_frames->setEnabled(false);

    int min_frame = ConvertFrameNumberTextToInt(txt_start_frame->text());
    int max_frame = ConvertFrameNumberTextToInt(txt_end_frame->text());

    if (!VerifyFrameSelection(min_frame, max_frame)) {
        btn_get_frames->setEnabled(true);

        return;
    }

    LoadAbirData(min_frame, max_frame);
}

void SirveApp::LoadAbirData(int min_frame, int max_frame)
{
    lbl_progress_status->setText(QString("Loading frames..."));
    progress_bar_main->setRange(0,4);
    progress_bar_main->setValue(0);
    progress_bar_main->setTextVisible(true);
    // Load the ABIR data
    playback_controller->StopTimer();
    ABIRDataResult abir_data_result = file_processor.LoadImageFile(abp_file_metadata.image_path, min_frame, max_frame, config_values.version);
    progress_bar_main->setValue(1);
    lbl_progress_status->setText(QString("Configuring Application..."));
    progress_bar_main->setValue(2);
    if (abir_data_result.had_error) {
        QtHelpers::LaunchMessageBox(QString("Error Reading ABIR Frames"), "Error reading .abpimage file. See log for more details.");
        btn_get_frames->setEnabled(true);
        return;
    }

    std::vector<std::vector<uint16_t>> video_frames = abir_data_result.video_frames_16bit;
    unsigned int number_frames = static_cast<unsigned int>(video_frames.size());

    int x_pixels = abir_data_result.x_pixels;
    int y_pixels = abir_data_result.y_pixels;
    int max_value = abir_data_result.max_value;
    VideoDetails vid_details = {x_pixels, y_pixels, max_value, video_frames};

    processingState primary = { ProcessingMethod::original, vid_details };
    video_display->container.ClearProcessingStates();
    video_display->container.AddProcessingState(primary);

    txt_start_frame->setText(QString::number(min_frame));
    txt_end_frame->setText(QString::number(max_frame));

    //---------------------------------------------------------------------------
    // Set frame number for playback controller and valid values for slider
    playback_controller->set_number_of_frames(number_frames);
    slider_video->setRange(0, number_frames - 1);

    // Start threads...
    if (!thread_timer.isRunning())
    {
        thread_video.start();
        thread_timer.start();
    }

    int index0 = min_frame - 1;
    int index1 = max_frame;
    std::vector<PlottingFrameData> temp = eng_data->get_subset_plotting_frame_data(index0, index1);
    lbl_progress_status->setText(QString("Finalizing application state"));
    progress_bar_main->setValue(3);
    video_display->InitializeTrackData(track_info->get_osm_frames(index0, index1), track_info->get_manual_frames(index0, index1));
    cmb_OSM_track_IDs->addItem("Primary");
    cmb_manual_track_IDs->clear();
    cmb_manual_track_IDs->addItem("Primary");
    std::set<int> track_ids = track_info->get_OSM_track_ids();
    for ( int track_id : track_ids ){
        cmb_OSM_track_IDs->addItem(QString::number(track_id));
    }

    video_display->InitializeFrameData(min_frame, temp, file_processor.abir_data.ir_data);
    video_display->ReceiveVideoData(x_pixels, y_pixels);
    UpdateGlobalFrameVector();

    // Reset engineering plots with new sub plot indices
    data_plots->index_sub_plot_xmin = min_frame - 1;
    data_plots->index_sub_plot_xmax = max_frame - 1;
    data_plots->plot_all_data = false;
    menu_plot_all_data->setIconVisibleInMenu(false);
    data_plots->plot_current_marker = true;
    menu_plot_frame_marker->setIconVisibleInMenu(true);
    UpdatePlots();

    //Update frame marker on engineering plot
    connect(playback_controller, &FramePlayer::frameSelected, data_plots, &EngineeringPlots::PlotCurrentStep);
    connect(this->data_plots->chart_view, &NewChartView::updatePlots, this, &SirveApp::UpdatePlots);

    connect(this->data_plots->chart_view, &NewChartView::updateFrameLine, this, &SirveApp::HandleZoomAfterSlider);

    playback_controller->set_initial_speed_index(10);
    UpdateFps();

    progress_bar_main->setValue(4);

    tab_plots->setCurrentIndex(1);

    btn_get_frames->setEnabled(true);

    tab_menu->setTabEnabled(0, true);
    tab_menu->setTabEnabled(1, true);
    tab_menu->setTabEnabled(2, true);
    tab_menu->setCurrentIndex(0);

    lbl_bad_pixel_count->setText("");

    cmb_processing_states->setEnabled(true);

    btn_create_track->setEnabled(true);
    btn_import_tracks->setEnabled(true);

    ToggleVideoPlaybackOptions(true);

    progress_bar_main->setValue(0);
    progress_bar_main->setTextVisible(false);
    lbl_progress_status->setText(QString(""));
}

void SirveApp::HandlePopoutEngineeringClick(bool checked)
{
    if (checked)
    {
        OpenPopoutEngineeringPlot();
    }
    else
    {
        popout_engineering->close();
    }
}

void SirveApp::OpenPopoutEngineeringPlot()
{
    popout_engineering->acquire(data_plots->chart_view);
    connect(popout_engineering, &QDialog::finished, this, &SirveApp::HandlePopoutEngineeringClosed);
    popout_engineering->open();
}

void SirveApp::HandlePopoutEngineeringClosed()
{
    btn_popout_engineering->setChecked(false);
    engineering_plot_layout->addWidget(data_plots->chart_view);
    frame_plots->setLayout(engineering_plot_layout);
}

void SirveApp::HandlePopoutHistogramClick(bool checked)
{
    if (checked) {
        OpenPopoutHistogramPlot();
    }
    else
    {
        popout_histogram->close();
    }
}

void SirveApp::OpenPopoutHistogramPlot()
{
    popout_histogram->acquire(histogram_plot->abs_chart_view);
    connect(popout_histogram, &QDialog::finished, this, &SirveApp::HandlePopoutHistogramClosed);
    popout_histogram->open();
}

void SirveApp::HandlePopoutHistogramClosed()
{
    btn_popout_histogram->setChecked(false);
    histogram_abs_layout->addWidget(histogram_plot->abs_chart_view);
}

void SirveApp::HandlePopoutVideoClick(bool checked)
{
    if (checked) {
        OpenPopoutVideoDisplay();
    }
    else {
        popout_video->close();
    }
}

void SirveApp::OpenPopoutVideoDisplay()
{
    video_display->lbl_image_canvas->disable();
    video_display->lbl_image_canvas->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    popout_video->acquire(video_display->lbl_image_canvas);
    connect(popout_video, &QDialog::finished, this, &SirveApp::HandlePopoutVideoClosed);
    popout_video->open();

}

void SirveApp::HandlePopoutVideoClosed()
{
    video_display->lbl_image_canvas->enable();
    video_display->lbl_image_canvas->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btn_popout_video->setChecked(false);
    video_display->ReclaimLabel();
}

void SirveApp::HandleZoomAfterSlider()
{
    data_plots->PlotCurrentStep(playback_controller->get_current_frame_number());
}

void SirveApp::StartStopVideoRecording()
{

    if (record_video)
    {
        //Stopping record video

        video_display->StopRecording();

        QPixmap record_image("icons/record.png");
        QIcon record_icon(record_image);
        btn_frame_record->setIcon(record_icon);
        btn_frame_record->setText("");
        btn_frame_record->setToolTip("Start Record");
        btn_frame_record->setEnabled(true);

        record_video = false;
    }
    else {
        //Starting record video
        bool file_opened = video_display->StartRecording(playback_controller->get_fps());

        if (file_opened) {

            QPixmap stop_image("icons/stop.png");
            QIcon stop_icon(stop_image);
            btn_frame_record->setIcon(stop_icon);
            btn_frame_record->setText("");
            btn_frame_record->setToolTip("Stop Record");
            btn_frame_record->setEnabled(true);

            record_video = true;
        }
        else
        {
            QtHelpers::LaunchMessageBox(QString("Video Record Failed "), "Video file could not be saved to this location");
        }

    }

}


void SirveApp::HandleZoomOnVideoToggle() {

    bool status_zoom_btn = btn_zoom->isChecked();

    if (status_zoom_btn)
    {
        video_display->ToggleActionZoom(true);
        btn_calculate_radiance->setChecked(false);
    }
    else {
        video_display->ToggleActionZoom(false);
    }

}


void SirveApp::HandleCalculationOnVideoToggle()
{

    bool status_calculation_btn = btn_calculate_radiance->isChecked();

    if (status_calculation_btn) {

        video_display->ToggleActionCalculateRadiance(true);
        btn_zoom->setChecked(false);
    }
    else {
        video_display->ToggleActionCalculateRadiance(false);
    }
}

void SirveApp::ClearZoomAndCalculationButtons()
{
    btn_zoom->setChecked(false);
    btn_calculate_radiance->setChecked(false);
}

void SirveApp::UpdateFps()
{
    QString fps = QString::number(playback_controller->speeds[playback_controller->index_speed], 'g', 2);
    fps.append(" fps");

    lbl_fps->setText(fps);
}


void SirveApp::HandleHistogramClick(double x0, double x1) {
    if (chk_auto_lift_gain->isChecked())
    {
        return;
    }
    // connects the clickable histogram to the main program

    // get current lift/gain values
    double lift_value = slider_lift->value() / 1000.;
    double gain_value = slider_gain->value() / 1000.;

    // defines the space around limit lines that will allow user to adjust limits
    double click_spacing = 0.015;

    // if user click is not near limits, then disregard click
    if (abs(lift_value - x0) >= click_spacing && abs(gain_value - x0) >= click_spacing) {

        return;
    }

    // define area around limit lines that clicks will be valid
    double gain_ll = gain_value - click_spacing;
    double gain_ul = gain_value + click_spacing;
    double lift_ll = lift_value - click_spacing;
    double lift_ul = lift_value + click_spacing;

    // check if space between limit lines is less than 2 * spacing. this is a special case
    if (gain_value - lift_value <= 2 * click_spacing)
    {
        // find mid-point between limits and set new upper/lower limits for gain/lift
        double mid_point = (gain_value - lift_value) / 2.0;
        gain_ll = gain_value - mid_point;
        lift_ul = lift_value + mid_point;
    }

    // if user click is closest to gain limit, adjust value
    if (x0 >= gain_ll && x0 <= gain_ul)
    {

        // if drag goes past upper range, then stop at upper range
        if (x1 > 1)
            x1 = 1;

        // if drag goes past lift value, then stop change before limit
        if (x1 < lift_value + 0.01)
            x1 = lift_value + 0.01;

        slider_gain->setValue(x1 * 1000);
    }

    // if user click is closest to lift limit, adjust value
    if (x0 >= lift_ll && x0 <= lift_ul)
    {

        // if drag goes past lower range, then stop at lower range
        if (x1 < 0)
            x1 = 0;

        // if drag goes past gain value, then stop change before limit
        if (x1 > gain_value - 0.01)
            x1 = gain_value - 0.01;

        slider_lift->setValue(x1 * 1000);
    }

}

void SirveApp::HandleAutoLiftGainCheck(int state)
{
    if (state == Qt::Checked)
    {
        slider_lift->setEnabled(false);
        slider_gain->setEnabled(false);

        UpdateGlobalFrameVector();

        btn_reset_color_correction->setEnabled(false);
        grpbox_auto_lift_gain->setEnabled(true);
    }
    else
    {
        slider_lift->setEnabled(true);
        slider_gain->setEnabled(true);

        btn_reset_color_correction->setEnabled(true);
        grpbox_auto_lift_gain->setEnabled(false);
    }
}

void SirveApp::SetLiftAndGain(double lift, double gain)
{
    slider_lift->setValue(lift * 1000);
    lbl_lift_value->setText(QString::number(lift, 'f', 2));

    slider_gain->setValue(gain * 1000);
    lbl_gain_value->setText(QString::number(gain, 'f', 2));
}

void SirveApp::HandleLiftSliderToggled() {
    if (chk_auto_lift_gain->isChecked())
    {
        return;
    }

    int lift_value = slider_lift->value();
    int gain_value = slider_gain->value();

    if (lift_value >= gain_value)
    {
        lift_value = gain_value - 1;
    }

    slider_lift->setValue(lift_value);
    lbl_lift_value->setText(QString::number(lift_value / 1000., 'f', 2));

    UpdateGlobalFrameVector();
}

void SirveApp::HandleGainSliderToggled() {
    if (chk_auto_lift_gain->isChecked())
    {
        return;
    }

    int lift_value = slider_lift->value();
    int gain_value = slider_gain->value();

    if (gain_value <= lift_value)
    {
        gain_value = lift_value + 1;
    }

    slider_gain->setValue(gain_value);
    lbl_gain_value->setText(QString::number(gain_value / 1000., 'f', 2));

    UpdateGlobalFrameVector();
}

void SirveApp::ResetColorCorrection()
{
    slider_lift->setValue(0);
    slider_gain->setValue(1000);
    chk_relative_histogram->setChecked(false);
}


void SirveApp::HandlePlotFullDataToggle()
{
    data_plots->plot_all_data = !data_plots->plot_all_data;
    menu_plot_all_data->setIconVisibleInMenu(data_plots->plot_all_data);

    UpdatePlots();
}

void SirveApp::HandlePlotPrimaryOnlyToggle()
{
    data_plots->plot_primary_only = !data_plots->plot_primary_only;
    menu_plot_primary->setIconVisibleInMenu(data_plots->plot_primary_only);

    UpdatePlots();
}

void SirveApp::HandlePlotCurrentFrameMarkerToggle()
{
    data_plots->plot_current_marker = !data_plots->plot_current_marker;
    menu_plot_frame_marker->setIconVisibleInMenu(data_plots->plot_current_marker);

    UpdatePlots();
}


// void SirveApp::ShowCalibrationDialog()
// {
// 	CalibrationDialog calibrate_dialog(calibration_model);

// 	auto response = calibrate_dialog.exec();

// 	if (response == 0) {

// 		return;
// 	}

// 	calibration_model = calibrate_dialog.model;
//     video_display->SetCalibrationModel(calibrate_dialog.model);
// 	btn_calculate_radiance->setEnabled(true);
// }

void SirveApp::SetDataTimingOffset()
{

    if (!eng_data)
        return;

    bool ok;
    double d = QInputDialog::getDouble(this, "Set Offset Time for Data", "Offset (seconds):", eng_data->get_offset_time(), -86400, 86400, 3, &ok, Qt::WindowFlags(), 1);
    if (ok) {
        eng_data->set_offset_time(d);

        data_plots->past_midnight = eng_data->get_seconds_from_midnight();
        data_plots->past_epoch = eng_data->get_seconds_from_epoch();

        int index0 = data_plots->index_sub_plot_xmin;
        int index1 = data_plots->index_sub_plot_xmax;

        std::vector<PlottingFrameData> temp = eng_data->get_subset_plotting_frame_data(index0, index1);
        video_display->UpdateFrameData(temp);

        UpdatePlots();
    }
}

void SirveApp::ChangeWorkspaceDirectory()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"), config_values.workspace_folder);

    if (!directory.isEmpty())
    {
        configReaderWriter::SaveWorkspaceFolder(directory);
        config_values = configReaderWriter::ExtractWorkspaceConfigValues();
        workspace = new Workspace(config_values.workspace_folder);
        txt_current_workspace_folder->setText(config_values.workspace_folder);
        txt_workspace_name->clear();
    }
}

void SirveApp::CloseWindow()
{
    close();
}

void SirveApp::SavePlot()
{
    data_plots->SavePlot();
}

void SirveApp::SaveFrame()
{
    if(playback_controller->is_running())
        playback_controller->StopTimer();

    video_display->SaveFrame();

    if(playback_controller->is_running())
        playback_controller->StartTimer();
}

void SirveApp::CreateMenuActions()
{
    QIcon on("icons/check.png");

    action_load_OSM = new QAction("Load OSM");
    action_load_OSM->setStatusTip("Load OSM abpimage file");
    connect(action_load_OSM, &QAction::triggered, this, &SirveApp::HandleAbpFileSelected);

    action_load_workspace = new QAction("Load Workspace");
    connect(action_load_workspace, &QAction::triggered, this, &SirveApp::LoadWorkspace);

    action_save_workspace = new QAction("Save Workspace");
    connect(action_save_workspace, &QAction::triggered, this, &SirveApp::SaveWorkspace);

    action_close = new QAction("Close");
    action_close->setStatusTip("Close main window");
    connect(action_close, &QAction::triggered, this, &SirveApp::CloseWindow);

    action_set_timing_offset = new QAction("Set Timing Offset");
    action_set_timing_offset->setStatusTip("Set a time offset to apply to collected data");
    connect(action_set_timing_offset, &QAction::triggered, this, &SirveApp::SetDataTimingOffset);

    action_change_workspace_directory = new QAction("Change Workspace Directory");
    action_change_workspace_directory->setStatusTip("Customize workspace directory so it points to your own folder.");
    connect(action_change_workspace_directory, &QAction::triggered, this, &SirveApp::ChangeWorkspaceDirectory);

    menu_file = menuBar()->addMenu(tr("&File"));
    menu_file->addAction(action_load_OSM);
    // menu_file->addAction(action_load_frames);
    menu_file->addAction(action_load_workspace);
    menu_file->addAction(action_save_workspace);
    menu_file->addAction(action_change_workspace_directory);
    menu_file->addAction(action_close);

    menu_settings = menuBar()->addMenu(tr("&Settings"));
    menu_settings->addAction(action_set_timing_offset);



    // ------------------------- PLOT MENU ACTIONS -------------------------

    menu_plot_all_data = new QAction(tr("&Plot all frame data"), this);
    menu_plot_all_data->setIcon(on);
    menu_plot_all_data->setStatusTip(tr("Plot all data from OSM file"));
    menu_plot_all_data->setIconVisibleInMenu(true);
    connect(menu_plot_all_data, &QAction::triggered, this, &SirveApp::HandlePlotFullDataToggle);

    menu_plot_primary = new QAction(tr("&Plot Primary Data Only"), this);
    menu_plot_primary->setIcon(on);
    menu_plot_primary->setStatusTip(tr("Plot only the primary object"));
    menu_plot_primary->setIconVisibleInMenu(false);
    connect(menu_plot_primary, &QAction::triggered, this, &SirveApp::HandlePlotPrimaryOnlyToggle);

    menu_plot_frame_marker = new QAction(tr("&Plot Marker for Current Frame"), this);
    menu_plot_frame_marker->setIcon(on);
    menu_plot_frame_marker->setStatusTip(tr("Plot marker to show current video frame"));
    menu_plot_frame_marker->setIconVisibleInMenu(false);
    connect(menu_plot_frame_marker, &QAction::triggered, this, &SirveApp::HandlePlotCurrentFrameMarkerToggle);

    menu_plot_edit_banner = new QAction(tr("&Edit Banner Text"), this);
    menu_plot_edit_banner->setStatusTip(tr("Edit the banner text for the plot"));
    connect(menu_plot_edit_banner, &QAction::triggered, this, &SirveApp::EditPlotText);

    menu_plot_edit_banner = new QAction(tr("&Export Tracking Data"), this);
    menu_plot_edit_banner->setStatusTip(tr("Export the plotted data to file"));
    connect(menu_plot_edit_banner, &QAction::triggered, this, &SirveApp::ExportPlotData);

    // ---------------------- Set Acctions to Menu --------------------

    plot_menu = new QMenu(this);
    plot_menu->addAction(menu_plot_all_data);
    plot_menu->addAction(menu_plot_primary);
    plot_menu->addAction(menu_plot_frame_marker);
    plot_menu->addAction(menu_plot_edit_banner);

    btn_plot_menu->setMenu(plot_menu);

}

void SirveApp::EditBannerText()
{
    bool ok;
    QString input_text = QInputDialog::getText(0, "Banner Text", "Input Banner Text", QLineEdit::Normal, video_display->banner_text, &ok);

    if (!ok)
    {
        return;
    }

    video_display->UpdateBannerText(input_text);

    // checks if banners are the same and asks user if they want them to be the same
    QString plot_banner_text = data_plots->title;
    int check = QString::compare(input_text, plot_banner_text, Qt::CaseSensitive);
    if (check != 0)
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Update All Banners", "Video and plot banners do not match. Would you like to set both to the same banner?");
        if (response == QMessageBox::Yes)
        {
            data_plots->SetPlotTitle(input_text);
        }
    }
}

void SirveApp::EditPlotText()
{
    bool ok;
    QString input_text = QInputDialog::getText(0, "Plot Header Text", "Input Plot Header Text", QLineEdit::Normal, data_plots->title, &ok);

    if (ok)
    {
        data_plots->SetPlotTitle(input_text);
    }
}

void SirveApp::ExportPlotData()
{

    QStringList items;
    items << "Export All Data" << "Export Only Selected Data";

    bool ok;
    QString item = QInputDialog::getItem(this, "Export Data", "Select Data to Export", items, 0, false, &ok);

    if (!ok && !item.isEmpty())
        return;

    QString path = QFileDialog::getSaveFileName(this, ("Save File"), "", ("csv(*.csv)"));
    std::string save_path = path.toStdString();

    if (path.size() == 0)
        return;

    unsigned int min_frame, max_frame;
    if (item == "Export All Data")
    {
        DataExport::WriteTrackDataToCsv(save_path, eng_data->get_plotting_frame_data(), track_info->get_osm_plotting_track_frames(), track_info->get_manual_plotting_frames());
    }
    else {
        min_frame = data_plots->index_sub_plot_xmin;
        max_frame = data_plots->index_sub_plot_xmax;

        DataExport::WriteTrackDataToCsv(save_path, eng_data->get_plotting_frame_data(), track_info->get_osm_plotting_track_frames(), track_info->get_manual_plotting_frames(), min_frame, max_frame);
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Export Data");
    msgBox.setText("Successfully exported track data to file");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

int SirveApp::GetCurrentColorIndex(QVector<QString> colors, QColor input_color) {

    int index_current_color;
    QString current_banner_color = input_color.name();
    for (int i = 0; i < colors.size(); i++)
    {
        QColor check_color(colors[i]);
        int check = QString::compare(current_banner_color, check_color.name(), Qt::CaseInsensitive);
        if (check == 0)
        {
            index_current_color = i;
        }
    }

    return index_current_color;
}


void SirveApp::EditColorMap()
{
    // QString color = cmb_color_maps->currentText();

    int i = cmb_color_maps->currentIndex();

    color_map_display->set_color_map(video_colors.maps[i].colors,lbl_lift_value->text().toDouble(),lbl_gain_value->text().toDouble());
    video_display->HandleColorMapUpdate(video_colors.maps[i].colors);

}

void SirveApp::EditBannerColor()
{
    QString color = cmb_text_color->currentText();
    video_display->UpdateBannerColor(color);
}

void SirveApp::EditTrackerColor()
{
    QString tracker_color = cmb_tracker_color->currentText();
    video_display->HandleTrackerColorUpdate(tracker_color);
}

void SirveApp::handle_outlier_processing_change()
{
    if(cmb_outlier_processing_type->currentIndex() == 0){
        txt_moving_median_N->setEnabled(false);
    }
    else{
        txt_moving_median_N->setEnabled(true);
    }
}
void SirveApp::edit_bad_pixel_color()
{
    QString bad_pixel_color = cmb_bad_pixel_color->currentText();
    video_display->HighlightBadPixelsColors(bad_pixel_color);
}

void SirveApp::UpdatePlots()
{
    // x - axis
    // Index 0 - Frames
    // Index 1 - Seconds from Midnight
    // Index 2 - Seconds from Epoch

    // y - axis
    // Index 0 - Irradiance
    // Index 1 - Azimuth
    // Index 2 - Elevation

    int x_index = cmb_plot_xaxis->currentIndex();
    int y_index = cmb_plot_yaxis->currentIndex();

    // Check that indices are all positive
    if (x_index >= 0 && y_index >= 0 && eng_data)
    {

        bool scientific_is_checked = rad_scientific->isChecked();
        bool log_is_checked = rad_log->isChecked();
        data_plots->toggle_yaxis_log(log_is_checked);

        // For x-axis, use scientific notation here for 'irradiance' only (irradiance option is first combo box option):
        data_plots->toggle_yaxis_scientific(scientific_is_checked && cmb_plot_yaxis->currentIndex() == 0 );

        // For y-axis, use fixed-point precision for 'seconds past' options only ('frame' option is first combo box option):
        data_plots->toggle_xaxis_fixed_pt(cmb_plot_xaxis->currentIndex() != 0);

        switch (x_index)
        {
            case 0:
                data_plots->set_xaxis_units(frames);
                break;

            case 1:
                data_plots->set_xaxis_units(seconds_past_midnight);
                break;

            case 2:
                data_plots->set_xaxis_units(seconds_from_epoch);
                break;

            default:
                break;
        }

        data_plots->SetYAxisChartId(y_index);
        data_plots->PlotChart();

        data_plots->PlotCurrentStep(playback_controller->get_current_frame_number());
    }

}

void SirveApp::AnnotateVideo()
{
    VideoInfo standard_info;
    standard_info.x_pixels = video_display->image_x;
    standard_info.y_pixels = video_display->image_y;

    standard_info.min_frame = data_plots->index_sub_plot_xmin + 1;
    standard_info.max_frame = data_plots->index_sub_plot_xmax + 1;

    annotate_gui = new AnnotationListDialog(video_display->annotation_list, standard_info);

    connect(annotate_gui, &AnnotationListDialog::showAnnotationStencil, video_display, &VideoDisplay::ShowStencil);
    connect(annotate_gui, &AnnotationListDialog::hideAnnotationStencil, video_display, &VideoDisplay::HideStencil);
    connect(annotate_gui, &AnnotationListDialog::updateAnnotationStencil, video_display, &VideoDisplay::InitializeStencilData);

    connect(video_display->annotation_stencil, &AnnotationStencil::mouseMoved, annotate_gui, &AnnotationListDialog::UpdateStencilPosition);
    connect(video_display->annotation_stencil, &AnnotationStencil::mouseReleased, annotate_gui, &AnnotationListDialog::SetStencilLocation);

    annotate_gui->show();
}

int SirveApp::ConvertFrameNumberTextToInt(QString input)
{
    bool convert_value_numeric;
    int value = input.toInt(&convert_value_numeric);

    if (convert_value_numeric)
    {
        return value;
    }
    else
    {
        return -1;
    }
}

void SirveApp::HandleRelativeHistogramToggle(bool input)
{
    if (input) {
        frame_histogram_rel->setHidden(false);
        vlayout_tab_histogram->addWidget(frame_histogram_rel);
    }
    else
    {
        frame_histogram_rel->setHidden(true);
        vlayout_tab_histogram->removeWidget(frame_histogram_rel);
    }
}

void SirveApp::ApplyEpochTime()
{
    double year, month, day, hr, min;
    double sec, msec, epoch_jdate;

    // --------------------------------------------------------------------
    // Get new date

    QDate date = dt_epoch->date();
    QTime time = dt_epoch->time();

    year = date.year();
    month = date.month();
    day = date.day();

    hr = time.hour();
    min = time.minute();
    sec = time.second();
    msec = time.msec() / 1000;

    sec += msec;

    std::vector<double> epoch{year, month, day, hr, min, sec};
    UpdateEpochString(CreateEpochString(epoch));
    // --------------------------------------------------------------------

    epoch_jdate = jtime::JulianDate(year, month, day, hr, min, sec);
    eng_data->update_epoch_time(epoch_jdate);

    data_plots->past_epoch = eng_data->get_seconds_from_epoch();
    UpdatePlots();
}

void SirveApp::HandleBadPixelReplacement()
{
    auto response = QtHelpers::LaunchYesNoMessageBox("Bad Pixel Confirmation", "Replacing bad pixels will reset all filters and modify the original frame. Are you sure you want to continue?");
    int min_frame = 1;
    int max_frame = osm_frames.size();
    processingState base_state = video_display->container.processing_states[0];
    int number_video_frames = static_cast<int>(base_state.details.frames_16bit.size());
    if (response == QMessageBox::Yes) {

        bool ok;

        int type_choice = cmb_bad_pixels_type->currentIndex();

        int outlier_method = cmb_outlier_processing_type->currentIndex();

        int outlier_sensitivity = cmb_outlier_processing_sensitivity->currentIndex();

        double N = 6.0;
        if (type_choice != 1){

            if (outlier_sensitivity == 0){
                N = 6.0;
            }
            else if (outlier_sensitivity == 1){
                N = 5.0;
            }
            else if (outlier_sensitivity == 2){
                N = 4.0;
            }
            else{
                N = 3.0;
            }
        }

        int start_frame = txt_bad_pixel_start_frame->text().toInt();
        int end_frame = txt_bad_pixel_end_frame->text().toInt();
        ABIRDataResult test_frames = file_processor.LoadImageFile(abp_file_metadata.image_path, start_frame, end_frame, config_values.version);
        lbl_progress_status->setText(QString("Finding bad pixels..."));
        progress_bar_main->setRange(0,number_video_frames - 1);
        progress_bar_main->setTextVisible(true);
        std::vector<unsigned int> dead_pixels;
        ImageProcessing BP;
        connect(&BP, &ImageProcessing::SignalProgress, progress_bar_main, &QProgressBar::setValue);
        connect(btn_cancel_operation, &QPushButton::clicked, &BP, &ImageProcessing::CancelOperation);
        if(type_choice == 0){
            lbl_progress_status->setText(QString("Finding dead pixels..."));
            arma::uvec index_dead0 = BP.FindDeadBadscalePixels(test_frames.video_frames_16bit);
            lbl_progress_status->setText(QString("Finding outlier pixels..."));
            if (outlier_method == 0){
                arma::uvec index_outlier0 =  BP.IdentifyBadPixelsMedian(N,test_frames.video_frames_16bit);
                index_outlier0 = arma::unique(arma::join_vert(index_outlier0,index_dead0));
                dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier0);
            }
            else{
                u_int window_length = txt_moving_median_N->text().toUInt();
                arma::uvec index_outlier0 =  BP.IdentifyBadPixelsMovingMedian(window_length,N,test_frames.video_frames_16bit);
                index_outlier0 = arma::unique(arma::join_vert(index_outlier0,index_dead0));
                dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier0);
            }
        } else if (type_choice == 1){
            lbl_progress_status->setText(QString("Finding outlier pixels..."));
            arma::uvec index_dead1 = BP.FindDeadBadscalePixels(test_frames.video_frames_16bit);
            dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_dead1);
        } else {
            lbl_progress_status->setText(QString("Finding outlier pixels..."));
            if (outlier_method == 0){
                arma::uvec index_outlier2 = BP.IdentifyBadPixelsMedian(N,test_frames.video_frames_16bit);
                dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier2);
            }
            else{
                u_int window_length = txt_moving_median_N->text().toUInt();
                arma::uvec index_outlier2 = BP.IdentifyBadPixelsMovingMedian(window_length,N,test_frames.video_frames_16bit);
                dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier2);
            }
        }
        if(dead_pixels.size()>0){
            ReplaceBadPixels(dead_pixels);
        }
        progress_bar_main->setValue(0);
        progress_bar_main->setTextVisible(false);
        lbl_progress_status->setText(QString(""));
    }
}


void SirveApp::ReceiveNewBadPixels(std::vector<unsigned int> new_pixels)
{
    std::vector<unsigned int> bad_pixels = video_display->container.processing_states[0].replaced_pixels;

    unsigned int count_new = 0;
    for (auto i = 0; i < new_pixels.size(); i++)
    {
        unsigned int candidate_pixel = new_pixels[i];
        if (std::find(bad_pixels.begin(), bad_pixels.end(), candidate_pixel) == bad_pixels.end())
        {
            bad_pixels.push_back(candidate_pixel);
            count_new += 1;
        }
    }

    if (count_new == 0)
    {
        QtHelpers::LaunchMessageBox("No Action Taken", "No new bad pixels will be replaced.");
    }
    else
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Bad Pixel Confirmation", "Replacing bad pixels will reset all filters and modify the original frame. Are you sure you want to continue? Number of new bad pixels to add: " + QString::number(count_new));

        if (response == QMessageBox::Yes)
        {
            ReplaceBadPixels(bad_pixels);
        }
    }
}

void SirveApp::ReplaceBadPixels(std::vector<unsigned int> & pixels_to_replace)
{
    processingState base_state = video_display->container.processing_states[0];
    int number_video_frames = static_cast<int>(base_state.details.frames_16bit.size());
    base_state.replaced_pixels = pixels_to_replace;
    lbl_progress_status->setText(QString("Replacing bad pixels..."));
    progress_bar_main->setRange(0,number_video_frames - 1);
    progress_bar_main->setTextVisible(true);
    ImageProcessing BP;
    connect(&BP, &ImageProcessing::SignalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, &BP, &ImageProcessing::CancelOperation);
    BP.ReplacePixelsWithNeighbors(base_state.details.frames_16bit, pixels_to_replace, base_state.details.x_pixels);
    progress_bar_main->setValue(0);
    progress_bar_main->setTextVisible(false);
    lbl_progress_status->setText(QString(""));
    if(pixels_to_replace.size()>0){
        video_display->container.ClearProcessingStates();
        uint16_t maxVal = std::numeric_limits<int>::min(); // Initialize with the smallest possible int
        for (const auto& row : base_state.details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
            base_state.details.max_value = maxVal;
            video_display->container.AddProcessingState(base_state);
            lbl_bad_pixel_count->setText("Bad pixels currently replaced: " + QString::number(pixels_to_replace.size()));
            chk_highlight_bad_pixels->setEnabled(true);
    }
}

void SirveApp::ReceiveNewGoodPixels(std::vector<unsigned int> pixels)
{
    std::vector<unsigned int> bad_pixels = video_display->container.processing_states[0].replaced_pixels;

    unsigned int count_to_remove = 0;
    for (auto i = 0; i < pixels.size(); i++)
    {
        unsigned int candidate_pixel = pixels[i];
        std::vector<unsigned int>::iterator position = std::find(bad_pixels.begin(), bad_pixels.end(), candidate_pixel);
        if (position != bad_pixels.end())
        {
            bad_pixels.erase(position);
            count_to_remove += 1;
        }
    }

    if (count_to_remove == 0)
    {
        QtHelpers::LaunchMessageBox("No Action Taken", "No bad pixels will be marked as good.");
    }
    else
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Bad Pixel Confirmation", "Removing bad pixels will reset all filters and require re-reading the original data. Are you sure you want to continue? Number of bad pixels that will be marked as good: " + QString::number(count_to_remove));

        if (response == QMessageBox::Yes)
        {
            int min_frame = data_plots->index_sub_plot_xmin + 1;
            int max_frame = data_plots->index_sub_plot_xmax + 1;
            LoadAbirData(min_frame, max_frame);

            ReplaceBadPixels(bad_pixels);
        }
    }
}

void SirveApp::ApplyFixedNoiseSuppressionFromExternalFile()
{
    ExternalNUCInformationWidget external_nuc_dialog;

    auto response = external_nuc_dialog.exec();

    if (response == 0) {
        return;
    }

    QString image_path = external_nuc_dialog.abp_metadata.image_path;
    unsigned int start_frame = external_nuc_dialog.start_frame;
    unsigned int end_frame = external_nuc_dialog.stop_frame;
    txt_FNS_start_frame->setText(QString::number(start_frame));
    txt_FNS_end_frame->setText(QString::number(end_frame));
    try
    {
        // assumes file version is same as base file opened
        int source_state_ind = cmb_processing_states->currentIndex();
        ApplyFixedNoiseSuppression(abp_file_metadata.image_path, image_path, start_frame, end_frame, source_state_ind);
    }
    catch (const std::exception& e)
    {
        // catch any errors when loading frames. try-catch not needed when loading frames from same file since no errors originally occurred
        //TODO: LAUNCHMESSAGEBOX
        QMessageBox msgBox;
        msgBox.setWindowTitle(QString("NUC Correction from External File"));
        QString box_text = "Error occurred when loading the frames for the NUC. See log for details.  ";
        msgBox.setText(box_text);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }
}

void SirveApp::ExecuteFixedNoiseSuppression()
{
    //Pause the video if it's running
    playback_controller->StopTimer();

    if (!chk_FNS_external_file->isChecked())
    {
        int delta_frames = data_plots->index_sub_plot_xmax - data_plots->index_sub_plot_xmin;
        int start_frame = txt_FNS_start_frame->text().toInt();
        int end_frame = txt_FNS_end_frame->text().toInt();
        int source_state_ind = cmb_processing_states->currentIndex();
        ApplyFixedNoiseSuppression(abp_file_metadata.image_path, abp_file_metadata.image_path, start_frame, end_frame, source_state_ind);
    }
    else
    {
        ApplyFixedNoiseSuppressionFromExternalFile();
    }

}

void SirveApp::ApplyFixedNoiseSuppression(QString image_path, QString file_path, unsigned int start_frame, unsigned int end_frame, int source_state_ind)
{
    int compare = QString::compare(file_path, image_path, Qt::CaseInsensitive);
    if (compare!=0){
        if (!VerifyFrameSelection(start_frame, end_frame))
        {
            QtHelpers::LaunchMessageBox(QString("Invalid Frame Selection"), "Fixed noise suppression not completed, invalid frame selection");
            return;
        }
    }

    processingState original = video_display->container.CopyCurrentStateIdx(source_state_ind);

    processingState noise_suppresion_state = original;
    noise_suppresion_state.details.frames_16bit.clear();

    int number_video_frames = static_cast<int>(original.details.frames_16bit.size());

    ImageProcessing FNS;
    progress_bar_main->setRange(0,number_video_frames - 1);
    progress_bar_main->setTextVisible(true);
    lbl_progress_status->setText(QString("Fixed Noise Suppression..."));
    connect(&FNS, &ImageProcessing::SignalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, &FNS, &ImageProcessing::CancelOperation);
    noise_suppresion_state.details.frames_16bit = FNS.FixedNoiseSuppression(abp_file_metadata.image_path, file_path, start_frame, end_frame, config_values.version, original.details);
    progress_bar_main->setValue(0);
    progress_bar_main->setTextVisible(false);
    lbl_progress_status->setText(QString(""));
    if(noise_suppresion_state.details.frames_16bit.size()>0){
        noise_suppresion_state.method = ProcessingMethod::fixed_noise_suppression;
        noise_suppresion_state.FNS_file_path = file_path;
        noise_suppresion_state.FNS_start_frame = start_frame;
        noise_suppresion_state.FNS_stop_frame = end_frame;
        noise_suppresion_state.source_state_ID = source_state_ind;
        uint16_t maxVal = std::numeric_limits<int>::min(); // Initialize with the smallest possible int
        for (const auto& row : noise_suppresion_state.details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        noise_suppresion_state.details.max_value = maxVal;
        noise_suppresion_state.state_ID = video_display->container.processing_states.size() + 1;
        video_display->container.AddProcessingState(noise_suppresion_state);

        QFileInfo fi(file_path);
        QString fileName = fi.fileName().toLower();
        QString current_filename = abp_file_metadata.file_name.toLower() + ".abpimage";

        if (fileName == current_filename)
            fileName = "Current File";

        QString description = "File: " + fileName + "\n";
        description += "From frame " + QString::number(start_frame) + " to " + QString::number(end_frame);

        lbl_fixed_suppression->setText(description);
    }
}

void SirveApp::ExecuteDeinterlace()
{
    DeinterlaceType deinterlace_method_type = static_cast<DeinterlaceType>(0);
    int source_state_ind = cmb_processing_states->currentIndex();
    ApplyDeinterlacing(deinterlace_method_type, source_state_ind);
}

void SirveApp::ExecuteDeinterlaceCurrent()
{
    DeinterlaceType deinterlace_method_type = static_cast<DeinterlaceType>(0);
    ApplyDeinterlacingCurrent(deinterlace_method_type);
}

void SirveApp::ApplyDeinterlacing(DeinterlaceType deinterlace_method_type, int source_state_ind)
{
    processingState original = video_display->container.CopyCurrentStateIdx(source_state_ind);

    processingState deinterlace_state = original;
    deinterlace_state.details.frames_16bit.clear();

    // Apply de-interlace to the frames

    int number_video_frames = static_cast<int>(original.details.frames_16bit.size());

    ImageProcessing DI;
    progress_bar_main->setRange(0,number_video_frames - 1);
    progress_bar_main->setTextVisible(true);
    lbl_progress_status->setText(QString("Deinterlacing..."));
    connect(&DI, &ImageProcessing::SignalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, &DI, &ImageProcessing::CancelOperation);
    deinterlace_state.details.frames_16bit = DI.DeinterlaceCrossCorrelation(osm_frames, original.details);
    progress_bar_main->setValue(0);
    lbl_progress_status->setText(QString(""));
    if(deinterlace_state.details.frames_16bit.size()>0){
        progress_bar_main->setTextVisible(false);
        deinterlace_state.method = ProcessingMethod::deinterlace;
        deinterlace_state.deint_type = deinterlace_method_type;
        deinterlace_state.source_state_ID = source_state_ind;
        uint16_t maxVal = std::numeric_limits<int>::min(); // Initialize with the smallest possible int
        for (const auto& row : deinterlace_state.details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        deinterlace_state.details.max_value = maxVal;
        deinterlace_state.state_ID =  video_display->container.processing_states.size() + 1;
        video_display->container.AddProcessingState(deinterlace_state);
    }
}

void SirveApp::ApplyDeinterlacingCurrent(DeinterlaceType deinterlace_method_type)
{
    processingState original = video_display->container.CopyCurrentStateIdx(cmb_processing_states->currentIndex());
    ImageProcessing DI;
    lbl_progress_status->setText(QString("Deinterlacing..."));
    int framei = video_display->counter;
    std::vector<uint16_t> current_frame_16bit = original.details.frames_16bit[framei];
    std::vector<uint16_t> current_frame_16bit_0 = current_frame_16bit;
    int nRows = original.details.y_pixels;
    int nCols = original.details.x_pixels;
    video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[video_display->counter] = DI.DeinterlaceCrossCorrelationCurrent(framei, nRows, nCols, current_frame_16bit);
    lbl_progress_status->setText(QString(""));
    UpdateGlobalFrameVector();
    if(chk_deinterlace_confirmation->isChecked()){
        auto response = QtHelpers::LaunchYesNoMessageBox("Deinterlace Frame Confirmation", "Keep result? (can not be undone after accepted)");
        if (response != QMessageBox::Yes) {
            video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[video_display->counter] = current_frame_16bit_0;
            UpdateGlobalFrameVector();
        }
    }
}

void SirveApp::ExecuteCenterOnTracks()
{
    int track_id;
    boolean findAnyTrack = false;
    QString trackTypePriority;
    if(cmb_track_centering_priority->currentIndex()==0 || cmb_track_centering_priority->currentIndex()==2){
        if (cmb_OSM_track_IDs->currentIndex()==0){
            track_id = -1;
        }
        else{
            track_id = cmb_OSM_track_IDs->currentText().toInt();
        }
        trackTypePriority = "OSM";
    }
    else if(cmb_track_centering_priority->currentIndex()==1 || cmb_track_centering_priority->currentIndex()==3){
        if (cmb_manual_track_IDs->currentIndex()==0){
            track_id = -1;
        }
        else{
            track_id = cmb_manual_track_IDs->currentText().toInt();
        }
        trackTypePriority = "Manual";
    }
    if(cmb_track_centering_priority->currentIndex()==2 || cmb_track_centering_priority->currentIndex()==3){
        findAnyTrack = true;
    }

    std::vector<std::vector<int>> track_centered_offsets;
    int source_state_ind = cmb_processing_states->currentIndex();
    CenterOnTracks(trackTypePriority, track_id, track_centered_offsets, findAnyTrack, source_state_ind);
}

void SirveApp::CenterOnTracks(QString trackTypePriority, int track_id, std::vector<std::vector<int>> & track_centered_offsets, boolean find_any_tracks, int source_state_ind)
{
    int OSMPriority = QString::compare(trackTypePriority,"OSM",Qt::CaseInsensitive);
    processingState original = video_display->container.CopyCurrentStateIdx(source_state_ind);

    processingState track_centered_state = original;
    track_centered_state.details.frames_16bit.clear();

    int number_frames = static_cast<int>(original.details.frames_16bit.size());
    int min_frame = ConvertFrameNumberTextToInt(txt_start_frame->text());
    int max_frame = ConvertFrameNumberTextToInt(txt_end_frame->text());
    std::vector<TrackFrame> osmFrames = track_info->get_osm_frames(min_frame - 1, max_frame);
    std::vector<TrackFrame> manualFrames = track_info->get_manual_frames(min_frame - 1, max_frame);
    track_centered_state.track_id = track_id;
    if (OSMPriority==0){
        track_centered_state.method = ProcessingMethod::center_on_OSM;
    }
    else{

        track_centered_state.method = ProcessingMethod::center_on_manual;
    }
    track_centered_state.find_any_tracks = find_any_tracks;
    ImageProcessing COST;
    progress_bar_main->setRange(0,number_frames - 1);
    lbl_progress_status->setText(QString("Center on OSM..."));
    connect(&COST, &ImageProcessing::SignalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, &COST, &ImageProcessing::CancelOperation);
    track_centered_state.details.frames_16bit = COST.CenterOnTracks(trackTypePriority, original.details, track_id, osmFrames, manualFrames, find_any_tracks, track_centered_offsets);
    progress_bar_main->setValue(0);
    progress_bar_main->setTextVisible(false);
    lbl_progress_status->setText(QString(""));
    if(track_centered_state.details.frames_16bit.size()>0){
        track_centered_state.offsets = track_centered_offsets;
        track_centered_state.source_state_ID = source_state_ind;
        uint16_t maxVal = std::numeric_limits<int>::min(); // Initialize with the smallest possible int
        for (const auto& row : track_centered_state.details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        track_centered_state.details.max_value = maxVal;
        track_centered_state.state_ID =  video_display->container.processing_states.size() + 1;
        video_display->container.AddProcessingState(track_centered_state);
    }
}

void SirveApp::ExecuteCenterOnBrightest()
{
    std::vector<std::vector<int>> brightest_centered_offsets;
    int source_state_ind = cmb_processing_states->currentIndex();
    CenterOnBrightest(brightest_centered_offsets,source_state_ind);
}

void SirveApp::CenterOnBrightest(std::vector<std::vector<int>> & brightest_centered_offsets, int source_state_ind)
{
    processingState original = video_display->container.CopyCurrentStateIdx(source_state_ind);
    processingState brightest_centered_state = original;
    brightest_centered_state.details.frames_16bit.clear();
    int number_video_frames = static_cast<int>(original.details.frames_16bit.size());
    int min_frame = ConvertFrameNumberTextToInt(txt_start_frame->text());
    int max_frame = ConvertFrameNumberTextToInt(txt_end_frame->text());
    brightest_centered_state.method = ProcessingMethod::center_on_brightest;
    progress_bar_main->setRange(0,number_video_frames - 1);
    progress_bar_main->setTextVisible(true);
    lbl_progress_status->setText(QString("Center on Brightest Object..."));
    ImageProcessing COB;
    connect(&COB, &ImageProcessing::SignalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, &COB, &ImageProcessing::CancelOperation);
    brightest_centered_state.details.frames_16bit = COB.CenterOnBrightest(original.details, brightest_centered_offsets);
    progress_bar_main->setValue(0);
    progress_bar_main->setTextVisible(false);
    lbl_progress_status->setText(QString(""));
    if(brightest_centered_state.details.frames_16bit.size()>0){
        brightest_centered_state.offsets = brightest_centered_offsets;
        brightest_centered_state.source_state_ID = source_state_ind;
        uint16_t maxVal = std::numeric_limits<int>::min(); // Initialize with the smallest possible int
        for (const auto& row : brightest_centered_state.details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        brightest_centered_state.details.max_value = maxVal;
        brightest_centered_state.state_ID =  video_display->container.processing_states.size() + 1;
        video_display->container.AddProcessingState(brightest_centered_state);
    }
}

void SirveApp::HandleOsmTracksToggle()
{
    bool current_status = chk_show_tracks->isChecked();
    video_display->ToggleOsmTracks(current_status);
    if (current_status) {
        cmb_tracker_color->setEnabled(true);
    }
    else
    {
        cmb_tracker_color->setEnabled(false);
    }
}

void SirveApp::HandleNewProcessingState(QString state_name, int index)
{
    cmb_processing_states->addItem(state_name);
    lbl_processing_description->setText(state_name);
    cmb_processing_states->setCurrentIndex(index);
}

void SirveApp::HandleProcessingStateRemoval(ProcessingMethod method, int index)
{
    cmb_processing_states->removeItem(index);

    if (method == ProcessingMethod::adaptive_noise_suppression)
    {
        lbl_adaptive_noise_suppression_status->setText("No Frames Setup");
    }
    else if (method == ProcessingMethod::fixed_noise_suppression)
    {
        lbl_fixed_suppression->setText("No Frames Selected");
    }
}
void SirveApp::HandleProcessingNewStateSelected()
{
    lbl_processing_description->setText(cmb_processing_states->currentText());
}

void SirveApp::HandleProcessingStatesCleared()
{
    cmb_processing_states->clear();
    lbl_adaptive_noise_suppression_status->setText("No Frames Setup");
    lbl_fixed_suppression->setText("No Frames Selected");
}

void SirveApp::ExecuteFrameStacking()
{
    int number_of_frames = txt_frame_stack_Nframes->text().toInt();
    int source_state_ind = cmb_processing_states->currentIndex();
    FrameStacking(number_of_frames, source_state_ind);
}

void SirveApp::FrameStacking(int number_of_frames, int source_state_ind)
{
    //Pause the video if it's running
    playback_controller->StopTimer();

    processingState original = video_display->container.CopyCurrentStateIdx(source_state_ind);
    int number_video_frames = static_cast<int>(original.details.frames_16bit.size());

    processingState frame_stacking_state = original;
    frame_stacking_state.details.frames_16bit.clear();

    ImageProcessing FS;
    lbl_progress_status->setText(QString("Frame Stacking..."));
    progress_bar_main->setRange(0,number_video_frames - 1);
    progress_bar_main->setTextVisible(true);
    connect(&FS, &ImageProcessing::SignalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, &FS, &ImageProcessing::CancelOperation);
    frame_stacking_state.details.frames_16bit = FS.FrameStacking(number_of_frames, original.details);
    progress_bar_main->setValue(0);
    progress_bar_main->setTextVisible(false);
    lbl_progress_status->setText(QString(""));
    if(frame_stacking_state.details.frames_16bit.size()>0){
        frame_stacking_state.method = ProcessingMethod::frame_stacking;
        frame_stacking_state.frame_stack_num_frames = number_of_frames;
        frame_stacking_state.source_state_ID = source_state_ind;
        uint16_t maxVal = std::numeric_limits<int>::min(); // Initialize with the smallest possible int
        for (const auto& row : frame_stacking_state.details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        frame_stacking_state.details.max_value = maxVal;
        frame_stacking_state.state_ID =  video_display->container.processing_states.size() + 1;
        video_display->container.AddProcessingState(frame_stacking_state);
    }
}

void SirveApp::ExecuteAdaptiveNoiseSuppression()
{
    //-----------------------------------------------------------------------------------------------
    // get user selected frames for suppression

    int delta_frames = data_plots->index_sub_plot_xmax - data_plots->index_sub_plot_xmin;

    int relative_start_frame = txt_ANS_offset_frames->text().toInt();
    int number_of_frames = txt_ANS_number_frames->text().toInt();
    bool hide_shadow_choice = chk_hide_shadow->isChecked();
    int shadow_sigma_thresh = 3 - cmb_shadow_threshold->currentIndex();
    int source_state_ind = cmb_processing_states->currentIndex();
    ApplyAdaptiveNoiseSuppression(relative_start_frame, number_of_frames, hide_shadow_choice, shadow_sigma_thresh, source_state_ind);
}

void SirveApp::ApplyAdaptiveNoiseSuppression(int relative_start_frame, int number_of_frames, bool hide_shadow_choice, int shadow_sigma_thresh, int source_state_ind)
{
    //Pause the video if it's running
    playback_controller->StopTimer();

    processingState original = video_display->container.CopyCurrentStateIdx(source_state_ind);
    int number_video_frames = static_cast<int>(original.details.frames_16bit.size());

    processingState noise_suppresion_state = original;
    noise_suppresion_state.details.frames_16bit.clear();

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG availPhysMem = memInfo.ullAvailPhys;
    double available_memory_ratio = double(availPhysMem)/(double(number_video_frames)*16*640*480);

    ImageProcessing ANS;
    lbl_progress_status->setText(QString("Adaptive Noise Suppression..."));
    progress_bar_main->setRange(0,number_video_frames - 1);
    progress_bar_main->setTextVisible(true);
    connect(&ANS, &ImageProcessing::SignalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, &ANS, &ImageProcessing::CancelOperation);
    if(available_memory_ratio >=1.5){
        noise_suppresion_state.details.frames_16bit = ANS.AdaptiveNoiseSuppressionMatrix(relative_start_frame, number_of_frames, shadow_sigma_thresh, original.details, hide_shadow_choice);
    }
    else{
        noise_suppresion_state.details.frames_16bit = ANS.AdaptiveNoiseSuppressionByFrame(relative_start_frame, number_of_frames, shadow_sigma_thresh, original.details, hide_shadow_choice);
    }
    progress_bar_main->setValue(0);
    progress_bar_main->setTextVisible(false);
    lbl_progress_status->setText(QString(""));
    if(noise_suppresion_state.details.frames_16bit.size()>0){
        QString description = "Filter starts at ";
        if (relative_start_frame > 0)
            description += "+";
        lbl_adaptive_noise_suppression_status->setWordWrap(true);
        description += QString::number(relative_start_frame) + " frames and averages " + QString::number(number_of_frames) + " frames";
        lbl_adaptive_noise_suppression_status->setText(description);
        noise_suppresion_state.method = ProcessingMethod::adaptive_noise_suppression;
        noise_suppresion_state.ANS_relative_start_frame = relative_start_frame;
        noise_suppresion_state.ANS_num_frames = number_of_frames;
        noise_suppresion_state.ANS_hide_shadow = hide_shadow_choice;
        noise_suppresion_state.ANS_shadow_threshold = shadow_sigma_thresh;
        noise_suppresion_state.source_state_ID = source_state_ind;
        uint16_t maxVal = std::numeric_limits<int>::min(); // Initialize with the smallest possible int
        for (const auto& row : noise_suppresion_state.details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        noise_suppresion_state.details.max_value = maxVal;
        noise_suppresion_state.state_ID =  video_display->container.processing_states.size() + 1;
        video_display->container.AddProcessingState(noise_suppresion_state);
    }
}

void SirveApp::ExecuteRPCPNoiseSuppression()
{
    int source_state_ind = cmb_processing_states->currentIndex();
    ApplyRPCPNoiseSuppression(source_state_ind);
}

void SirveApp::ApplyRPCPNoiseSuppression(int source_state_ind)
{
    //Pause the video if it's running
    playback_controller->StopTimer();
    processingState original = video_display->container.CopyCurrentStateIdx(source_state_ind);
    int number_video_frames = static_cast<int>(original.details.frames_16bit.size());
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG availPhysMem = memInfo.ullAvailPhys;
    double available_memory_ratio = double(availPhysMem)/(double(number_video_frames)*16*640*480);
    processingState noise_suppresion_state = original;
    noise_suppresion_state.details.frames_16bit.clear();
    if(available_memory_ratio >=1.5){
        ImageProcessing RPCP;
        lbl_progress_status->setText(QString("RPCP Noise Suppression..."));
        progress_bar_main->setRange(0,number_video_frames);
        progress_bar_main->setTextVisible(true);
        connect(&RPCP, &ImageProcessing::SignalProgress, progress_bar_main, &QProgressBar::setValue);
        connect(btn_cancel_operation, &QPushButton::clicked, &RPCP, &ImageProcessing::CancelOperation);
        noise_suppresion_state.details.frames_16bit = RPCP.RPCPNoiseSuppression(original.details);
        lbl_progress_status->setText(QString(""));
        progress_bar_main->setValue(0);
        progress_bar_main->setTextVisible(false);
        if(noise_suppresion_state.details.frames_16bit.size()>0){
            noise_suppresion_state.method = ProcessingMethod::RPCP_noise_suppression;
            noise_suppresion_state.source_state_ID = source_state_ind;
            uint16_t maxVal = std::numeric_limits<int>::min(); // Initialize with the smallest possible int
            for (const auto& row : noise_suppresion_state.details.frames_16bit) {
                maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
            }
            noise_suppresion_state.details.max_value = maxVal;
            noise_suppresion_state.state_ID =  video_display->container.processing_states.size() + 1;
            video_display->container.AddProcessingState(noise_suppresion_state);
        }
    }
    else
    {
        QtHelpers::LaunchMessageBox(QString("Low memory"), "Insufficient memory for this operation. Please select fewer frames.");
    }
}

void SirveApp::ToggleVideoPlaybackOptions(bool input)
{
    btn_fast_forward->setEnabled(input);
    btn_slow_back->setEnabled(input);

    btn_frame_record->setEnabled(input);
    btn_frame_save->setEnabled(input);
    btn_zoom->setEnabled(input);

    slider_video->setEnabled(input);
    btn_play->setEnabled(input);
    btn_pause->setEnabled(input);
    btn_next_frame->setEnabled(input);
    btn_prev_frame->setEnabled(input);
    btn_reverse->setEnabled(input);

    if (!input)
    {
        playback_controller->StopTimer();
        lbl_fps->setText("");
    }
}

void SirveApp::EnableEngineeringPlotOptions()
{
    tab_plots->setCurrentIndex(1);

    rad_linear->setChecked(true);
    rad_linear->setChecked(true);

    cmb_plot_xaxis->clear();
    cmb_plot_xaxis->setEnabled(true);
    cmb_plot_xaxis->addItem(QString("Frames"));
    cmb_plot_xaxis->addItem(QString("Seconds from Midnight"));
    cmb_plot_xaxis->addItem(QString("Seconds from Epoch"));
    cmb_plot_xaxis->setCurrentIndex(0);

    cmb_plot_yaxis->clear();
    cmb_plot_yaxis->setEnabled(true);
    cmb_plot_yaxis->addItem(QString("Irradiance"));
    cmb_plot_yaxis->addItem(QString("Azimuth"));
    cmb_plot_yaxis->addItem(QString("Elevation"));
    cmb_plot_yaxis->addItem(QString("IFOV - X"));
    cmb_plot_yaxis->addItem(QString("IFOV - Y"));
    cmb_plot_yaxis->addItem(QString("Boresight Azimuth"));
    cmb_plot_yaxis->addItem(QString("Boresight Elevation"));
    cmb_plot_yaxis->setCurrentIndex(0);


    // ------------------------------------------ Set Plot Options ------------------------------------------

    rad_decimal->setEnabled(true);
    rad_scientific->setEnabled(true);

    rad_log->setEnabled(true);
    rad_linear->setEnabled(true);

    btn_plot_menu->setEnabled(true);
    btn_save_plot->setEnabled(true);
}

void SirveApp::UpdateEpochString(QString new_epoch_string)
{

    QString out = "Applied Epoch: ";
    out = out +new_epoch_string;
    lbl_current_epoch->setText(out);

}

void SirveApp::DisplayOriginalEpoch(QString new_epoch_string)
{
    QString out = "Original Epoch: ";
    out = out + new_epoch_string;
    lbl_current_epoch->setToolTip(out);
}

QString SirveApp::CreateEpochString(std::vector<double> new_epoch) {

    QString out = "";

    int number;
    int length = static_cast<int>(new_epoch.size());
    for (int i = 0; i < length; i++)
    {
        if (i == 0)
        {
            out = out + QString::number(new_epoch[i]);
        }
        else {
            number = new_epoch[i];
            if (number < 10)
            {
                out = out + "0" + QString::number(new_epoch[i]);
            }
            else
            {
                out = out + QString::number(new_epoch[i]);
            }

        }

        // Add date/time separator
        if (i < 2)
        {
            out = out + "/";
        }
        if (i == 2)
        {
            out = out + " ";
        }
        if (i > 2 && i < 5)
        {
            out = out + ":";
        }

    }

    return out;
}

bool SirveApp::VerifyFrameSelection(int min_frame, int max_frame)
{
    if (min_frame < 1)
    {
        QtHelpers::LaunchMessageBox(QString("Non-Numeric Data"), "Invalid data entered for the start frame");
        return false;
    }

    if (max_frame < 1)
    {
        QtHelpers::LaunchMessageBox(QString("Non-Numeric Data"), "Invalid data entered for the end frame");
        return false;
    }

    if (min_frame > max_frame)
    {
        QtHelpers::LaunchMessageBox(QString("Bad Data Entered"), "Start frame is greater than the end frame");
        return false;
    }

    int frame_stop = data_plots->full_plot_xmax + 1;

    if (max_frame > frame_stop)
    {
        QtHelpers::LaunchMessageBox(QString("Outside of Data Range"), "Data must be within valid range (1-" + QString::number(frame_stop) + ")");
        return false;
    }

    return true;
}

void SirveApp::HandleFrameChange()
{
    UpdateGlobalFrameVector();
}

void SirveApp::HandleFrameNumberChange(unsigned int new_frame_number)
{
    video_display->ViewFrame(new_frame_number);
    UpdateGlobalFrameVector();
}

void SirveApp::UpdateGlobalFrameVector()
{
    std::vector<double> original_frame_vector = {video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[video_display->counter].begin(),
        video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[video_display->counter].end()};

    //Convert current frame to armadillo matrix
    arma::vec image_vector(original_frame_vector);

    int image_max_value = image_vector.max();
    if (!rad_scale_by_frame->isChecked()){
        image_max_value = video_display->container.processing_states[video_display->container.current_idx].details.max_value;
    }

    int image_min_value = image_vector.min();

    lbl_min_scale_value->setText("Low: " + QString::number(image_min_value));
    lbl_max_scale_value->setText("High: " + QString::number(image_max_value));
    image_vector = image_vector - image_min_value;
    image_vector = image_vector / image_max_value;

    if (chk_auto_lift_gain->isChecked())
    {
        double lift_sigma = txt_lift_sigma->text().toDouble();
        double gain_sigma = txt_gain_sigma->text().toDouble();
        double sigma = arma::stddev(image_vector);
        double meanVal = arma::mean(image_vector);
        double lift = meanVal - (lift_sigma * sigma);
        double gain = meanVal + (gain_sigma * sigma);

        lift = std::max(lift, 0.);
        gain = std::min(gain, 1.);
        SetLiftAndGain(lift, gain);
    }

    double lift = lbl_lift_value->text().toDouble();
    double gain = lbl_gain_value->text().toDouble();

    int max_val = std::round(image_max_value* gain);
    QString max_val_info = "Dark Set Pt: " + QString::number(max_val);
    lbl_max_count_val->setText(max_val_info);

    int min_val = std::round(image_max_value * lift);
    QString min_val_info = "Light Set Pt:" + QString::number(min_val);
    lbl_min_count_val->setText(min_val_info);

    color_map_display->set_color_map(video_colors.maps[cmb_color_maps->currentIndex()].colors,lift,gain);
    histogram_plot->UpdateHistogramAbsPlot(image_vector, lift, gain);

    // Correct image based on min/max value inputs
    ColorCorrection::UpdateColor(image_vector, lift, gain);

    histogram_plot->UpdateHistogramRelPlot(image_vector);

    image_vector = image_vector * 255;

    std::vector<double>out_vector = arma::conv_to<std::vector<double>>::from(image_vector);
    std::vector<uint8_t> display_ready_converted_values = {out_vector.begin(), out_vector.end()};
    std::vector<std::vector<int>> offsets;
    if (video_display->container.processing_states[video_display->container.current_idx].offsets.size()>0){
        offsets = video_display->container.processing_states[video_display->container.current_idx].offsets;
    }

    video_display->UpdateFrameVector(original_frame_vector, display_ready_converted_values, offsets);
}
