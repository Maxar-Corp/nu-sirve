#include "SirveApp.h"

SirveApp::SirveApp(QWidget *parent)
    : QMainWindow(parent)
{
    //GetAboutTimeStamp();

    config_values = configReaderWriter::ExtractWorkspaceConfigValues();

    workspace = new Workspace(config_values.workspace_folder);

    file_processor = new ProcessFile();

    // establish object that will hold video and connect it to the playback thread
    color_map_display = new ColorMapDisplay(video_colors.maps[0].colors, 0, 1);
    video_display = new VideoDisplay(video_colors.maps[0].colors);
    // video_display->moveToThread(&thread_video);

    histogram_plot = new HistogramLinePlot();

    SetupUi();
    popout_video = new PopoutDialog();
    popout_histogram = new PopoutDialog();
    popout_engineering = new PopoutDialog();

    //---------------------------------------------------------------------------

    // establish object to control playback timer and move to a new thread
    playback_controller = new FramePlayer(1);
    // playback_controller->moveToThread(&thread_timer);

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

    osmDataLoaded = false;
    UpdateGuiPostDataLoad(osmDataLoaded);

    connect(this->file_processor, &ProcessFile::forwardProgress, this->progress_bar_main, &QProgressBar::setValue);
}

SirveApp::~SirveApp() {
    delete video_display;
    delete playback_controller;
    delete eng_data;
    delete data_plots;
    // thread_video.terminate();
    // thread_timer.terminate();
}

void SirveApp::SetupUi() {

    QHBoxLayout* main_layout = new QHBoxLayout();
    QVBoxLayout* main_layout_col1 = new QVBoxLayout();
    QVBoxLayout* main_layout_col2 = new QVBoxLayout();
    QVBoxLayout* main_layout_col3 = new QVBoxLayout();

    // Define main widgets in UI
    tab_menu = new QTabWidget();
    frame_video_player = new QFrame();
    tab_plots = new QTabWidget();
    tab_plots->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Preferred);
    tab_plots->setMinimumWidth(500);

    // tab_menu->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    // ------------------------------------------------------------------------
    // Define complete tab widget
    // ------------------------------------------------------------------------
    tab_menu->addTab(SetupProcessingTab(), "Processing");
    tab_menu->addTab(SetupColorCorrectionTab(), "Color/Overlays");
    tab_menu->addTab(SetupTracksTab(), "Tracks");

	QSizePolicy fixed_width_video;
    fixed_width_video.setVerticalPolicy(QSizePolicy::Expanding);
	frame_video_player->setSizePolicy(fixed_width_video);
    frame_video_player->setMinimumWidth(725);
    frame_video_player->setObjectName("frame_video_player");

    // ------------------------------------------------------------------------
    // Adds all elements to main UI

    SetupVideoFrame();
    SetupPlotFrame();

    lbl_max_frames = new QLabel("Available Frames: ");
	QFormLayout *form_start_frame = new QFormLayout;
	QFormLayout *form_stop_frame = new QFormLayout;
	txt_start_frame = new QLineEdit();
	txt_start_frame->setAlignment(Qt::AlignVCenter);
    txt_start_frame->setFixedWidth(60);
	txt_stop_frame = new QLineEdit();
	txt_stop_frame->setAlignment(Qt::AlignVCenter);
    txt_stop_frame->setFixedWidth(60);
    form_start_frame->addRow(tr("&Start Frame:"),txt_start_frame);
    form_start_frame->setFormAlignment(Qt::AlignHCenter | Qt::AlignCenter);
    form_stop_frame->addRow(tr("&Stop Frame:"),txt_stop_frame);
    form_stop_frame->setFormAlignment(Qt::AlignHCenter | Qt::AlignCenter);

	btn_get_frames = new QPushButton("Load Frames");
    grpbox_load_frames_area = new QGroupBox();
	grpbox_load_frames_area->setObjectName("grpbox_load_frames_area");
	QHBoxLayout *hlayout_load_frames_area = new QHBoxLayout(grpbox_load_frames_area);
	hlayout_load_frames_area->addWidget(lbl_max_frames);
	hlayout_load_frames_area->addLayout(form_start_frame);
	hlayout_load_frames_area->addLayout(form_stop_frame);
	hlayout_load_frames_area->addWidget(btn_get_frames);
    hlayout_load_frames_area->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout
    grpbox_load_frames_area->setFixedHeight(50);

    grpbox_progressbar_area = new QGroupBox();
    grpbox_progressbar_area->setObjectName("grpbox_progressbar_area");
	QHBoxLayout *hlayout_progressbar_area = new QHBoxLayout();
	grpbox_progressbar_area->setLayout(hlayout_progressbar_area);
    grpbox_progressbar_area->setEnabled(false);
	progress_bar_main = new QProgressBar();
    progress_bar_main->setFixedWidth(300);
	btn_cancel_operation = new QPushButton("Cancel");
	btn_cancel_operation->setFixedWidth(75);
	hlayout_progressbar_area->addWidget(progress_bar_main);
	hlayout_progressbar_area->addWidget(btn_cancel_operation);

	grpbox_status_area = new QGroupBox("State Control");
	grpbox_status_area->setObjectName("grpbox_status_area");
    grpbox_status_area->setFixedHeight(200);
	QVBoxLayout *vlayout_status_area = new QVBoxLayout();
	grpbox_status_area->setLayout(vlayout_status_area);
	cmb_processing_states = new QComboBox();
	btn_undo_step = new QPushButton("Undo One Step");
	btn_undo_step->setFixedWidth(110);
    btn_delete_state = new QPushButton("Delete State");
    connect(btn_delete_state, &QPushButton::clicked, this, &SirveApp::DeleteState);
    lbl_processing_description = new QLabel("");
	lbl_processing_description->setWordWrap(true);
    scrollarea_processing_description = new QScrollArea();
    scrollarea_processing_description->setWidgetResizable(true);
    scrollarea_processing_description->setWidget(lbl_processing_description);
    scrollarea_processing_description->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollarea_processing_description->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QVBoxLayout *vlayout_processing_description = new QVBoxLayout(scrollarea_processing_description);
    vlayout_processing_description->addWidget(lbl_processing_description);
    QSpacerItem * d_bottom_vertical_spacer = new QSpacerItem(100, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vlayout_processing_description->addItem(d_bottom_vertical_spacer);
    vlayout_status_area->addWidget(scrollarea_processing_description);
    QFormLayout *form_processing_state = new QFormLayout;
    form_processing_state->addRow(tr("&Processing State:"),cmb_processing_states);
    form_processing_state->setFormAlignment(Qt::AlignHCenter | Qt::AlignCenter);
    vlayout_status_area->addLayout(form_processing_state);
    QHBoxLayout *hlayout_processing_state_buttons = new QHBoxLayout();
    hlayout_processing_state_buttons->addWidget(btn_delete_state);
	hlayout_processing_state_buttons->addWidget(btn_undo_step);
    hlayout_processing_state_buttons->insertStretch(-1,0);
    vlayout_status_area->addLayout(hlayout_processing_state_buttons);
	grpbox_status_area->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    main_layout_col1->addWidget(grpbox_load_frames_area);
    main_layout_col1->addWidget(tab_menu);
    main_layout_col1->addWidget(grpbox_status_area);
    main_layout_col2->addWidget(frame_video_player);
    main_layout_col3->addWidget(tab_plots);
    main_layout->addLayout(main_layout_col1);
    main_layout->addLayout(main_layout_col2);
    main_layout->addLayout(main_layout_col3);
    // main_layout->insertStretch(-1,0);

    QFrame* frame_main = new QFrame();
    frame_main->setLayout(main_layout);

    // ------------------------------------------------------------------------
    // initialize ui elements

    tab_menu->setCurrentIndex(0);

    tab_menu->setTabEnabled(0, false);
    tab_menu->setTabEnabled(1, false);
    tab_menu->setTabEnabled(2, false);

    tab_menu->tabBar()->hide();

    txt_start_frame->setEnabled(false);
    txt_stop_frame->setEnabled(false);
    btn_get_frames->setEnabled(false);

    dt_epoch->setEnabled(false);
    btn_apply_epoch->setEnabled(false);

    btn_delete_state->setEnabled(false);
    btn_undo_step->setEnabled(false);

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

    status_bar = this->statusBar();
    lbl_file_name = new QLabel("OSM File Name:");
    lbl_file_name->setFont(QFont("Arial", 8, QFont::Bold));
	lbl_loaded_frames = new QLabel("Loaded Frames: ");
    lbl_loaded_frames->setFont(QFont("Arial", 8, QFont::Bold));
	lbl_status_start_frame = new QLabel("Start Frame:");
    lbl_status_start_frame->setFont(QFont("Arial", 8, QFont::Bold));
	lbl_status_stop_frame = new QLabel("Stop Frame:");
    lbl_status_stop_frame->setFont(QFont("Arial", 8, QFont::Bold));
    lbl_current_workspace_folder = new QLabel("Workspace Folder:");
    lbl_current_workspace_folder->setFont(QFont("Arial", 8, QFont::Bold));
    lbl_current_workspace_folder_field = new QLabel(config_values.workspace_folder);
    lbl_current_workspace_folder->setWordWrap(false);
	lbl_workspace_name = new QLabel("Workspace File:");
    lbl_workspace_name->setFont(QFont("Arial", 8, QFont::Bold));
    lbl_workspace_name_field = new QLabel("");
    lbl_workspace_name_field->setFont(QFont("Arial", 8, QFont::Bold));
    lbl_progress_status = new QLabel("");
    lbl_progress_status->setFixedWidth(300);
    lbl_progress_status->setWordWrap(true);
    QGroupBox *grpbox_status_bar = new QGroupBox();
    grpbox_status_bar->setMinimumWidth(1050);
    QHBoxLayout * hlayout_status_bar1 = new QHBoxLayout();
    QHBoxLayout * hlayout_status_bar2 = new QHBoxLayout();
    QGroupBox *grpbox_status_permanent = new QGroupBox();
    grpbox_status_permanent->setMinimumWidth(650);
    QHBoxLayout * hlayout_status_permanent = new QHBoxLayout();
    grpbox_status_permanent->setLayout(hlayout_status_permanent);

    QSpacerItem *hspacer_item10 = new QSpacerItem(10,1);
    QVBoxLayout * vlayout_status_lbl = new QVBoxLayout();

    hlayout_status_bar1->addWidget(lbl_file_name);
    hlayout_status_bar1->addItem(hspacer_item10);
    hlayout_status_bar1->addWidget(lbl_loaded_frames);
    hlayout_status_bar1->addItem(hspacer_item10);
    hlayout_status_bar1->addWidget(lbl_status_start_frame);
    hlayout_status_bar1->addItem(hspacer_item10);
    hlayout_status_bar1->addWidget(lbl_status_stop_frame);
    hlayout_status_bar1->insertStretch(-1,0);
    hlayout_status_bar2->addWidget(lbl_current_workspace_folder);
    hlayout_status_bar2->addWidget(lbl_current_workspace_folder_field);
    hlayout_status_bar2->addItem(hspacer_item10);
    hlayout_status_bar2->addWidget(lbl_workspace_name);
    hlayout_status_bar2->addWidget(lbl_workspace_name_field);
    hlayout_status_bar2->insertStretch(-1,0);
    vlayout_status_lbl->addLayout(hlayout_status_bar1);
    vlayout_status_lbl->addLayout(hlayout_status_bar2);
    grpbox_status_bar->setLayout(vlayout_status_lbl);

    status_bar->addWidget(grpbox_status_bar);
    hlayout_status_permanent->addWidget(lbl_progress_status);
    hlayout_status_permanent->addItem(hspacer_item10);
    hlayout_status_permanent->addWidget(grpbox_progressbar_area);
    hlayout_status_permanent->addItem(hspacer_item10);
    hlayout_status_permanent->insertStretch(-1,0);
    status_bar->addPermanentWidget(grpbox_status_permanent,0);

    this->show();
}

QWidget* SirveApp::SetupColorCorrectionTab()
{
    
    color_map_display->setMinimumHeight(20);
    QWidget* widget_tab_color = new QWidget(tab_menu);
    QDoubleValidator* ensure_double = new QDoubleValidator(widget_tab_color);
    QVBoxLayout* vlayout_tab_color = new QVBoxLayout(widget_tab_color);

    grpbox_image_controls = new QGroupBox("Image Controls");
    grpbox_image_controls->setStyleSheet(bold_large_styleSheet);

    lbl_lift_value = new QLabel("0.0");
    lbl_lift_value->setFixedWidth(50);
    lbl_gain_value = new QLabel("1.0");
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
    slider_gain->setTickPosition(QSlider::TicksAbove);
    slider_gain->setTickInterval(100);
    slider_gain->setEnabled(false);

    txt_lift_sigma = new QLineEdit("3");
    txt_lift_sigma->setValidator(ensure_double);
    txt_lift_sigma->setFixedWidth(50);

    txt_gain_sigma = new QLineEdit("3");
    txt_gain_sigma->setValidator(ensure_double);
    txt_gain_sigma->setFixedWidth(50);

    lbl_max_scale_value = new QLabel("High");
    lbl_max_scale_value->setStyleSheet("color:rgb(81,72,65);");

    chk_auto_lift_gain = new QCheckBox("Auto\nLift/Gain");

    btn_reset_color_correction = new QPushButton("Reset Set Points");

    lbl_min_count_val = new QLabel("Dark Set Pt:");
    lbl_min_count_val->setStyleSheet("color: black; background-color: rgba(245, 200, 125, 255); font-weight: bold;");
    lbl_min_count_val->setFixedWidth(130);
    lbl_min_count_val->setFixedHeight(30);
    lbl_max_count_val = new QLabel("Light Set Pt:");
    lbl_max_count_val->setStyleSheet("color: black; background-color: rgba(245, 200, 125, 255); font-weight: bold;");
    lbl_max_count_val->setFixedHeight(30);
    
    lbl_min_scale_value = new QLabel("Low");
    lbl_min_scale_value->setFixedWidth(125);
    lbl_min_scale_value->setStyleSheet("color:rgb(81,72,65);");

    cmb_color_maps = new QComboBox();
    int number_maps = video_colors.maps.size();
    for (int i = 0; i < number_maps; i++)
        cmb_color_maps->addItem(video_colors.maps[i].name);
    cmb_color_maps->setFixedWidth(150);

    rad_scale_by_frame = new QRadioButton("Scale by\nframe maximum");
    rad_scale_by_frame->setAutoExclusive(true);
    rad_scale_by_frame->setChecked(true);
    rad_scale_by_cube = new QRadioButton("Scale by\ncube maximum");
    rad_scale_by_cube->setAutoExclusive(true);
    rad_scale_by_cube->setChecked(false);
    connect(rad_scale_by_frame, &QCheckBox::toggled, this, &SirveApp::UpdateGlobalFrameVector);
    connect(rad_scale_by_cube, &QCheckBox::toggled, this, &SirveApp::UpdateGlobalFrameVector);

    grpbox_auto_lift_gain = new QGroupBox();
    QHBoxLayout *hlayout_auto_lift_gain = new QHBoxLayout(grpbox_auto_lift_gain);
    grpbox_auto_lift_gain->setObjectName("grpbox_auto_lift_gain");
    grpbox_auto_lift_gain->setStyleSheet("#grpbox_auto_lift_gain {border: 0px solid gray; border-width: 0px;}");

    QFormLayout *form_auto_lift_gain = new QFormLayout;
    form_auto_lift_gain->addRow(tr("&Min (sigma below mean)"),txt_lift_sigma);
    form_auto_lift_gain->addRow(tr("&Max (sigma above mean)"),txt_gain_sigma);
    hlayout_auto_lift_gain->addLayout(form_auto_lift_gain);

    QHBoxLayout *hlayout_auto_gain_group = new QHBoxLayout;
    hlayout_auto_gain_group->addWidget(chk_auto_lift_gain);
    hlayout_auto_gain_group->addWidget(grpbox_auto_lift_gain);
    hlayout_auto_gain_group->insertStretch(0,0);
    hlayout_auto_gain_group->insertStretch(-1,0);

    QFormLayout *form_slider_lift_control = new QFormLayout;
    form_slider_lift_control->addRow(tr("&Dark\nSet Point"),slider_lift);
    QFormLayout *form_slider_gain_control = new QFormLayout;
    form_slider_gain_control->addRow(tr("&Light\nSet Point"),slider_gain);
    QVBoxLayout *vlayout_scale_sliders = new QVBoxLayout;
    QHBoxLayout *hlayout_lift_slider = new QHBoxLayout;
    QHBoxLayout *hlayout_gain_slider = new QHBoxLayout;

    vlayout_scale_sliders->addLayout(hlayout_lift_slider);
    vlayout_scale_sliders->addLayout(hlayout_gain_slider);

    hlayout_lift_slider->addLayout(form_slider_lift_control);
    hlayout_lift_slider->addWidget(lbl_lift_value);

    hlayout_gain_slider->addLayout(form_slider_gain_control);
    hlayout_gain_slider->addWidget(lbl_gain_value);

    QGroupBox *grpbox_scale_options = new QGroupBox();
    QHBoxLayout *hlayout_grpbox_scale_options = new QHBoxLayout(grpbox_scale_options);
    grpbox_scale_options->setObjectName("grpbox_scale_options");
    grpbox_scale_options->setStyleSheet("#grpbox_scale_options {border: 0px solid gray; border-width: 0px;}");
    hlayout_grpbox_scale_options->addWidget(rad_scale_by_frame);
    hlayout_grpbox_scale_options->addWidget(rad_scale_by_cube);
    hlayout_grpbox_scale_options->insertStretch(-1,0);
    hlayout_grpbox_scale_options->insertStretch(0,0);
    
    QFormLayout *form_colormap_control = new QFormLayout;
    form_colormap_control->addRow(tr("&Set Colormap"),cmb_color_maps);

    QHBoxLayout *hlayout_slider_controls = new QHBoxLayout;
    hlayout_slider_controls->addLayout(vlayout_scale_sliders);

    QHBoxLayout *hlayout_colormap_controls = new QHBoxLayout;
    hlayout_colormap_controls->addWidget(lbl_min_scale_value);
    hlayout_colormap_controls->addLayout(form_colormap_control);
    hlayout_colormap_controls->insertStretch(2,0);
    hlayout_colormap_controls->addWidget(lbl_max_scale_value);

    QHBoxLayout *hlayout_colormap_bar_row2 = new QHBoxLayout;
    hlayout_colormap_bar_row2->addWidget(lbl_min_count_val);
    hlayout_colormap_bar_row2->addWidget(btn_reset_color_correction);
    hlayout_colormap_bar_row2->addWidget(lbl_max_count_val);

    QVBoxLayout *vlayout_image_controls = new QVBoxLayout(grpbox_image_controls);
    vlayout_image_controls->addLayout(hlayout_auto_gain_group);
    vlayout_image_controls->addLayout(hlayout_slider_controls);
    vlayout_image_controls->addWidget(grpbox_scale_options);
    vlayout_image_controls->addLayout(hlayout_colormap_controls);
    vlayout_image_controls->addWidget(color_map_display);
    vlayout_image_controls->addLayout(hlayout_colormap_bar_row2);
    vlayout_image_controls->insertStretch(-1,0);

    vlayout_tab_color->addWidget(grpbox_image_controls);

    grpbox_overlay_controls = new QGroupBox("Overlay Controls");
    grpbox_overlay_controls->setStyleSheet(bold_large_styleSheet);
    QHBoxLayout* hlayout_overlay_controls = new QHBoxLayout(grpbox_overlay_controls);
    QVBoxLayout* vlayout_overlay_controls_col1 = new QVBoxLayout;
    QVBoxLayout* vlayout_overlay_controls_col2 = new QVBoxLayout;

    video_display->ToggleOsmTracks(true);

    chk_sensor_track_data = new QCheckBox("Sensor Info");

    chk_show_time = new QCheckBox("Zulu Time");

    btn_change_banner_text = new QPushButton("Change Banner Text");

    btn_add_annotations = new QPushButton("Add/Edit Annotations");

    QStringList colors = ColorScheme::get_track_colors();
    cmb_text_color = new QComboBox();
    cmb_text_color->addItems(colors);
    QFormLayout *form_text_color = new QFormLayout;
    form_text_color->addRow(tr("&Text Color"),cmb_text_color);
    QSpacerItem *vspacer_item10 = new QSpacerItem(1,10);
    vlayout_overlay_controls_col1->addItem(vspacer_item10);
    vlayout_overlay_controls_col1->addLayout(form_text_color);
    vlayout_overlay_controls_col1->addWidget(chk_sensor_track_data);
    vlayout_overlay_controls_col1->addWidget(chk_show_time);
    vlayout_overlay_controls_col2->addWidget(btn_change_banner_text);
    vlayout_overlay_controls_col2->addWidget(btn_add_annotations);
    hlayout_overlay_controls->addLayout(vlayout_overlay_controls_col1);
    hlayout_overlay_controls->addLayout(vlayout_overlay_controls_col2);
    hlayout_overlay_controls->insertStretch(-1,0);
    hlayout_overlay_controls->insertStretch(0,0);

    vlayout_tab_color->addWidget(grpbox_overlay_controls);

    QGroupBox *grpbox_epoch_area = new QGroupBox();
    QHBoxLayout *hlayout_epoch_area = new QHBoxLayout(grpbox_epoch_area);
    QVBoxLayout *vlayout_epoch_area = new QVBoxLayout();

    QLabel* label_date_format = new QLabel("Format is:   YYYY/MM/DD HH:MM:SS");
    
    dt_epoch = new QDateTimeEdit(QDateTime(QDate(2001, 01, 01), QTime(0, 0, 0, 0)));
    dt_epoch->setDisplayFormat("yyyy/MM/dd hh:mm:ss.zzz");
    dt_epoch->setAlignment(Qt::AlignHCenter);

    QFormLayout * form_epoch_layout = new QFormLayout;
    form_epoch_layout->addRow(tr("&Epoch "),dt_epoch);

    lbl_current_epoch = new QLabel("Applied Epoch: ");

    btn_apply_epoch = new QPushButton("Apply Epoch");

    vlayout_epoch_area->addLayout(form_epoch_layout);
    vlayout_epoch_area->addWidget(label_date_format);
    vlayout_epoch_area->addWidget(lbl_current_epoch);
    vlayout_epoch_area->addWidget(btn_apply_epoch);
    hlayout_epoch_area->addLayout(vlayout_epoch_area);
    hlayout_epoch_area->insertStretch(-1,0);
    hlayout_epoch_area->insertStretch(0,0);

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
    grpbox_bad_pixels_correction->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    QVBoxLayout *vlayout_bad_pixels = new QVBoxLayout(grpbox_bad_pixels_correction);
    vlayout_bad_pixels->setAlignment(Qt::AlignCenter);
	lbl_bad_pixel_count = new QLabel("No Bad Pixels Replaced.");
    chk_bad_pixels_from_original = new QCheckBox("Load raw data");
    chk_bad_pixels_from_original->setChecked(true);
    connect(chk_bad_pixels_from_original, &QCheckBox::stateChanged, this, &SirveApp::HandleBadPixelRawToggle);
	cmb_bad_pixels_type = new QComboBox();
	cmb_bad_pixels_type->addItem("All Bad Pixels");
	cmb_bad_pixels_type->addItem("Dead/Bad Scale Only");
	cmb_bad_pixels_type->addItem("Outlier Only");

	cmb_outlier_processing_type = new QComboBox();
	cmb_outlier_processing_type->addItem("Median");
	cmb_outlier_processing_type->addItem("Moving Median");
	connect(cmb_outlier_processing_type, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &SirveApp::handle_outlier_processing_change);

    cmb_outlier_processing_sensitivity = new QComboBox();
	cmb_outlier_processing_sensitivity->addItem("Low 6 sigma");
	cmb_outlier_processing_sensitivity->addItem("Medium 5 sigma");
	cmb_outlier_processing_sensitivity->addItem("High 4 sigma");
	cmb_outlier_processing_sensitivity->addItem("Max 3 sigma");

    btn_replace_bad_pixels = new QPushButton("Replace Bad Pixels");
    btn_replace_bad_pixels->setFixedWidth(200);
    connect(btn_replace_bad_pixels, &QPushButton::clicked, this, &SirveApp::HandleBadPixelReplacement);

    QFormLayout *form_replace_which_pixels_col1 = new QFormLayout;
    form_replace_which_pixels_col1->addRow(tr("&Replace Pixels:"),cmb_bad_pixels_type);
    form_replace_which_pixels_col1->addRow(tr("&Method:"),cmb_outlier_processing_type);
    form_replace_which_pixels_col1->addRow(tr("&Sensitivity:"),cmb_outlier_processing_sensitivity);
    form_replace_which_pixels_col1->setFormAlignment(Qt::AlignHCenter | Qt::AlignCenter);

	txt_bad_pixel_start_frame = new QLineEdit("1");
	txt_bad_pixel_start_frame->setFixedWidth(60);
	txt_bad_pixel_stop_frame = new QLineEdit("500");
	txt_bad_pixel_stop_frame->setFixedWidth(60);
	txt_moving_median_N = new QLineEdit("30");
	txt_moving_median_N->setFixedWidth(60);
	txt_moving_median_N->setEnabled(false);
    txt_moving_median_N->setObjectName("txt_moving_median_N");
    txt_moving_median_N->setStyleSheet("#txt_moving_median_N {background-color:#f0f0f0; color:rgb(75,75,75);}");

    QFormLayout *form_replace_which_pixels_col2 = new QFormLayout;
    form_replace_which_pixels_col2->addRow(tr("&Sample Start:"),txt_bad_pixel_start_frame);
    form_replace_which_pixels_col2->addRow(tr("&Sample Stop:"),txt_bad_pixel_stop_frame);
    form_replace_which_pixels_col2->addRow(tr("&Window Length:"),txt_moving_median_N);
    form_replace_which_pixels_col2->setFormAlignment(Qt::AlignHCenter | Qt::AlignCenter);

    QVBoxLayout *vlayout_bad_pixels_col1 = new QVBoxLayout;
    QVBoxLayout *vlayout_bad_pixels_col2 = new QVBoxLayout;
   
	chk_highlight_bad_pixels = new QCheckBox("Highlight Bad Pixels");
	cmb_bad_pixel_color = new QComboBox();
	cmb_bad_pixel_color->setFixedWidth(100);
	cmb_bad_pixel_color->addItems(colors);
	cmb_bad_pixel_color->setCurrentIndex(2);
	connect(cmb_bad_pixel_color, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &SirveApp::edit_bad_pixel_color);
    QFormLayout *form_highlight_bad_pixels = new QFormLayout;
    form_highlight_bad_pixels->addRow(tr("&Color"),cmb_bad_pixel_color);

    vlayout_bad_pixels_col1->addLayout(form_replace_which_pixels_col1);
    vlayout_bad_pixels_col1->addWidget(chk_highlight_bad_pixels);
    vlayout_bad_pixels_col1->addLayout(form_highlight_bad_pixels);

    vlayout_bad_pixels_col2->addLayout(form_replace_which_pixels_col2);
    vlayout_bad_pixels_col2->insertStretch(1,0);
    vlayout_bad_pixels_col2->addWidget(btn_replace_bad_pixels);
    vlayout_bad_pixels_col2->insertStretch(-1,0);

    vlayout_bad_pixels->addWidget(lbl_bad_pixel_count);
    vlayout_bad_pixels->addWidget(chk_bad_pixels_from_original);
    QHBoxLayout *hlayout_bad_pixels = new QHBoxLayout();
    hlayout_bad_pixels->addLayout(vlayout_bad_pixels_col1);
    hlayout_bad_pixels->addLayout(vlayout_bad_pixels_col2);
    hlayout_bad_pixels->insertStretch(-1,0);
    vlayout_bad_pixels->addLayout(hlayout_bad_pixels);
    vlayout_bad_pixels->insertStretch(-1, 0);
    // ------------------------------------------------------------------------

	grpbox_image_processing = new QGroupBox();
    grpbox_image_processing->setObjectName("grpbox_image_processing");
    stck_noise_suppresssion_methods = new QStackedWidget();
    stck_noise_suppresssion_methods->setObjectName("stck_noise_suppresssion_methods");
    QComboBox *cmb_noise_suppresion = new QComboBox();
    cmb_noise_suppresion->setObjectName("cmb_noise_suppresion");
    cmb_noise_suppresion->addItem("Fixed Median Background Noise Suppression");
    cmb_noise_suppresion->addItem("Rolling Mean Background Noise Suppression");
    cmb_noise_suppresion->addItem("Adaptive Median Background Noise Suppression");
    cmb_noise_suppresion->addItem("RPCP Background Noise Suppression");

    connect(cmb_noise_suppresion, qOverload<int>(&QComboBox::currentIndexChanged), stck_noise_suppresssion_methods, &QStackedWidget::setCurrentIndex);

	QVBoxLayout *vlayout_image_processing = new QVBoxLayout(grpbox_image_processing);

    vlayout_image_processing->addWidget(cmb_noise_suppresion);
    vlayout_image_processing->addWidget(stck_noise_suppresssion_methods);
    vlayout_image_processing->insertStretch(-1,0);
	
    grpbox_FNS_processing = new QGroupBox("");
	lbl_fixed_suppression = new QLabel("No Frames Selected");

    QHBoxLayout *hlayout_fns = new QHBoxLayout(grpbox_FNS_processing);
    QVBoxLayout *vlayout_fns = new QVBoxLayout;

	chk_FNS_external_file = new QCheckBox("External File");
    connect(chk_FNS_external_file, &QCheckBox::stateChanged, this, &SirveApp::HandleExternalFileToggle);
	chk_FNS_external_file->setFixedWidth(150);
	txt_FNS_start_frame = new QLineEdit("1");
    txt_FNS_start_frame->setObjectName("txt_FNS_start_frame");
    txt_FNS_start_frame->setStyleSheet("#txt_FNS_start_frame {background-color:#ffffff; color:rgb(0,0,0);}");
	txt_FNS_start_frame->setFixedWidth(60);
	txt_FNS_stop_frame = new QLineEdit("50");
    txt_FNS_stop_frame->setObjectName("txt_FNS_stop_frame");
	txt_FNS_stop_frame->setFixedWidth(60);
	btn_FNS = new QPushButton("Fixed Median Background Noise Suppression");
	btn_FNS->setFixedWidth(300);

    QFormLayout *form_fns = new QFormLayout;
    form_fns->addRow(tr("&Start:"),txt_FNS_start_frame);
    form_fns->addRow(tr("&Stop:"),txt_FNS_stop_frame);
    form_fns->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    vlayout_fns->addWidget(lbl_fixed_suppression);
    vlayout_fns->addWidget(chk_FNS_external_file);
    vlayout_fns->addLayout(form_fns);
    vlayout_fns->addWidget(btn_FNS);
    vlayout_fns->insertStretch(-1,0);
    hlayout_fns->insertStretch(0,0);
    hlayout_fns->addLayout(vlayout_fns);
    hlayout_fns->insertStretch(-1,0);

	// ------------------------------------------------------------------------
	grpbox_ANS_processing = new QGroupBox("");
    QHBoxLayout *hlayout_ans = new QHBoxLayout(grpbox_ANS_processing);
    QVBoxLayout *vlayout_ans = new QVBoxLayout;
	lbl_adaptive_noise_suppression_status = new QLabel("No Frames Setup");
	txt_ANS_offset_frames = new QLineEdit("0");
	txt_ANS_offset_frames->setFixedWidth(60);
	txt_ANS_number_frames = new QLineEdit("60");
	txt_ANS_number_frames->setFixedWidth(60);
	btn_ANS = new QPushButton("Adaptive Median Noise Suppression");
	btn_ANS->setFixedWidth(300);

    QFormLayout *form_ans = new QFormLayout;
    form_ans->addRow(tr("&Offset:"),txt_ANS_offset_frames);
    form_ans->addRow(tr("&Number:"),txt_ANS_number_frames);
    form_ans->setFormAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    vlayout_ans->addWidget(lbl_adaptive_noise_suppression_status);
    vlayout_ans->addLayout(form_ans);
    vlayout_ans->addWidget(btn_ANS);
    vlayout_ans->insertStretch(-1,0);
    hlayout_ans->insertStretch(0,0);
    hlayout_ans->addLayout(vlayout_ans);
    hlayout_ans->insertStretch(-1,0);

	QGroupBox *grpbox_RPCP_processing = new QGroupBox("");
    grpbox_RPCP_processing->setObjectName("grpbox_RPCP_processing");
	grpbox_RPCP_processing->setFlat(true);
	grpbox_RPCP_processing->setStyleSheet("#grpbox_RPCP_processing {border-width: 0px;}");
	QGridLayout* grid_RPCP_processing = new QGridLayout(grpbox_RPCP_processing);
	btn_RPCP = new QPushButton("RPCP Noise Suppression");
	btn_RPCP->setFixedWidth(300);
	connect(btn_RPCP, &QPushButton::clicked, this, &SirveApp::ExecuteRPCPNoiseSuppression);
	grid_RPCP_processing->addWidget(btn_RPCP, 0, 0, 1, 1);

    QGroupBox *grpbox_accumulator_processing = new QGroupBox("");
    grpbox_accumulator_processing->setObjectName("grpbox_accumulator_processing");
    chk_hide_shadow = new QCheckBox("Hide Shadow");
	chk_hide_shadow->setFixedWidth(110);
	chk_hide_shadow->setChecked(false);
	cmb_shadow_threshold = new QComboBox();
	cmb_shadow_threshold->setFixedWidth(100);
    cmb_shadow_threshold->addItem("6 sigma");
    cmb_shadow_threshold->addItem("5 sigma");
    cmb_shadow_threshold->addItem("4 sigma");
	cmb_shadow_threshold->addItem("3 sigma");
	cmb_shadow_threshold->addItem("2 sigma");
	cmb_shadow_threshold->addItem("1 sigma");
    cmb_shadow_threshold->setCurrentIndex(0);
    QHBoxLayout* hlayout_accumulator_processing = new QHBoxLayout(grpbox_accumulator_processing);
	QVBoxLayout* vlayout_accumulator_processing = new QVBoxLayout;
	btn_accumulator = new QPushButton("Rolling Mean Noise Suppression");
	btn_accumulator->setFixedWidth(300);
	connect(btn_accumulator, &QPushButton::clicked, this, &SirveApp::ExecuteAccumulatorNoiseSuppression);
    txt_accumulator_weight = new QLineEdit("0.5");
	txt_accumulator_weight->setFixedWidth(60);
    txt_accumulator_offset = new QLineEdit("0");
    txt_accumulator_offset->setFixedWidth(60);
    QFormLayout *form_accumulator = new QFormLayout;
    form_accumulator->addRow(tr("&Added Frame Weight:"),txt_accumulator_weight);
    form_accumulator->addRow(tr("&Offset:"),txt_accumulator_offset);
    vlayout_accumulator_processing->addLayout(form_accumulator);
    vlayout_accumulator_processing->addWidget(chk_hide_shadow);
    QFormLayout *form_hide_shadow = new QFormLayout;
    form_hide_shadow->addRow(tr("&Shadow Threshold:"),cmb_shadow_threshold);

    vlayout_accumulator_processing->addLayout(form_hide_shadow);
    vlayout_accumulator_processing->addWidget(btn_accumulator);
    vlayout_accumulator_processing->insertStretch(-1,0);
    hlayout_accumulator_processing->insertStretch(0,0);
    hlayout_accumulator_processing->addLayout(vlayout_accumulator_processing);
    hlayout_accumulator_processing->insertStretch(-1,0);
	// ------------------------------------------------------------------------

	QGroupBox * grpbox_deinterlacing = new QGroupBox("");
    QHBoxLayout * hlayout_deinterlacing = new QHBoxLayout(grpbox_deinterlacing);
	QVBoxLayout * vlayout_deinterlacing = new QVBoxLayout;
	btn_deinterlace = new QPushButton("Deinterlace");
	btn_deinterlace->setFixedWidth(175);
	connect(btn_deinterlace, &QPushButton::clicked, this, &SirveApp::ExecuteDeinterlace);
	btn_deinterlace_current_frame = new QPushButton("Deinterlace Current Frame");
	btn_deinterlace_current_frame->setFixedWidth(175);
	connect(btn_deinterlace_current_frame, &QPushButton::clicked, this, &SirveApp::ExecuteDeinterlaceCurrent);
	chk_deinterlace_confirmation = new QCheckBox("Require Confirmation");
	chk_deinterlace_confirmation->setChecked(true);

    vlayout_deinterlacing->addWidget(btn_deinterlace);
    vlayout_deinterlacing->addWidget(btn_deinterlace_current_frame);
    vlayout_deinterlacing->addWidget(chk_deinterlace_confirmation);
    vlayout_deinterlacing->insertStretch(-1,0);
    vlayout_deinterlacing->insertStretch(0,0);
    hlayout_deinterlacing->insertStretch(0,0);
    hlayout_deinterlacing->addLayout(vlayout_deinterlacing);
    hlayout_deinterlacing->insertStretch(-1,0);

    QToolBox *toolbox_image_processing = new QToolBox();
    toolbox_image_processing->addItem(grpbox_bad_pixels_correction,QString("Bad Pixel Correction"));
    toolbox_image_processing->addItem(grpbox_image_processing,QString("Noise Suppression"));
    toolbox_image_processing->addItem(grpbox_deinterlacing,QString("Deinterlacing"));
    stck_noise_suppresssion_methods->addWidget(grpbox_FNS_processing);
    stck_noise_suppresssion_methods->addWidget(grpbox_accumulator_processing);
    stck_noise_suppresssion_methods->addWidget(grpbox_ANS_processing);
    stck_noise_suppresssion_methods->addWidget(grpbox_RPCP_processing);

    // ------------------------------------------------------------------------
    grpbox_image_shift = new QGroupBox();
    QHBoxLayout *hlayout_image_shift = new QHBoxLayout(grpbox_image_shift);
    QVBoxLayout *vlayout_image_shift = new QVBoxLayout;
    vlayout_image_shift->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);

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

    QHBoxLayout *hlayout_centering_btns = new QHBoxLayout;
    hlayout_centering_btns->addWidget(btn_center_on_tracks);
    hlayout_centering_btns->addWidget(btn_center_on_brightest);

    QFormLayout *form_image_shift = new QFormLayout;
    form_image_shift->addRow(tr("&Centering Priority:"),cmb_track_centering_priority);
    form_image_shift->addRow(tr("&OSM Track ID:"),cmb_OSM_track_IDs);
    form_image_shift->addRow(tr("&Manual Track ID:"),cmb_manual_track_IDs);
    form_image_shift->setFormAlignment(Qt::AlignHCenter | Qt::AlignCenter);
    hlayout_image_shift->addLayout(vlayout_image_shift);
    vlayout_image_shift->addLayout(form_image_shift);
    vlayout_image_shift->addLayout(hlayout_centering_btns);
    hlayout_centering_btns->insertStretch(-1,0);

    QVBoxLayout *vlayout_frame_stacking = new QVBoxLayout;
    vlayout_frame_stacking->setAlignment(Qt::AlignRight);
    QFormLayout *form_frame_stack = new QFormLayout;
    form_frame_stack->addRow(tr("&Number of Frames:"), txt_frame_stack_Nframes);
    form_frame_stack->setFormAlignment(Qt::AlignHCenter | Qt::AlignCenter);
    vlayout_frame_stacking->addLayout(form_frame_stack);
    vlayout_frame_stacking->addWidget(btn_frame_stack);
    vlayout_frame_stacking->insertStretch(-1,0);

    hlayout_image_shift->insertStretch(1,0);
    hlayout_image_shift->addLayout(vlayout_frame_stacking);
    hlayout_image_shift->insertStretch(-1,0);
    vlayout_image_shift->insertStretch(-1,0);

	// // ------------------------------------------------------------------------

	toolbox_image_processing->addItem(grpbox_image_shift,QString("Image Stabilization"));
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
    btn_create_track->setFixedWidth(100);
    btn_finish_create_track = new QPushButton("Finish");
    btn_finish_create_track->setHidden(true);
    btn_finish_create_track->setFixedWidth(100);
    btn_import_tracks = new QPushButton("Import Tracks");
    btn_import_tracks->setFixedWidth(100);
    QVBoxLayout* vlayout_workspace = new QVBoxLayout();
    QHBoxLayout *hlayout_workspace = new QHBoxLayout();
    hlayout_workspace->setAlignment(Qt::AlignLeft);
    vlayout_workspace->addWidget(lbl_track, Qt::AlignLeft);
    vlayout_workspace->addWidget(lbl_create_track_message);
    // hlayout_workspace->insertStretch(0,0);
    // hlayout_workspace->insertStretch(-1,0);
    hlayout_workspace->addWidget(btn_create_track,Qt::AlignLeft);
    hlayout_workspace->addWidget(btn_finish_create_track,Qt::AlignLeft);
    hlayout_workspace->addWidget(btn_import_tracks,Qt::AlignLeft);
    vlayout_workspace->addLayout(hlayout_workspace);

    tm_widget = new TrackManagementWidget(widget_tab_tracks);
    QScrollArea *track_management_scroll_area = new QScrollArea();
    track_management_scroll_area->setMinimumHeight(275);
    track_management_scroll_area->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    track_management_scroll_area->setWidgetResizable(true);
    track_management_scroll_area->setWidget(tm_widget);
    track_management_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vlayout_workspace->addWidget(track_management_scroll_area);
    // vlayout_workspace->insertStretch(3,0);
    vlayout_workspace->insertStretch(-1,0);
    vlayout_workspace->insertStretch(0,0);
	QStringList colors = ColorScheme::get_track_colors();
    chk_show_OSM_tracks = new QCheckBox("OSM Tracks");
    chk_show_OSM_tracks->setChecked(true);
    cmb_OSM_track_color = new QComboBox();
    cmb_OSM_track_color->addItems(colors);
    cmb_OSM_track_color->setEnabled(true);
    cmb_OSM_track_color->setCurrentIndex(4);

    QGroupBox * grpbox_OSM_track_display = new QGroupBox;
    QHBoxLayout *hlayout_OSM_track_display = new QHBoxLayout(grpbox_OSM_track_display);
    hlayout_OSM_track_display->addWidget(chk_show_OSM_tracks);
    hlayout_OSM_track_display->addWidget(cmb_OSM_track_color);
    hlayout_OSM_track_display->insertStretch(-1,0);
    hlayout_OSM_track_display->insertStretch(0,0);

    QGroupBox * grpbox_autotrack = new QGroupBox("Auto Tracking");
    QVBoxLayout *vlayout_auto_track_control = new QVBoxLayout(grpbox_autotrack);
    QHBoxLayout *hlayout_auto_track_control = new QHBoxLayout;
    btn_auto_track_target = new QPushButton("Auto Tracker");
    connect(btn_auto_track_target, &QPushButton::clicked, this, &SirveApp::ExecuteAutoTracking);
    txt_auto_track_start_frame = new QLineEdit("1");
    txt_auto_track_start_frame->setFixedWidth(60);
    txt_auto_track_stop_frame = new QLineEdit("");
    txt_auto_track_stop_frame->setFixedWidth(60);
    QFormLayout *form_auto_track_frame_limits = new QFormLayout;
    form_auto_track_frame_limits->addRow(tr("&Frame Start:"), txt_auto_track_start_frame);
    form_auto_track_frame_limits->addRow(tr("&Frame Stop:"), txt_auto_track_stop_frame);
    cmb_autotrack_threshold = new QComboBox;
    cmb_autotrack_threshold->addItem("6 Sigma");
    cmb_autotrack_threshold->addItem("5 Sigma");
    cmb_autotrack_threshold->addItem("4 Sigma");
    cmb_autotrack_threshold->addItem("3 Sigma");
    cmb_autotrack_threshold->addItem("2 Sigma");
    cmb_autotrack_threshold->addItem("1 Sigma");
    cmb_autotrack_threshold->addItem("0 Sigma");
    cmb_autotrack_threshold->setCurrentIndex(3);
    connect(cmb_autotrack_threshold, qOverload<int>(&QComboBox::currentIndexChanged), video_display, &VideoDisplay::GetThreshold);
    form_auto_track_frame_limits->addRow(tr("&Threshold:"), cmb_autotrack_threshold);
    QVBoxLayout *vlayout_auto_track = new QVBoxLayout;
    vlayout_auto_track->addLayout( form_auto_track_frame_limits);
    vlayout_auto_track->addWidget(btn_auto_track_target);

    QGroupBox* grpbox_autotrack_filters = new QGroupBox("Pre Filter Options");
    QGridLayout *grid_autotrack_filters = new QGridLayout(grpbox_autotrack_filters);
    rad_autotrack_filter_none = new QRadioButton("None");
    rad_autotrack_filter_none->setChecked(true);
    rad_autotrack_filter_gaussian = new QRadioButton("Gaussian");
    rad_autotrack_filter_median = new QRadioButton("Median");
    rad_autotrack_filter_nlmeans = new QRadioButton("Non Local Means");
    QSpacerItem *vspacer_item20 = new QSpacerItem(10,10,QSizePolicy::Expanding,QSizePolicy::Minimum);
    
    QButtonGroup * buttongrp_autotrack_filters = new QButtonGroup();
    buttongrp_autotrack_filters->addButton(rad_autotrack_filter_none);
    buttongrp_autotrack_filters->addButton(rad_autotrack_filter_gaussian);
    buttongrp_autotrack_filters->addButton(rad_autotrack_filter_median);
    buttongrp_autotrack_filters->addButton(rad_autotrack_filter_nlmeans);
  
    grid_autotrack_filters->addWidget(rad_autotrack_filter_none,1,0);
    grid_autotrack_filters->addWidget(rad_autotrack_filter_gaussian,1,1);
    grid_autotrack_filters->addWidget(rad_autotrack_filter_median,2,0);
    grid_autotrack_filters->addWidget(rad_autotrack_filter_nlmeans,2,1);
    grid_autotrack_filters->addItem(vspacer_item20,0,0,1,2);

    QGroupBox* grpbox_autotrack_feature = new QGroupBox("Feature Options");
    QGridLayout *grid_autotrack_feature = new QGridLayout(grpbox_autotrack_feature);
    rad_autotrack_feature_weighted_centroid = new QRadioButton("Weighted Centroid");
    rad_autotrack_feature_weighted_centroid->setChecked(true);
    rad_autotrack_feature_centroid = new QRadioButton("Centroid");
    rad_autotrack_feature_peak = new QRadioButton("Peak");

    QButtonGroup * buttongrp_autotrack_feature = new QButtonGroup();
    buttongrp_autotrack_feature->addButton(rad_autotrack_feature_weighted_centroid);
    buttongrp_autotrack_feature->addButton(rad_autotrack_feature_centroid);
    buttongrp_autotrack_feature->addButton(rad_autotrack_feature_peak);

    grid_autotrack_feature->addWidget(rad_autotrack_feature_weighted_centroid,1,0);
    grid_autotrack_feature->addWidget(rad_autotrack_feature_centroid,2,0);
    grid_autotrack_feature->addWidget(rad_autotrack_feature_peak,3,0);
    grid_autotrack_feature->addItem(vspacer_item20,0,0,1,1);

    hlayout_auto_track_control->addLayout(vlayout_auto_track);
    hlayout_auto_track_control->addWidget(grpbox_autotrack_filters);
    hlayout_auto_track_control->addWidget(grpbox_autotrack_feature);
    hlayout_auto_track_control->insertStretch(0,0);
    hlayout_auto_track_control->insertStretch(-1,0);
    vlayout_auto_track_control->insertStretch(0,0);
    vlayout_auto_track_control->addItem(vspacer_item20);
    vlayout_auto_track_control->addLayout(hlayout_auto_track_control);
    vlayout_auto_track_control->insertStretch(-1,0);
    
    vlayout_tab_workspace->addLayout(vlayout_workspace);
    vlayout_tab_workspace->addWidget(grpbox_autotrack);
    vlayout_tab_workspace->addWidget(grpbox_OSM_track_display);
    vlayout_tab_workspace->insertStretch(-1, 0);

    return widget_tab_tracks;
}
void SirveApp::ResetEngineeringDataAndSliderGUIs()
{
    slider_video->setValue(0);
    if (eng_data != NULL){
        std::set<int> previous_manual_track_ids = track_info->get_manual_track_ids();
        for ( int track_id : previous_manual_track_ids )
        {
            HandleTrackRemoval(track_id);
            video_display->DeleteManualTrack(track_id);
        }
    }
    UpdatePlots();
}

void SirveApp::SetupVideoFrame(){

    frame_video_player->setFrameShape(QFrame::Box);
    QVBoxLayout* vlayout_frame_video = new QVBoxLayout(frame_video_player);

    // ------------------------------------------------------------------------
    vlayout_frame_video->addLayout(video_display->video_display_layout);

    // ------------------------------------------------------------------------

    lbl_fps = new QLabel("fps");
    lbl_fps->setFixedWidth(30);

    // ------------------------------------------------------------------------

    slider_video = new QSlider();
    slider_video->setOrientation(Qt::Horizontal);

    vlayout_frame_video->addWidget(slider_video);

    // ------------------------------------------------------------------------

    // int button_video_width = 45;
    int button_video_width = 42;
    int button_video_height = 50;

    //Add icons to video playback buttons
    btn_play = new QPushButton();
    btn_play->setFixedWidth(button_video_width);
    // btn_play->resize(button_video_width, button_video_height);
    btn_play->setIcon(QIcon(":/icons/play.png"));
    btn_play->setProperty("id", "play");
    btn_play->setToolTip("Play Video");

    btn_pause = new QPushButton();
    // btn_pause->resize(button_video_width, button_video_height);
    btn_pause->setFixedWidth(button_video_width);
    btn_pause->setIcon(QIcon(":/icons/pause.png"));
    btn_pause->setProperty("id", "pause");
    btn_pause->setToolTip("Pause Video");

    btn_reverse = new QPushButton();
    // btn_reverse->resize(button_video_width, button_video_height);
    btn_reverse->setFixedWidth(button_video_width);
    btn_reverse->setIcon(QIcon(":/icons/reverse.png"));
    btn_reverse->setProperty("id", "reverse");
    btn_reverse->setToolTip("Reverse Video");

    btn_fast_forward = new QPushButton();
    btn_fast_forward->setFixedWidth(button_video_width);
    // btn_fast_forward->resize(button_video_width, button_video_height);
    btn_fast_forward->setIcon(QIcon(":/icons/chevron-double-up.png"));
    btn_fast_forward->setToolTip("Increase FPS");

    btn_next_frame = new QPushButton();
    btn_next_frame->setFixedWidth(button_video_width);
    // btn_next_frame->resize(button_video_width, button_video_height);
    btn_next_frame->setIcon(QIcon(":/icons/skip-next.png"));
    btn_next_frame->setProperty("id", "next");
    btn_next_frame->setToolTip("Next Frame");

    btn_slow_back = new QPushButton();
    btn_slow_back->setFixedWidth(button_video_width);
    // btn_slow_back->resize(button_video_width, button_video_height);
    btn_slow_back->setIcon(QIcon(":/icons/chevron-double-down.png"));
    btn_slow_back->setToolTip("Decrease FPS");

    btn_prev_frame = new QPushButton();
    btn_prev_frame->setFixedWidth(button_video_width);
    // btn_prev_frame->resize(button_video_width, button_video_height);
    btn_prev_frame->setIcon(QIcon(":/icons/skip-previous.png"));
    btn_prev_frame->setProperty("id", "previous");
    btn_prev_frame->setToolTip("Previous Frame");

    btn_frame_record = new QPushButton();
    btn_frame_record->setFixedWidth(button_video_width);
    // btn_frame_record->resize(button_video_width, button_video_height);
    btn_frame_record->setIcon(QIcon(":/icons/record.png"));
    btn_frame_record->setToolTip("Record Video");

    btn_frame_save = new QPushButton();
    btn_frame_save->setFixedWidth(button_video_width);
    // btn_frame_save->resize(button_video_width, button_video_height);
    btn_frame_save->setIcon(QIcon(":/icons/content-save.png"));
    btn_frame_save->setToolTip("Save Frame");

    btn_zoom = new QPushButton();
    btn_zoom->setFixedWidth(button_video_width);
    // btn_zoom->resize(button_video_width, button_video_height);
    btn_zoom->setIcon(QIcon(":/icons/magnify.png"));
    btn_zoom->setCheckable(true);

    // btn_calculate_radiance = new QPushButton();
    // btn_calculate_radiance->resize(button_video_width, button_video_height);
    // btn_calculate_radiance->setIcon(QIcon(":/icons/signal.png"));
    // btn_calculate_radiance->setCheckable(true);

    btn_popout_video = new QPushButton();
    btn_popout_video->setFixedWidth(button_video_width);
    // btn_popout_video->resize(button_video_width, button_video_height);
    btn_popout_video->setIcon(QIcon(":/icons/expand.png"));
    btn_popout_video->setCheckable(true);

    btn_popout_video->setEnabled(false);

    lbl_goto_frame = new QLabel("# Frames");
    lbl_goto_frame->setFixedWidth(60);

    txt_goto_frame = new QLineEdit("");
    txt_goto_frame->setFixedWidth(50);
    txt_goto_frame->setEnabled(false);

    connect(txt_goto_frame, &QLineEdit::editingFinished,this, &SirveApp::HandleFrameNumberChangeInput);
    QFormLayout *formLayout = new QFormLayout;
    formLayout->setAlignment(Qt::AlignHCenter|Qt::AlignCenter);
    formLayout->addRow(tr("&Frame:"),txt_goto_frame);
    QHBoxLayout* hlayout_video_buttons = new QHBoxLayout();
    hlayout_video_buttons->addWidget(btn_frame_save);
    hlayout_video_buttons->addWidget(btn_frame_record);
    hlayout_video_buttons->addWidget(btn_zoom);
    hlayout_video_buttons->addWidget(btn_popout_video);
    hlayout_video_buttons->addLayout(formLayout);
    hlayout_video_buttons->addWidget(btn_prev_frame);
    hlayout_video_buttons->addWidget(btn_reverse);
    hlayout_video_buttons->addWidget(btn_pause);
    hlayout_video_buttons->addWidget(btn_play);
    hlayout_video_buttons->addWidget(btn_next_frame);
    hlayout_video_buttons->addWidget(lbl_fps);
    hlayout_video_buttons->addWidget(btn_fast_forward);
    hlayout_video_buttons->addWidget(btn_slow_back);
    hlayout_video_buttons->insertStretch(0,0);
    hlayout_video_buttons->insertStretch(-1,0);
    vlayout_frame_video->addLayout(hlayout_video_buttons);
}

void SirveApp::SetupPlotFrame() {

    tab_plots->setTabPosition(QTabWidget::South);

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
    btn_popout_histogram->setEnabled(false);

    vlayout_tab_histogram->addWidget(btn_popout_histogram);
    chk_relative_histogram = new QCheckBox("Relative Histogram");
    vlayout_tab_histogram->addWidget(frame_histogram_abs);
    vlayout_tab_histogram->addWidget(frame_histogram_rel);
    vlayout_tab_histogram->addWidget(chk_relative_histogram);

    // ------------------------------------------------------------------------

    frame_plots = new QFrame();
    plot_groupbox = new QGroupBox("Y-Axis Options");

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
    btn_save_plot = new QPushButton("Save Plot");

    btn_save_plot->setToolTip("Save Plot");

    btn_plot_menu = new QPushButton("Plot Options");

    QGridLayout* grid_y_axis_options_groupbox = new QGridLayout(plot_groupbox);
    grid_y_axis_options_groupbox->addWidget(rad_linear, 0, 0);
    grid_y_axis_options_groupbox->addWidget(rad_log, 1, 0);
    grid_y_axis_options_groupbox->addWidget(rad_decimal, 0, 1);
    grid_y_axis_options_groupbox->addWidget(rad_scientific, 1, 1);

    // set layout for combo boxes
    QFormLayout *form_plot_axis_options = new QFormLayout;
    form_plot_axis_options->addRow(tr("&Y-Axis:"), cmb_plot_yaxis);
    form_plot_axis_options->addRow(tr("&X-Axis:"), cmb_plot_xaxis);
    form_plot_axis_options->addRow(tr("&"), btn_plot_menu);
    form_plot_axis_options->addRow(tr(""), btn_save_plot);

    // set layout for everything below the plot
    QHBoxLayout* hlayout_widget_plots_tab_color_control = new QHBoxLayout();
    hlayout_widget_plots_tab_color_control->insertStretch(0,0);
    hlayout_widget_plots_tab_color_control->addLayout(form_plot_axis_options);
    hlayout_widget_plots_tab_color_control->addWidget(plot_groupbox);
    hlayout_widget_plots_tab_color_control->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout
    // plot_groupbox->setMinimumWidth(333);
    plot_groupbox->setEnabled(false);

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

    tab_plots->tabBar()->hide();
}

void SirveApp::setupConnections() {

    connect(this, &SirveApp::updateVideoDisplayPinpointControls, this->video_display, &VideoDisplay::HandlePinpointControlActivation);

    //---------------------------------------------------------------------------
    connect(&video_display->container, &VideoContainer::updateDisplayVideo, this, &SirveApp::HandleFrameChange);
    connect(btn_undo_step, &QPushButton::clicked, &video_display->container, &VideoContainer::PopProcessingState);
    connect(playback_controller, &FramePlayer::frameSelected, this, &SirveApp::HandleFrameNumberChange);

    connect(&video_display->container, &VideoContainer::stateAdded, this, &SirveApp::HandleNewProcessingState);
    connect(&video_display->container, &VideoContainer::stateRemoved, this, &SirveApp::HandleProcessingStateRemoval);
    connect(&video_display->container, &VideoContainer::statesCleared, this, &SirveApp::HandleProcessingStatesCleared);

    connect(cmb_processing_states, qOverload<int>(&QComboBox::currentIndexChanged), video_display, &VideoDisplay::GetCurrentIdx);
    connect(cmb_processing_states, qOverload<int>(&QComboBox::currentIndexChanged), &video_display->container, &VideoContainer::SelectState);
    connect(cmb_processing_states, qOverload<int>(&QComboBox::currentIndexChanged), this, &SirveApp::HandleProcessingNewStateSelected);

    connect(histogram_plot, &HistogramLinePlot::clickDragHistogram, this, &SirveApp::HandleHistogramClick);

    connect(video_display, &VideoDisplay::addNewBadPixels, this, &SirveApp::ReceiveNewBadPixels);
    connect(video_display, &VideoDisplay::removeBadPixels, this, &SirveApp::ReceiveNewGoodPixels);

    //---------------------------------------------------------------------------

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

    connect(chk_show_OSM_tracks, &QCheckBox::stateChanged, this, &SirveApp::HandleOsmTracksToggle);
    connect(cmb_OSM_track_color, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::EditOSMTrackColor);

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
    // connect(btn_calculate_radiance, &QPushButton::clicked, this, &SirveApp::HandleCalculationOnVideoToggle);
    connect(video_display, &VideoDisplay::clearMouseButtons, this, &SirveApp::ClearZoomAndCalculationButtons);

    connect(btn_popout_video, &QPushButton::clicked, this, &SirveApp::HandlePopoutVideoClick);

    //---------------------------------------------------------------------------

    //Link buttons to functions
    connect(btn_get_frames, &QPushButton::clicked, this, &SirveApp::UiLoadAbirData);
    connect(txt_stop_frame, &QLineEdit::returnPressed, this, &SirveApp::UiLoadAbirData);

    connect(chk_highlight_bad_pixels, &QPushButton::clicked, video_display, &VideoDisplay::HighlightBadPixels);

    connect(btn_FNS, &QPushButton::clicked, this, &SirveApp::ExecuteFixedNoiseSuppression);

    connect(btn_ANS, &QPushButton::clicked, this, &SirveApp::ExecuteAdaptiveNoiseSuppression);

    //---------------------------------------------------------------------------
    connect(btn_import_tracks, &QPushButton::clicked, this, &SirveApp::ImportTracks);
    connect(btn_create_track, &QPushButton::clicked, this, &SirveApp::HandleCreateTrackClick);
    connect(btn_finish_create_track, &QPushButton::clicked, this, &SirveApp::HandleFinishCreateTrackClick);
    connect(video_display, &VideoDisplay::finishTrackCreation, this, &SirveApp::HandleFinishCreateTrackClick);

    connect(tm_widget, &TrackManagementWidget::displayTrack, this, &SirveApp::HandleShowManualTrackId);
    connect(tm_widget, &TrackManagementWidget::hideTrack, this, &SirveApp::HandleHideManualTrackId);
    connect(tm_widget, &TrackManagementWidget::deleteTrack, this, &SirveApp::HandleTrackRemoval);
    connect(tm_widget, &TrackManagementWidget::recolorTrack, this, &SirveApp::HandleManualTrackRecoloring);

    // Connect epoch button click to function
    connect(btn_apply_epoch, &QPushButton::clicked, this, &SirveApp::ApplyEpochTime);

    //Enable saving frame
    connect(btn_frame_save, &QPushButton::clicked, this, &SirveApp::SaveFrame);

    //---------------------------------------------------------------------------
    // Connect y-axis change to function
    connect(cmb_plot_yaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::HandleYAxisChange);

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
    connect(this, &SirveApp::enableYAxisOptions, this, &SirveApp::EnableYAxisOptions);
}

void SirveApp::HandleBadPixelRawToggle()
{
    if (chk_bad_pixels_from_original->isChecked()){
        txt_bad_pixel_start_frame->setText("1");
        int nframes = osm_frames.size();
        int istop = nframes;
        txt_bad_pixel_stop_frame->setText(QString::number(istop));
    }
    else{
        txt_bad_pixel_start_frame->setText(txt_start_frame->text());
        int istop = std::min(txt_start_frame->text().toInt() + 2000,txt_stop_frame->text().toInt());
        txt_bad_pixel_stop_frame->setText(QString::number(istop));
    }
}
void SirveApp::HandleExternalFileToggle()
{
    if (chk_FNS_external_file->isChecked()){
        txt_FNS_start_frame->setStyleSheet("#txt_FNS_start_frame {background-color:#f0f0f0; color:rgb(75,75,75);}");
        txt_FNS_stop_frame->setStyleSheet("#txt_FNS_stop_frame {background-color:#f0f0f0; color:rgb(75,75,75);}");
        txt_FNS_stop_frame->setEnabled(false);
        txt_FNS_start_frame->setEnabled(false);
    }
    else{
        txt_FNS_start_frame->setStyleSheet("#txt_FNS_start_frame {background-color:#ffffff; color:rgb(0,0,0);}");
        txt_FNS_stop_frame->setStyleSheet("#txt_FNS_stop_frame {background-color:#ffffff; color:rgb(0,0,0);}");
        txt_FNS_stop_frame->setEnabled(true);
        txt_FNS_start_frame->setEnabled(true);
    }
}

void SirveApp::HandleYAxisChange()
{
    if(cmb_plot_yaxis->currentIndex()==0){
        rad_scientific->setChecked(true);
    }
    else{
        rad_decimal->setChecked(true);
        rad_linear->setChecked(true);
    }
    yAxisChanged=true;
    UpdatePlots();
}

void SirveApp::HandleXAxisOptionChange()
{
    if (data_plots != NULL)
    {
        double ymax = data_plots->yaxis_is_log ? data_plots->axis_ylog->max() : data_plots->axis_y->max();
        double ymin = data_plots->yaxis_is_log ? data_plots->axis_ylog->min() : data_plots->axis_y->min();
        UpdatePlots();

        if (ymin > 0 && ymax > 0)
        {
            data_plots->set_yaxis_limits(ymin, ymax);
        }
    }
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
            auto response = QtHelpers::LaunchYesNoMessageBox("Confirm Track Overwriting", "Warning: Overwriting track ID: " + QString::number(track_id));
            if (response == QMessageBox::Yes)
            {
                video_display->AddManualTrackIdToShowLater(track_id);
                tm_widget->AddTrackControl(track_id);
                QWidget * existing_track_control = tm_widget->findChild<QWidget*>(QString("TrackControl_%1").arg(track_id));
                if (existing_track_control != nullptr)
                {
                    QLabel *lbl_track_description = existing_track_control->findChild<QLabel*>("track_description");
                    const QFileInfo info(file_selection);
                    lbl_track_description->setText(info.fileName());
                } 
                track_info->AddManualTracks(result.frames);

                int index0 = data_plots->index_sub_plot_xmin;
                int index1 = data_plots->index_sub_plot_xmax + 1;
                video_display->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
                data_plots->UpdateManualPlottingTrackFrames(track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
                FramePlotSpace();
            }
        }
        else
        {
            video_display->AddManualTrackIdToShowLater(track_id);
            tm_widget->AddTrackControl(track_id);
            track_info->AddManualTracks(result.frames);
            cmb_manual_track_IDs->clear();
            cmb_manual_track_IDs->addItem("Primary");
            std::set<int> track_ids = track_info->get_manual_track_ids();
            for ( int track_id : track_ids ){
                cmb_manual_track_IDs->addItem(QString::number(track_id));
            } 

            QWidget * existing_track_control = tm_widget->findChild<QWidget*>(QString("TrackControl_%1").arg(track_id));
            if (existing_track_control != nullptr)
            {
                QLabel *lbl_track_description = existing_track_control->findChild<QLabel*>("track_description");
                const QFileInfo info(file_selection);
                lbl_track_description->setText(info.fileName());
            } 
            int index0 = data_plots->index_sub_plot_xmin;
            int index1 = data_plots->index_sub_plot_xmax + 1;
            video_display->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
            data_plots->UpdateManualPlottingTrackFrames(track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
            FramePlotSpace();
        }

    } 
}

void SirveApp::HandleCreateTrackClick()
{
    bool ok;
    std::set<int> previous_manual_track_ids = track_info->get_manual_track_ids();
    int maxID = 0;
    if (previous_manual_track_ids.size()>0){
        maxID = *max_element(previous_manual_track_ids.begin(), previous_manual_track_ids.end());
    }
    u_int track_id = QInputDialog::getInt(this, tr("Select New Track Identifier"), tr("Track ID:"), maxID+1, 1, 1000000, 1, &ok);
    if (!ok || track_id < 0)
    {
        return;
    }

    if (previous_manual_track_ids.find(track_id) != previous_manual_track_ids.end())
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Confirm Track Overwriting", "The manual track ID you have chosen already exists. You can edit this track without saving, but finalizing this track will overwrite it. Are you sure you want to proceed with editing the existing manual track?");
        if (response == QMessageBox::Yes)
        {
            QWidget * existing_track_control = tm_widget->findChild<QWidget*>(QString("TrackControl_%1").arg(track_id));
            if (existing_track_control != nullptr)
            {
                existing_track_control->findChild<QCheckBoxWithId*>()->setChecked(false);
            } 
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
        QString new_track_file_name;
        QString suggested_track_name;
        std::set<int> previous_manual_track_ids = track_info->get_manual_track_ids();
        bool existing_track_TF = false;
        if (previous_manual_track_ids.find(currently_editing_or_creating_track_id) == previous_manual_track_ids.end())
        {
            QString base_track_folder = config_values.workspace_folder;
            QDate today = QDate::currentDate();
            QTime currentTime = QTime::currentTime();;
            QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
            suggested_track_name = base_track_folder + "/manual_track_" + QString::number(currently_editing_or_creating_track_id) + "_" + formattedDate;
        }
        else
        {
            existing_track_TF = true;          
            QWidget * existing_track_control = tm_widget->findChild<QWidget*>(QString("TrackControl_%1").arg(currently_editing_or_creating_track_id));
            if (existing_track_control != nullptr)
                {
                    QLabel *lbl_track_description = existing_track_control->findChild<QLabel*>("track_description");
                    suggested_track_name = lbl_track_description->text();
                }    
            
        }
        new_track_file_name = QFileDialog::getSaveFileName(this, "Select a new file to save the track into", config_values.workspace_folder + "/" + suggested_track_name,  "CSV (*.csv)");
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

        FramePlotSpace();

        if (!existing_track_TF)
        {
            cmb_manual_track_IDs->clear();
            cmb_manual_track_IDs->addItem("Primary");
            std::set<int> track_ids = track_info->get_manual_track_ids();
            for ( int track_id : track_ids ){
                cmb_manual_track_IDs->addItem(QString::number(track_id));
            }
        }

        QStringList color_options = ColorScheme::get_track_colors();
        QWidget * existing_track_control = tm_widget->findChild<QWidget*>(QString("TrackControl_%1").arg(currently_editing_or_creating_track_id));
        if (existing_track_control != nullptr)
        {
            QLabel *lbl_track_description = existing_track_control->findChild<QLabel*>("track_description");
            const QFileInfo info(new_track_file_name);
            lbl_track_description->setText(info.fileName());
            int ind = existing_track_control->findChild<QComboBoxWithId*>()->currentIndex();
            HandleManualTrackRecoloring(currently_editing_or_creating_track_id, color_options[ind]);
        }  
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

void SirveApp::HandleHideManualTrackId(int track_id)
{
    QColor new_color(0,0,0,0);
    video_display->HideManualTrackId(track_id);
    int index0 = data_plots->index_sub_plot_xmin;
    int index1 = data_plots->index_sub_plot_xmax + 1;
    data_plots->RecolorManualTrack(track_id, new_color);
    FramePlotSpace();
}

void SirveApp::HandleShowManualTrackId(int track_id, QColor new_color)
{
    video_display->ShowManualTrackId(track_id);
    data_plots->RecolorManualTrack(track_id, new_color);
    FramePlotSpace();
}

void SirveApp::HandleTrackRemoval(int track_id)
{
    tm_widget->RemoveTrackControl(track_id);
    track_info->RemoveManualTrack(track_id);
    cmb_manual_track_IDs->clear();
    cmb_manual_track_IDs->addItem("Primary");
    std::set<int> track_ids = track_info->get_manual_track_ids();
    for ( int track_id : track_ids ){
        cmb_manual_track_IDs->addItem(QString::number(track_id));
    } 
    track_info->RemoveManualTrackPlotting(track_id);
    track_info->RemoveManualTrackImage(track_id);
    int index0 = data_plots->index_sub_plot_xmin;
    int index1 = data_plots->index_sub_plot_xmax + 1;
    video_display->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
    video_display->DeleteManualTrack(track_id);
    data_plots->UpdateManualPlottingTrackFrames(track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
    FramePlotSpace();
}

void SirveApp::HandleManualTrackRecoloring(int track_id, QColor new_color)
{
    video_display->RecolorManualTrack(track_id, new_color);
    data_plots->RecolorManualTrack(track_id, new_color);
    FramePlotSpace();
}

void SirveApp::FramePlotSpace()
{
    double xmax = data_plots->axis_x->max();
    double xmin = data_plots->axis_x->min();
    double ymax = data_plots->yaxis_is_log ? data_plots->axis_ylog->max() : data_plots->axis_y->max();
    double ymin = data_plots->yaxis_is_log ? data_plots->axis_ylog->min() : data_plots->axis_y->min();
    UpdatePlots();
    data_plots->set_xaxis_limits(xmin,xmax);
    data_plots->set_yaxis_limits(ymin,ymax);
}

void SirveApp::SaveWorkspace()
{
    if (abp_file_metadata.image_path == "" || video_display->container.get_processing_states().size() == 0) {
        QtHelpers::LaunchMessageBox(QString("Issue Saving Workspace"), "No frames are loaded, unable to save workspace.");
    }
    else {
        QString current_workspace_name = lbl_workspace_name_field->text();
        QDate today = QDate::currentDate();
        QTime currentTime = QTime::currentTime();
        QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
        QString start_frame = QString::number(data_plots->index_sub_plot_xmin + 1);
        QString stop_frame = QString::number(data_plots->index_sub_plot_xmax + 1);
        QString initial_name = abpimage_file_base_name + "_" + start_frame + "-"+ stop_frame + "_" + formattedDate;

        QString selectedUserFilePath = QFileDialog::getSaveFileName(this, tr("Workspace File"), config_values.workspace_folder + "/" + initial_name, tr("Workspace Files *.json"));

        if (selectedUserFilePath.length() > 0) {
            QFileInfo fileInfo(selectedUserFilePath);
            workspace->SaveState(selectedUserFilePath, abp_file_metadata.image_path, data_plots->index_sub_plot_xmin + 1, data_plots->index_sub_plot_xmax + 1, video_display->container.get_processing_states(), video_display->annotation_list);
            lbl_workspace_name_field->setText(fileInfo.fileName());
        }
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

        lbl_current_workspace_folder_field->setText(filePath);
        lbl_workspace_name_field->setText(fileInfo.fileName());
        bool validated = ValidateAbpFiles(workspace_vals.image_path);
        if (validated) {
            LoadOsmData();
            QFileInfo fileInfo0(workspace_vals.image_path);
            abpimage_file_base_name = fileInfo0.baseName();
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

        for (auto i = 1; i < workspace_vals.all_states.size(); i++)
        {
            processingState current_state = workspace_vals.all_states[i];

            switch (current_state.method)
            {
                case ProcessingMethod::original:
                {
                    break;
                }
                case ProcessingMethod::replace_bad_pixels:
                {
                    ReplaceBadPixels(current_state.replaced_pixels,current_state.source_state_ID);
                    break;
                }
                case ProcessingMethod::RPCP_noise_suppression:
                {
                    ApplyRPCPNoiseSuppression(current_state.source_state_ID);
                    break;
                }

                case ProcessingMethod::accumulator_noise_suppression:
                {
                    ApplyAccumulatorNoiseSuppression(current_state.weight, current_state.offset, current_state.hide_shadow, current_state.shadow_threshold, current_state.source_state_ID);
                    break;
                }

                case ProcessingMethod::adaptive_noise_suppression:
                {
                    ApplyAdaptiveNoiseSuppression(current_state.ANS_relative_start_frame, current_state.ANS_num_frames, current_state.source_state_ID);
                    break;
                }
                case ProcessingMethod::deinterlace:{
                    ApplyDeinterlacing(current_state.source_state_ID);
                    break;
                }
                case ProcessingMethod::fixed_noise_suppression:{
                    ApplyFixedNoiseSuppression(workspace_vals.image_path, current_state.FNS_file_path, current_state.frame0, current_state.FNS_start_frame, current_state.FNS_stop_frame, current_state.source_state_ID);
                    break;
                }
                case ProcessingMethod::center_on_OSM:{
                    QString trackFeaturePriority = "OSM";
                    CenterOnOffsets(trackFeaturePriority,current_state.track_id, current_state.offsets,current_state.find_any_tracks, current_state.source_state_ID);
                    break;
                }
                case ProcessingMethod::center_on_manual:{
                    QString trackFeaturePriority = "manual";
                    CenterOnOffsets(trackFeaturePriority,current_state.track_id, current_state.offsets,current_state.find_any_tracks, current_state.source_state_ID);
                    break;
                }
                case ProcessingMethod::center_on_brightest:{
                    CenterOnBrightest(current_state.offsets,current_state.source_state_ID);
                    break;
                }
                case ProcessingMethod::frame_stacking:{
                    FrameStacking(current_state.frame_stack_num_frames, current_state.source_state_ID);
                    break;
                }
                default:
                    QtHelpers::LaunchMessageBox(QString("Unexpected Workspace Behavior"), "Unexpected processing method in workspace, unable to proceed.");
            }
        }

        video_display->annotation_list.clear();
        video_display->EstablishStencil();
        video_display->HideStencil();

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
        QFileInfo fileInfo(file_selection);
        abpimage_file_base_name = fileInfo.baseName();
	}
};

bool SirveApp::ValidateAbpFiles(QString path_to_image_file)
{
    AbpFileMetadata possible_abp_file_metadata = file_processor->LocateAbpFiles(path_to_image_file);

	if (!possible_abp_file_metadata.error_msg.isEmpty())
	{
		if (eng_data != NULL) {
			// if eng_data already initialized, allow user to re-select frames
			txt_start_frame->setEnabled(true);
			txt_stop_frame->setEnabled(true);
			btn_get_frames->setEnabled(true);
			txt_start_frame->setStyleSheet(orange_styleSheet);
			txt_stop_frame->setStyleSheet(orange_styleSheet);
		}
		else{
			txt_start_frame->setEnabled(false);
			txt_stop_frame->setEnabled(false);
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
    ResetEngineeringDataAndSliderGUIs();
    osm_frames = osm_reader.ReadOsmFileData(abp_file_metadata.osm_path);
    if (osm_frames.size() == 0)
    {
        QtHelpers::LaunchMessageBox(QString("Error loading OSM file"), QString("Error reading OSM file. Close program and open logs for details."));
        return;
    }

    lbl_file_name->setText("OSM File Name: " + abp_file_metadata.file_name);
    lbl_file_name->setToolTip(abp_file_metadata.directory_path);

    this->window()->setWindowTitle("SirveApp - " + abp_file_metadata.directory_path + "/" + abp_file_metadata.file_name);

    QString osm_max_frames = QString::number(osm_frames.size());
    txt_start_frame->setText(QString("1"));
    txt_stop_frame->setText(osm_max_frames);

    lbl_max_frames->setText("Available Frames: " + osm_max_frames);

    SetLiftAndGain(0, 1);
	txt_start_frame->setStyleSheet(orange_styleSheet);
	txt_stop_frame->setStyleSheet(orange_styleSheet);
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
        txt_stop_frame->setStyleSheet(orange_styleSheet);
    }

    eng_data = new EngineeringData(osm_frames);
    track_info = new TrackInformation(osm_frames);
    data_plots = new EngineeringPlots(osm_frames);

    osmDataLoaded = true;

    connect(btn_pause, &QPushButton::clicked, data_plots, &EngineeringPlots::HandlePlayerButtonClick);
    connect(btn_play, &QPushButton::clicked, data_plots, &EngineeringPlots::HandlePlayerButtonClick);
    connect(btn_reverse, &QPushButton::clicked, data_plots, &EngineeringPlots::HandlePlayerButtonClick);
    connect(btn_next_frame, &QPushButton::clicked, data_plots, &EngineeringPlots::HandlePlayerButtonClick);
    connect(btn_prev_frame, &QPushButton::clicked, data_plots, &EngineeringPlots::HandlePlayerButtonClick);

    // Locks down and frees up the frame range specification text boxes:
    connect(this->data_plots, &EngineeringPlots::changeMotionStatus, this, &SirveApp::HandlePlayerStateChanged);

    // Ensure the user can zoom out sooner rather than later:
    connect(this->data_plots->chart_view, &NewChartView::updatePlots, this, &SirveApp::UpdatePlots);

    size_t num_tracks = track_info->get_track_count();
    if (num_tracks == 0)
    {
        QtHelpers::LaunchMessageBox(QString("No Tracking Data"), "No tracking data was found within the file. No data will be plotted.");
    }

    data_plots->past_midnight = eng_data->get_seconds_from_midnight();
    data_plots->past_epoch = eng_data->get_seconds_from_epoch();

    data_plots->set_plotting_track_frames(track_info->get_osm_plotting_track_frames(), track_info->get_track_count());

    //--------------------------------------------------------------------------------

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
    // btn_popout_engineering->resize(40, 40);
    btn_popout_engineering->setCheckable(true);
    connect(btn_popout_engineering, &QPushButton::clicked, this, &SirveApp::HandlePopoutEngineeringClick);
    engineering_plot_layout->addWidget(btn_popout_engineering);
    engineering_plot_layout->addWidget(data_plots->chart_view);
    frame_plots->setLayout(engineering_plot_layout);

    // btn_calculate_radiance->setChecked(false);
    // btn_calculate_radiance->setEnabled(false);
    chk_highlight_bad_pixels->setChecked(false);
    chk_highlight_bad_pixels->setEnabled(false);

    btn_create_track->setEnabled(false);
    btn_import_tracks->setEnabled(false);

    CalibrationData temp;
    calibration_model = temp;

    // Reset settings on video playback to defaults
    chk_show_OSM_tracks->setChecked(true);
    chk_show_time->setChecked(false);
    chk_sensor_track_data->setChecked(false);
    cmb_text_color->setCurrentIndex(2);
    video_display->InitializeToggles();

    // Reset setting engineering plot defaults
    menu_plot_all_data->setIconVisibleInMenu(true);

    menu_plot_primary->setIconVisibleInMenu(false);

    menu_plot_frame_marker->setIconVisibleInMenu(false);

    EnableEngineeringPlotOptions();

    EnableYAxisOptions(false);

    data_plots->SetPlotTitle(QString("EDIT CLASSIFICATION"));

    data_plots->InitializeIntervals(osm_frames);

    UpdateGuiPostDataLoad(osmDataLoaded);

    return;
}

void SirveApp::UpdateGuiPostDataLoad(bool osm_data_status)
{
    // Enable plot capabilities
    btn_save_plot->setEnabled(osm_data_status);
    btn_plot_menu->setEnabled(osm_data_status);

    // Enable setting of epoch
    dt_epoch->setEnabled(osm_data_status);
    btn_apply_epoch->setEnabled(osm_data_status);

    // Enable frame range selection
    btn_get_frames->setEnabled(osm_data_status);
    txt_start_frame->setEnabled(osm_data_status);
    txt_stop_frame->setEnabled(osm_data_status);

    plot_groupbox->setEnabled(osm_data_status);
    cmb_plot_yaxis->setEnabled(osm_data_status);
    cmb_plot_xaxis->setEnabled(osm_data_status);

    osm_data_status ? tab_plots->tabBar()->show() : tab_plots->tabBar()->hide();

    connect(cmb_plot_xaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::HandleXAxisOptionChange );
}

void SirveApp::UpdateGuiPostFrameRangeLoad(bool frame_range_status)
{
    btn_popout_video->setEnabled(frame_range_status);
    txt_goto_frame->setEnabled(frame_range_status);

    // Enable plot popout only
    btn_popout_histogram->setEnabled(frame_range_status);

    frame_range_status ? tab_menu->tabBar()->show() : tab_menu->tabBar()->hide();

    // action_export_current_frame->setEnabled(frame_range_status);
    // action_export_frame_range->setEnabled(frame_range_status);
    // action_export_all_frames->setEnabled(frame_range_status);

    // Enable the video pinpoint capabilities, which are
    // privately held within the video display class
    emit updateVideoDisplayPinpointControls(frame_range_status);
}

void SirveApp::UiLoadAbirData()
{
    btn_get_frames->setEnabled(false);

    int min_frame = ConvertFrameNumberTextToInt(txt_start_frame->text());
    int max_frame = ConvertFrameNumberTextToInt(txt_stop_frame->text());

    if (!VerifyFrameSelection(min_frame, max_frame)) {
        btn_get_frames->setEnabled(true);

        return;
    }

    if (video_display->annotation_list.size() > 0)
    {
        video_display->annotation_list.clear();
        video_display->EstablishStencil();
        video_display->HideStencil();
    }

    LoadAbirData(min_frame, max_frame);
    lbl_workspace_name->setText("Workspace File: ");
}

void SirveApp::LoadAbirData(int min_frame, int max_frame)
{
    DeleteAbirData();
    ResetEngineeringDataAndSliderGUIs();
    AllocateAbirData(min_frame, max_frame);
}

void SirveApp::DeleteAbirData()
{
    // abir_data_result = nullptr;
    if (file_processor->abir_data.ir_data.size()>0){
        file_processor->abir_data.ir_data.clear();
        file_processor->data_result->video_frames_16bit.clear();
    }
}

void SirveApp::AllocateAbirData(int min_frame, int max_frame)
{
    playback_controller->StopTimer();
    video_display->container.ClearProcessingStates();

    grpbox_progressbar_area->setEnabled(true);
    progress_bar_main->setTextVisible(true);
    progress_bar_main->setRange(0,100);

    // Task 1:
    progress_bar_main->setValue(0);
    lbl_progress_status->setText(QString("Loading ABIR data frames..."));
    file_processor->LoadImageFile(abp_file_metadata.image_path, min_frame, max_frame, config_values.version);

    if (file_processor->getAbirDataLoadResult()->had_error) {
        QtHelpers::LaunchMessageBox(QString("Error Reading ABIR Frames"), "Error reading .abpimage file. See log for more details.");
        btn_get_frames->setEnabled(true);

        return;
    }

    // Task 2:
    QCoreApplication::processEvents();
    progress_bar_main->setValue(0);
    lbl_progress_status->setText(QString("Deriving processing state..."));
    this->repaint();
    processingState primary;
    primary.method = ProcessingMethod::original;
    primary.state_ID = 0;
    unsigned int number_frames = static_cast<unsigned int>(file_processor->getAbirDataLoadResult()->video_frames_16bit.size());
    int x_pixels = file_processor->getAbirDataLoadResult()->x_pixels;
    progress_bar_main->setValue(20);
    int y_pixels = file_processor->getAbirDataLoadResult()->y_pixels;
    progress_bar_main->setValue(40);
    int max_value = file_processor->getAbirDataLoadResult()->max_value;
    progress_bar_main->setValue(60);
    primary.details = {x_pixels, y_pixels, max_value, file_processor->getAbirDataLoadResult()->video_frames_16bit};
    progress_bar_main->setValue(80);
    max_frame = file_processor->data_result->last_valid_frame;
    progress_bar_main->setValue(100);

    // Task 3:
    QCoreApplication::processEvents();
    progress_bar_main->setValue(30);
    lbl_progress_status->setText(QString("Adding processing state..."));
    this->repaint();

    video_display->container.processing_states.push_back(primary);
    QString state_name = "State " + QString::number(0) + ": " + video_display->container.processing_states[0].get_friendly_description();
    QString combobox_state_name = QString::number(0) + ": " + video_display->container.processing_states[0].get_combobox_description();
    video_display->container.processing_states[0].state_description = state_name;
    HandleNewProcessingState(state_name, combobox_state_name, 0);

    progress_bar_main->setValue(80);
    
    this->repaint();

    txt_start_frame->setText(QString::number(min_frame));
    txt_stop_frame->setText(QString::number(max_frame));

    //---------------------------------------------------------------------------
    // Set frame number for playback controller and valid values for slider
    playback_controller->set_number_of_frames(number_frames);
    slider_video->setRange(0, number_frames - 1);

    // Start threads...
    // if (!thread_timer.isRunning())
    // {
    //     thread_video.start();
    //     thread_timer.start();
    // }

    // Task 4:
    QCoreApplication::processEvents();
    progress_bar_main->setValue(0);
    lbl_progress_status->setText(QString("Configuring the chart plotter..."));
    this->repaint();

    int index0 = min_frame - 1;
    int index1 = max_frame;
    std::vector<PlottingFrameData> temp = eng_data->get_subset_plotting_frame_data(index0, index1);

    video_display->InitializeTrackData(track_info->get_osm_frames(index0, index1), track_info->get_manual_frames(index0, index1));
    cmb_OSM_track_IDs->clear();
    cmb_OSM_track_IDs->addItem("Primary");
    cmb_manual_track_IDs->clear();
    cmb_manual_track_IDs->addItem("Primary");
    std::set<int> track_ids = track_info->get_OSM_track_ids();
    for ( int track_id : track_ids ){
        cmb_OSM_track_IDs->addItem(QString::number(track_id));
    }

    video_display->InitializeFrameData(min_frame, temp, file_processor->abir_data.ir_data);
    DeleteAbirData();
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

    // Update frame marker on engineering plot
    connect(playback_controller, &FramePlayer::frameSelected, data_plots, &EngineeringPlots::PlotCurrentStep);
    connect(this->data_plots, &EngineeringPlots::updatePlots, this, &SirveApp::UpdatePlots);
    connect(this->data_plots->chart_view, &NewChartView::updateFrameLine, this, &SirveApp::HandleZoomAfterSlider);

    playback_controller->set_initial_speed_index(10);
    UpdateFps();

    progress_bar_main->setValue(100);
    this->repaint();;

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
    int num_video_frames = max_frame - min_frame + 1;
    lbl_loaded_frames->setText("Loaded Frames: " + QString::number(num_video_frames));
    lbl_status_start_frame->setText("Start Frame: " + QString::number(min_frame));
    lbl_status_stop_frame->setText("Stop Frame: " + QString::number(max_frame));
    QValidator *validator = new QIntValidator(min_frame, max_frame, this);
    txt_auto_track_start_frame->setValidator(validator);
    txt_auto_track_stop_frame->setValidator(validator);
    txt_auto_track_start_frame->setText(QString::number(min_frame));
    txt_auto_track_stop_frame->setText(QString::number(max_frame));
    connect(txt_auto_track_start_frame, &QLineEdit::editingFinished,this, &SirveApp::HandleAutoTrackStartChangeInput);
    connect(txt_auto_track_stop_frame, &QLineEdit::editingFinished,this, &SirveApp::HandleAutoTrackStopChangeInput);

    ToggleVideoPlaybackOptions(true);
    UpdateGuiPostFrameRangeLoad(true);

    progress_bar_main->setValue(0);
    progress_bar_main->setTextVisible(false);
    grpbox_progressbar_area->setEnabled(false);
    lbl_progress_status->setText(QString(""));

    txt_FNS_start_frame->setText(txt_start_frame->text());
    int istop = txt_start_frame->text().toInt() + 50;
    txt_FNS_stop_frame->setText(QString::number(istop));
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

void SirveApp::HandleProgressUpdate(int percent)
{
    progress_bar_main->setValue(percent);
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
        btn_frame_record->setIcon(QIcon(":icons/record.png"));
        btn_frame_record->setText("");
        btn_frame_record->setToolTip("Start Record");
        btn_frame_record->setEnabled(true);

        record_video = false;
    }
    else {
        //Starting record video
        QString start_frame = txt_start_frame->text();
        QString stop_frame = txt_stop_frame->text();
        QString base_folder = config_values.workspace_folder;
        QDate today = QDate::currentDate();
        QTime currentTime = QTime::currentTime();;
        QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
        QString suggested_name = base_folder + "/" + abp_file_metadata.file_name + "_" +start_frame + "_" + stop_frame +"_" + formattedDate;
        QString file_name = QFileDialog::getSaveFileName(this, "Save file to your workspace location using name suggested, or choose a new location/filename..", suggested_name, "Video (*.avi)");

        if (file_name.isEmpty())
            return;

        bool file_opened = video_display->StartRecording(file_name, playback_controller->get_fps());

        if (file_opened) {

            btn_frame_record->setIcon(QIcon(":icons/stop.png"));
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
        // btn_calculate_radiance->setChecked(false);
    }
    else {
        video_display->ToggleActionZoom(false);
    }

}


// void SirveApp::HandleCalculationOnVideoToggle()
// {

//     bool status_calculation_btn = btn_calculate_radiance->isChecked();

//     if (status_calculation_btn) {

//         video_display->ToggleActionCalculateRadiance(true);
//         btn_zoom->setChecked(false);
//     }
//     else {
//         video_display->ToggleActionCalculateRadiance(false);
//     }
// }

void SirveApp::ClearZoomAndCalculationButtons()
{
    btn_zoom->setChecked(false);
    // btn_calculate_radiance->setChecked(false);
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
        workspace = new Workspace(directory);
        lbl_current_workspace_folder_field->setText(directory);
        config_values.workspace_folder = directory;
        lbl_workspace_name_field->clear();
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

void SirveApp::ExportFrame()
{
    if(playback_controller->is_running())
        playback_controller->StopTimer();

    int nRows = video_display->container.processing_states[0].details.y_pixels;
    int nCols = video_display->container.processing_states[0].details.x_pixels;
    std::vector<uint16_t> original_frame_vector = {video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[video_display->counter].begin(),
            video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[video_display->counter].end()};
    arma::u32_mat frame_matrix = arma::reshape(arma::conv_to<arma::u32_vec>::from(original_frame_vector),nCols,nRows).t();
    QDate today = QDate::currentDate();
    QTime currentTime = QTime::currentTime();;
    QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
    QString current_frame = QString::number(video_display->counter + data_plots->index_sub_plot_xmin + 1);
    QString initial_name = abpimage_file_base_name + "_Frame_" + current_frame + "_" + formattedDate;

    QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save File Name"), config_values.workspace_folder + "/" + initial_name, tr("Data files *.bin"));
    frame_matrix.save(savefile_name.toStdString(),arma::arma_binary);

    if(playback_controller->is_running())
        playback_controller->StartTimer();
}

void SirveApp::ExportFrameRange()
{
    if(playback_controller->is_running())
        playback_controller->StopTimer();

    QDialog dialog(this);
    // Use a layout allowing to have a label next to each field
    QFormLayout form(&dialog);

    // Add some text above the fields
    form.addRow(new QLabel("Frame Range"));

    // Add the lineEdits with their respective labels
    QList<QLineEdit *> fields;

    QLineEdit *lineEdit0 = new QLineEdit(&dialog);
    QString label0 = QString("Start: ").arg(1);
    form.addRow(label0, lineEdit0);
    fields << lineEdit0;

    QLineEdit *lineEdit1 = new QLineEdit(&dialog);
    QString label1 = QString("Stop: ").arg(2);
    form.addRow(label1, lineEdit1);
    fields << lineEdit1;

    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Show the dialog as modal
    if (dialog.exec() == QDialog::Accepted) {
        int nRows = video_display->container.processing_states[0].details.y_pixels;
        int nCols = video_display->container.processing_states[0].details.x_pixels;
        QString start_frame = fields.at(0)->text();
        QString end_frame = fields.at(1)->text();
        int startframe = start_frame.toInt() - 1;
        int endframe = end_frame.toInt() - 1;
        int num_video_frames = endframe - startframe + 1;
        int start_framei = startframe - data_plots->index_sub_plot_xmin;
        int end_framei = start_framei + num_video_frames - 1;
        arma::u32_cube frame_cube(nRows,nCols,num_video_frames);
        int k = 0;
        for (int framei = start_framei; framei < end_framei; framei++){
            std::vector<uint16_t> original_frame_vector = {video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[framei].begin(),
                    video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[framei].end()};
            frame_cube.slice(k) = arma::reshape(arma::conv_to<arma::u32_vec>::from(original_frame_vector),nCols,nRows).t();
            k+=1;
        }

        QDate today = QDate::currentDate();
        QTime currentTime = QTime::currentTime();;
        QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
        QString initial_name = abpimage_file_base_name + "_Frames_" + start_frame + "_" + end_frame + "_" + formattedDate;
        QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save File Name"), config_values.workspace_folder + "/" + initial_name, tr("Data files *.bin"));
        frame_cube.save(savefile_name.toStdString(),arma::arma_binary);
    }

    if(playback_controller->is_running())
        playback_controller->StartTimer();
}

void SirveApp::ExportAllFrames()
{
    if(playback_controller->is_running())
        playback_controller->StopTimer();

    int min_frame = ConvertFrameNumberTextToInt(txt_start_frame->text());
    int max_frame = ConvertFrameNumberTextToInt(txt_stop_frame->text());
	QString start_frame = QString::number(min_frame);
    QString end_frame = QString::number(max_frame);
    int num_video_frames = video_display->container.processing_states[0].details.frames_16bit.size();
    int nRows = video_display->container.processing_states[0].details.y_pixels;
    int nCols = video_display->container.processing_states[0].details.x_pixels;
    arma::u32_cube frame_cube(nRows,nCols,num_video_frames);
    for (int framei = 0; framei < num_video_frames ; framei++){
        std::vector<uint16_t> original_frame_vector = {video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[framei].begin(),
                video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[framei].end()};
        frame_cube.slice(framei) = arma::reshape(arma::conv_to<arma::u32_vec>::from(original_frame_vector),nCols,nRows).t();
    }

    QDate today = QDate::currentDate();
    QTime currentTime = QTime::currentTime();;
    QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
    QString initial_name = abpimage_file_base_name + "_Frames_" + start_frame + "_" + end_frame + "_" + formattedDate;
    QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save File Name"), config_values.workspace_folder + "/" + initial_name, tr("Data files *.bin"));
    frame_cube.save(savefile_name.toStdString(),arma::arma_binary);

    if(playback_controller->is_running())
        playback_controller->StartTimer();
}

void SirveApp::CreateMenuActions()
{
    QIcon on(":/icons/check.png");

    action_load_OSM = new QAction("Load Data");
    action_load_OSM->setStatusTip("Load OSM abpimage file");
    connect(action_load_OSM, &QAction::triggered, this, &SirveApp::HandleAbpFileSelected);

    action_close = new QAction("Close");
    action_close->setStatusTip("Close main window");
    connect(action_close, &QAction::triggered, this, &SirveApp::CloseWindow);

    action_load_workspace = new QAction("Load Workspace File");
    connect(action_load_workspace, &QAction::triggered, this, &SirveApp::LoadWorkspace);
    //connect(workspace, updateWorkspaceFolder, this, &SirveApp::LoadWorkspace);

    action_save_workspace = new QAction("Save Workspace File");
    connect(action_save_workspace, &QAction::triggered, this, &SirveApp::SaveWorkspace);

    action_change_workspace_directory = new QAction("Change Workspace Directory");
    action_change_workspace_directory->setStatusTip("Customize workspace directory so it points to your own folder.");
    connect(action_change_workspace_directory, &QAction::triggered, this, &SirveApp::ChangeWorkspaceDirectory);

    action_export_current_frame = new QAction("Export Current Frame");
    connect(action_export_current_frame, &QAction::triggered, this, &SirveApp::ExportFrame);
    action_export_current_frame->setEnabled(false);

    action_export_frame_range = new QAction("Export Frame Range");
    connect(action_export_frame_range, &QAction::triggered, this, &SirveApp::ExportFrameRange);
    action_export_frame_range->setEnabled(false);

    action_export_all_frames = new QAction("Export All Frames");
    connect(action_export_all_frames, &QAction::triggered, this, &SirveApp::ExportAllFrames);
    action_export_all_frames->setEnabled(false);

    action_set_timing_offset = new QAction("Set Timing Offset");
    action_set_timing_offset->setStatusTip("Set a time offset to apply to collected data");
    connect(action_set_timing_offset, &QAction::triggered, this, &SirveApp::SetDataTimingOffset);

    action_export_tracking_data = new QAction("Export Tracking Data");
    action_export_tracking_data->setStatusTip(tr("Export the track data to file"));
    connect(action_export_tracking_data, &QAction::triggered, this, &SirveApp::ExportPlotData);

    action_about = new QAction("SirveApp");
    connect(action_about, &QAction::triggered, this, &SirveApp::ProvideInformationAbout);

	file_menu = menuBar()->addMenu(tr("&File"));
	file_menu->addAction(action_load_OSM);
	file_menu->addAction(action_close);
	menu_workspace = menuBar()->addMenu(tr("&Workspace"));
	menu_workspace->addAction(action_load_workspace);
	menu_workspace->addAction(action_save_workspace);
	menu_workspace->addAction(action_change_workspace_directory);
    menu_export = menuBar()->addMenu(tr("&Export"));
    menu_export->addAction(action_export_tracking_data);
    menu_export->addAction(action_export_current_frame);
	menu_export->addAction(action_export_frame_range);
	menu_export->addAction(action_export_all_frames);
	menu_settings = menuBar()->addMenu(tr("&Settings"));
	menu_settings->addAction(action_set_timing_offset);
    menu_about = menuBar()->addMenu(tr("&About"));
    menu_about->addAction(action_about);

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
    QString start_frame = txt_start_frame->text();
    QString stop_frame = txt_stop_frame->text();
    QStringList items;
    items << "Export All Data" << "Export Only Selected Data";

    bool ok;
    QString item = QInputDialog::getItem(this, "Export Data", "Select Data to Export", items, 0, false, &ok);

    if (!ok && !item.isEmpty())
        return;

    QString base_track_folder = config_values.workspace_folder;
    QDate today = QDate::currentDate();
    QTime currentTime = QTime::currentTime();

    QString osm_max_frames = QString::number(osm_frames.size());
    QString export_stop_frame = item == "Export All Data" ? osm_max_frames : stop_frame;

    QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
    QString suggested_file_name = base_track_folder + "/track_data_export_frames_" + start_frame + "_" + export_stop_frame + "_" + formattedDate;
    QString new_track_file_name = QFileDialog::getSaveFileName(this, "Select a new file to save the track into", suggested_file_name, "CSV (*.csv)");
    std::string save_path = new_track_file_name.toStdString();

    if (new_track_file_name.size() == 0)
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

void SirveApp::ProvideInformationAbout()
{
    QString message = QString("This version of SIRVE was compiled on %1 at %2.").arg(__DATE__).arg(__TIME__);
    QMessageBox::information(nullptr, "About SirveApp", message);
    qDebug() << "This Version of SIRVE was built on: " << __DATE__ " at " << __TIME__ << ".";
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

void SirveApp::EditOSMTrackColor()
{
    // QString tracker_color = cmb_OSM_track_color->currentText();
    QStringList color_options = ColorScheme::get_track_colors();
    QColor color = color_options[cmb_OSM_track_color->currentIndex()];
    video_display->HandleTrackerColorUpdate(color);
    double xmax = data_plots->axis_x->max();
    double xmin = data_plots->axis_x->min();
    double ymax = data_plots->axis_y->max();
    double ymin = data_plots->axis_y->min();
    data_plots->RecolorOsmTrack(color);
    data_plots->set_xaxis_limits(xmin,xmax);
    data_plots->set_yaxis_limits(ymin,ymax);
    // UpdatePlots(); //Note: Engineering_Plots does not yet control its own graphical updates like VideoDisplay
}

void SirveApp::handle_outlier_processing_change()
{
    if(cmb_outlier_processing_type->currentIndex() == 0){
        txt_moving_median_N->setEnabled(false);
        txt_moving_median_N->setStyleSheet("#txt_moving_median_N {background-color:#f0f0f0; color:rgb(75,75,75);}");
    }
    else{
        txt_moving_median_N->setEnabled(true);
        txt_moving_median_N->setStyleSheet("#txt_moving_median_N {background-color:#ffffff; color:rgb(0,0,0);}");
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

        // Feed the current Y-Axis Linear/Log option into the plot engineering widget
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

        emit enableYAxisOptions(y_index == 0);

        data_plots->SetXAxisChartId(x_index);
        data_plots->SetYAxisChartId(y_index);
        data_plots->PlotChart();

        data_plots->PlotCurrentStep(playback_controller->get_current_frame_number());
    }

    if (osmDataLoaded == true)
    {
        for (int id : this->track_info->get_manual_track_ids())
        {
            QLineSeries *trackSeries = new QLineSeries();
            trackSeries->setName("Track " + QString::number(id));

            QWidget * existing_track_control = tm_widget->findChild<QWidget*>(QString("TrackControl_%1").arg(id));
            if (existing_track_control != nullptr)
            {
                QComboBoxWithId *cmb_box = existing_track_control->findChild<QComboBoxWithId*>();

                QPen pen;
                QStringList color_options = ColorScheme::get_track_colors();
                int index = cmb_box->currentIndex();
                QColor trackColor = color_options[index];
                pen.setColor(trackColor);
                pen.setStyle(Qt::SolidLine);
                pen.setWidth(3);

                trackSeries->setPen(pen);

                trackSeries->append(0, 0);
                trackSeries->append(0, 0);

                data_plots->chart->addSeries(trackSeries);
            }
        }
    }

    yAxisChanged = false;
}

void SirveApp::AnnotateVideo()
{
    VideoInfo standard_info;
    standard_info.x_pixels = video_display->image_x;
    standard_info.y_pixels = video_display->image_y;

    standard_info.min_frame = data_plots->index_sub_plot_xmin + 1;
    standard_info.max_frame = data_plots->index_sub_plot_xmax + 1;

    standard_info.x_correction = video_display->xCorrection;
    standard_info.y_correction = video_display->yCorrection;

    annotate_gui = new AnnotationListDialog(video_display->annotation_list, standard_info);

    connect(annotate_gui, &AnnotationListDialog::showAnnotationStencil, video_display, &VideoDisplay::ShowStencil);
    connect(annotate_gui, &AnnotationListDialog::hideAnnotationStencil, video_display, &VideoDisplay::HideStencil);
    connect(annotate_gui, &AnnotationListDialog::updateAnnotationStencil, video_display, &VideoDisplay::InitializeStencilData);
    connect(annotate_gui, &AnnotationListDialog::rejected, this, &SirveApp::HandleAnnotationDialogClosed);
    connect(annotate_gui, &AnnotationListDialog::accepted, this, &SirveApp::HandleAnnotationDialogClosed);

    connect(video_display->annotation_stencil, &AnnotationStencil::mouseMoved, annotate_gui, &AnnotationListDialog::UpdateStencilPosition, Qt::UniqueConnection);
    connect(video_display->annotation_stencil, &AnnotationStencil::mouseReleased, annotate_gui, &AnnotationListDialog::SetStencilLocation, Qt::UniqueConnection);

    btn_add_annotations->setDisabled(true);

    annotate_gui->show();
}

void SirveApp::HandleAnnotationDialogClosed()
{
    btn_add_annotations->setDisabled(false);
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
    data_plots->InitializeIntervals(osm_frames);
    UpdatePlots();
}

void SirveApp::HandleBadPixelReplacement()
{
    processingState new_state = video_display->container.processing_states[cmb_processing_states->currentIndex()];
    std::vector<std::vector<uint16_t>> test_data;
    int number_video_frames = static_cast<int>(new_state.details.frames_16bit.size());

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
    int stop_frame = txt_bad_pixel_stop_frame->text().toInt();

    if (chk_bad_pixels_from_original->isChecked()){
        if (stop_frame > osm_frames.size() ||\
         start_frame >= stop_frame){         
            QtHelpers::LaunchMessageBox(QString("Invalid frame range."), "Max frame: " + QString::number(osm_frames.size()) + ". Stop must be greater than start. Recommend the number of sample frames must be less <= 2000.");
            return;
        }
        file_processor->LoadImageFile(abp_file_metadata.image_path, start_frame, stop_frame, config_values.version);
        ABIRDataResult test_frames = *file_processor->getAbirDataLoadResult();
        test_data = test_frames.video_frames_16bit;
    }
    else{
        if (stop_frame > txt_stop_frame->text().toInt() ||\
         start_frame < txt_start_frame->text().toInt() || \
         start_frame >= stop_frame){         
            QtHelpers::LaunchMessageBox(QString("Invalid frame range."), "Min frame: " + txt_start_frame->text() + ". Max frame: " + txt_stop_frame->text() + ". Stop must be greater than start. Recommend the number of sample frames must be less <= 2000.");
            return;
        } 
        int start_offset = start_frame - txt_start_frame->text().toInt();
        int stop_offset = stop_frame - txt_start_frame->text().toInt();
        test_data = {new_state.details.frames_16bit.begin()+ start_offset,new_state.details.frames_16bit.begin()+stop_offset};
    }

    OpenProgressArea("Finding bad pixels",number_video_frames - 1);

    std::vector<unsigned int> dead_pixels;
    ImageProcessing *ImageProcessor = new ImageProcessing();

    connect(ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, ImageProcessor, &ImageProcessing::CancelOperation);

    if(type_choice == 0){
        lbl_progress_status->setText(QString("Finding dead pixels..."));
        arma::uvec index_dead0 = ImageProcessor->FindDeadBadscalePixels(test_data);
        lbl_progress_status->setText(QString("Finding outlier pixels..."));
        if (outlier_method == 0){
            arma::uvec index_outlier0 = ImageProcessor->IdentifyBadPixelsMedian(N,test_data);
            index_outlier0 = arma::unique(arma::join_vert(index_outlier0,index_dead0));
            dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier0);
        }
        else{
            u_int window_length = txt_moving_median_N->text().toUInt();
            arma::uvec index_outlier0 = ImageProcessor->IdentifyBadPixelsMovingMedian(window_length,N,test_data);
            index_outlier0 = arma::unique(arma::join_vert(index_outlier0,index_dead0));
            dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier0);
        }
    } else if (type_choice == 1){
        lbl_progress_status->setText(QString("Finding outlier pixels..."));
        arma::uvec index_dead1 = ImageProcessor->FindDeadBadscalePixels(test_data);
        dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_dead1);
    } else {
        lbl_progress_status->setText(QString("Finding outlier pixels..."));
        if (outlier_method == 0){
            arma::uvec index_outlier2 = ImageProcessor->IdentifyBadPixelsMedian(N,test_data);
            dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier2);
        }
        else{
            u_int window_length = txt_moving_median_N->text().toUInt();
            arma::uvec index_outlier2 = ImageProcessor->IdentifyBadPixelsMovingMedian(window_length,N,test_data);
            dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier2);
        }
    }

    if(dead_pixels.size()>0){
        ReplaceBadPixels(dead_pixels,cmb_processing_states->currentIndex());
    }

    CloseProgressArea();

    ImageProcessor->deleteLater();
}


void SirveApp::ReceiveNewBadPixels(std::vector<unsigned int> new_pixels)
{
    int source_state_idx = video_display->container.processing_states[video_display->container.current_idx].source_state_ID;
    int current_state_idx = video_display->container.processing_states[video_display->container.current_idx].state_ID;
    ProcessingMethod method = video_display->container.processing_states[video_display->container.current_idx].method;
    if (method == ProcessingMethod::replace_bad_pixels)
    //If current state is already a new state with replaced pixels, just add new pixels to the replaced pixels
    {
        std::vector<unsigned int> bad_pixels = video_display->container.processing_states[video_display->container.current_idx].replaced_pixels;

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
            ImageProcessing *ImageProcessor = new ImageProcessing();
            ImageProcessor->ReplacePixelsWithNeighbors(video_display->container.processing_states[current_state_idx].details.frames_16bit, new_pixels, video_display->container.processing_states[current_state_idx].details.x_pixels);
            uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
            for (const auto& row : video_display->container.processing_states[current_state_idx].details.frames_16bit) {
                maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
            }
            video_display->container.processing_states[current_state_idx].details.max_value = maxVal;
            video_display->container.processing_states[current_state_idx].replaced_pixels = bad_pixels;
            lbl_bad_pixel_count->setText("Bad pixels currently replaced: " + QString::number(bad_pixels.size()));
            QString state_name = "State " + QString::number(current_state_idx) + ": " + video_display->container.processing_states[current_state_idx].get_friendly_description();
	        video_display->container.processing_states[current_state_idx].state_description = state_name;
            lbl_processing_description->setText(state_name);
            ImageProcessor->deleteLater();
            UpdateGlobalFrameVector();
        }
    }
    else
    //Create a new replaced pixel state
    {
        ReplaceBadPixels(new_pixels, current_state_idx);
    }
}

void SirveApp::ReplaceBadPixels(std::vector<unsigned int> & pixels_to_replace,int source_state_idx)
{
    video_display->container.processing_states.push_back(video_display->container.processing_states[source_state_idx]);
    int endi = video_display->container.processing_states.size()-1;
    video_display->container.processing_states[endi].method = ProcessingMethod::replace_bad_pixels;
    int number_video_frames = static_cast<int>(video_display->container.processing_states[source_state_idx].details.frames_16bit.size());
    video_display->container.processing_states[endi].replaced_pixels = pixels_to_replace;
    OpenProgressArea("Replacing bad pixels...",number_video_frames - 1);

    ImageProcessing *ImageProcessor = new ImageProcessing();
    connect(ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, ImageProcessor, &ImageProcessing::CancelOperation);

    ImageProcessor->ReplacePixelsWithNeighbors(video_display->container.processing_states[endi].details.frames_16bit, pixels_to_replace, video_display->container.processing_states[endi].details.x_pixels);

    if(pixels_to_replace.size()>0){

        // fetch max value
        video_display->container.processing_states[endi].source_state_ID = source_state_idx;
        uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
        for (const auto& row : video_display->container.processing_states[endi].details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        video_display->container.processing_states[endi].details.max_value = maxVal;
        video_display->container.processing_states[endi].state_ID = video_display->container.processing_states.size() - 1;
        video_display->container.processing_states[source_state_idx].descendants.push_back(video_display->container.processing_states[endi].state_ID);
        video_display->container.processing_states[source_state_idx].descendants = GetUniqueIntegerVector(video_display->container.processing_states[source_state_idx].descendants);
        video_display->container.processing_states[endi].ancestors = video_display->container.processing_states[source_state_idx].ancestors;
        video_display->container.processing_states[endi].ancestors.push_back(source_state_idx);

        // update state gui status
        std::string result;
        for (auto num : video_display->container.processing_states[endi].ancestors) {
            result += std::to_string(num) + " -> ";
        }
        result += std::to_string(video_display->container.processing_states[endi].state_ID);
        QString state_steps = QString::fromStdString(result);
        video_display->container.processing_states[endi].state_steps = state_steps;
        video_display->container.processing_states[endi].process_steps.push_back(" [Replace Bad Pixels] ");
        QString state_name = "State " + QString::number(endi) + ": " + video_display->container.processing_states[endi].get_friendly_description();
	    QString combobox_state_name = QString::number(endi) + ": " + video_display->container.processing_states[endi].get_combobox_description();
	    video_display->container.processing_states[endi].state_description = state_name;
        HandleNewProcessingState(state_name, combobox_state_name, endi);
        UpdateGlobalFrameVector();
        lbl_bad_pixel_count->setText("Bad pixels currently replaced: " + QString::number(pixels_to_replace.size()));
        chk_highlight_bad_pixels->setEnabled(true);
    }

    CloseProgressArea();

    ImageProcessor->deleteLater();
}

void SirveApp::ReceiveNewGoodPixels(std::vector<unsigned int> pixels)
{
    bool isReplacedBadPixelState = false;
    int source_state_idx = video_display->container.processing_states[video_display->container.current_idx].source_state_ID;
    int current_state_idx = video_display->container.processing_states[video_display->container.current_idx].state_ID;
    ProcessingMethod method = video_display->container.processing_states[video_display->container.current_idx].method;
    if (method == ProcessingMethod::replace_bad_pixels)
    //If current state is already a new state with replaced pixels, just replace pixels from source state
    {
        std::vector<unsigned int> bad_pixels = video_display->container.processing_states[current_state_idx].replaced_pixels;
        int num_video_frames = video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit.size();
        for (auto i = 0; i < pixels.size(); i++)
        {
            unsigned int candidate_pixel = pixels[i];
            std::vector<unsigned int>::iterator position = std::find(bad_pixels.begin(), bad_pixels.end(), candidate_pixel);
            if (position != bad_pixels.end())
            {
                bad_pixels.erase(position);
                for (int framei = 0; framei < num_video_frames; framei++)
                {
                    video_display->container.processing_states[current_state_idx].details.frames_16bit[framei][pixels[i]] =\
                    video_display->container.processing_states[source_state_idx].details.frames_16bit[framei][pixels[i]];
                }
                
            }
        }
        video_display->container.processing_states[current_state_idx].replaced_pixels = bad_pixels;
        lbl_bad_pixel_count->setText("Bad pixels currently replaced: " + QString::number(bad_pixels.size()));
        QString state_name = "State " + QString::number(current_state_idx) + ": " + video_display->container.processing_states[current_state_idx].get_friendly_description();
        video_display->container.processing_states[current_state_idx].state_description = state_name;
        lbl_processing_description->setText(state_name);
        UpdateGlobalFrameVector();
    }
    else
    {
        QtHelpers::LaunchMessageBox("No Action Taken", "Pixels can only be recovered from 'Replace Bad Pixels' states.");
    }

}

void SirveApp::ReceiveProgressBarUpdate(int percent)
{
    progress_bar_main->setValue(percent);
}

bool SirveApp::CheckCurrentStateisNoiseSuppressed(int source_state_idx)
{
    std::set<ProcessingMethod> test_set = {ProcessingMethod::fixed_noise_suppression, ProcessingMethod::accumulator_noise_suppression, ProcessingMethod::adaptive_noise_suppression,ProcessingMethod::RPCP_noise_suppression};
    ProcessingMethod currentMethod = video_display->container.processing_states[source_state_idx].method;
    if (test_set.count(currentMethod) > 0) {
        return true;
    } else {
        return false;
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
    unsigned int stop_frame = external_nuc_dialog.stop_frame;
    txt_FNS_start_frame->setText(QString::number(start_frame));
    txt_FNS_stop_frame->setText(QString::number(stop_frame));
    try
    {
        // assumes file version is same as base file opened
        int source_state_idx = cmb_processing_states->currentIndex();
        bool continueTF = true;
        if (CheckCurrentStateisNoiseSuppressed(source_state_idx))
        {
            auto response = QtHelpers::LaunchYesNoMessageBox("Current state is already noise suppressed.", "Continue?");
            if (response != QMessageBox::Yes)
                {
                    continueTF = false;
                }
        }
        if (continueTF)
        {       
            int frame0 = start_frame - 1;
            ApplyFixedNoiseSuppression(abp_file_metadata.image_path, image_path, frame0, start_frame, stop_frame, source_state_idx);
        }
    }
    catch (const std::exception& e)
    {
        // catch any errors when loading frames. try-catch not needed when loading frames from same file since no errors originally occurred
        //TODO: LAUNCHMESSAGEBOX
        QMessageBox msgBox;
        msgBox.setWindowTitle(QString("Fixed Noise Suppression from External File"));
        QString box_text = "Error occurred when loading the frames for the noise suppression. See log for details.  ";
        msgBox.setText(box_text);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }
}

void SirveApp::ExecuteFixedNoiseSuppression()
{
    // Pause the video if it's running
    playback_controller->StopTimer();

    int source_state_idx = cmb_processing_states->currentIndex();
    bool continueTF = true;
    if (CheckCurrentStateisNoiseSuppressed(source_state_idx))
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Current state is already noise suppressed.", "Continue?");
        if (response != QMessageBox::Yes)
            {
                continueTF = false;
            }
    }
    if (continueTF)
    {
        if (!chk_FNS_external_file->isChecked())
        {       
            if (txt_FNS_stop_frame->text().toInt() > txt_stop_frame->text().toInt() ||\
            txt_FNS_start_frame->text().toInt() < txt_start_frame->text().toInt() || \
            txt_FNS_start_frame->text().toInt() >= txt_FNS_stop_frame->text().toInt()){         
                QtHelpers::LaunchMessageBox(QString("Invalid frame range."), "Min frame: " + txt_start_frame->text() + ". Max frame: " +txt_stop_frame->text() + ".");
                return;
            }
            
            int start_frame = txt_FNS_start_frame->text().toInt();
            int stop_frame = txt_FNS_stop_frame->text().toInt();
            int frame0 = txt_start_frame->text().toInt();
            ApplyFixedNoiseSuppression(abp_file_metadata.image_path, abp_file_metadata.image_path, frame0, start_frame, stop_frame, source_state_idx);
        }
        else
        {
            ApplyFixedNoiseSuppressionFromExternalFile();
        }
    }

}

void SirveApp::ApplyFixedNoiseSuppression(QString image_path, QString file_path, unsigned int frame0, unsigned int start_frame, unsigned int stop_frame, int source_state_idx)
{
    int compare = QString::compare(file_path, image_path, Qt::CaseInsensitive);
    if (compare!=0){
        if (!VerifyFrameSelection(start_frame, stop_frame))
        {
            QtHelpers::LaunchMessageBox(QString("Invalid Frame Selection"), "Fixed noise suppression not completed, invalid frame selection");
            return;
        }
    }

    int number_video_frames = static_cast<int>(video_display->container.processing_states[source_state_idx].details.frames_16bit.size());
    ImageProcessing *ImageProcessor = new ImageProcessing();
    OpenProgressArea("Fixed median noise suppression...",number_video_frames - 1);

    connect(ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, ImageProcessor, &ImageProcessing::CancelOperation);
    video_display->container.processing_states.push_back(video_display->container.processing_states[source_state_idx]);
    int endi = video_display->container.processing_states.size()-1;
    video_display->container.processing_states[endi].details.frames_16bit = ImageProcessor->FixedNoiseSuppression(abp_file_metadata.image_path, file_path, frame0, start_frame, stop_frame, config_values.version, video_display->container.processing_states[source_state_idx].details);

    if(video_display->container.processing_states[endi].details.frames_16bit.size()>0){

        // set new state
        video_display->container.processing_states[endi].method = ProcessingMethod::fixed_noise_suppression;
        video_display->container.processing_states[endi].FNS_file_path = file_path;
        video_display->container.processing_states[endi].frame0 = frame0;
        video_display->container.processing_states[endi].FNS_start_frame = start_frame;
        video_display->container.processing_states[endi].FNS_stop_frame = stop_frame;
        video_display->container.processing_states[endi].source_state_ID = source_state_idx;

        // fetch max value
        uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
        for (const auto& row : video_display->container.processing_states[endi].details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        video_display->container.processing_states[endi].details.max_value = maxVal;
        video_display->container.processing_states[endi].state_ID = video_display->container.processing_states.size() - 1;
        video_display->container.processing_states[source_state_idx].descendants.push_back(video_display->container.processing_states[endi].state_ID);
        video_display->container.processing_states[source_state_idx].descendants = GetUniqueIntegerVector(video_display->container.processing_states[source_state_idx].descendants);
        video_display->container.processing_states[endi].ancestors = video_display->container.processing_states[source_state_idx].ancestors;
        video_display->container.processing_states[endi].ancestors.push_back(source_state_idx);
        std::string result;
        for (auto num : video_display->container.processing_states[endi].ancestors) {
            result += std::to_string(num) + " -> ";
        }

        // update state gui status
        result += std::to_string(video_display->container.processing_states[endi].state_ID);
        QString state_steps = QString::fromStdString(result);
        video_display->container.processing_states[endi].state_steps = state_steps;
        video_display->container.processing_states[endi].process_steps.push_back(" [Fixed Noise Suppression] ");
        QString state_name = "State " + QString::number(endi) + ": " + video_display->container.processing_states[endi].get_friendly_description();
	    QString combobox_state_name = QString::number(endi) + ": " +video_display->container.processing_states[endi].get_combobox_description();
	    video_display->container.processing_states[endi].state_description = state_name;
        HandleNewProcessingState(state_name, combobox_state_name, endi);
        UpdateGlobalFrameVector();

        QFileInfo fi(file_path);
        QString fileName = fi.fileName().toLower();
        QString current_filename = abp_file_metadata.file_name.toLower() + ".abpimage";

        if (fileName == current_filename)
            fileName = "Current File";

        QString description = "File: " + fileName + "\n";
        description += "From frame " + QString::number(start_frame) + " to " + QString::number(stop_frame);

        lbl_fixed_suppression->setText(description);
    }
    CloseProgressArea();

    ImageProcessor->deleteLater();
}

void SirveApp::ExecuteDeinterlace()
{
    int source_state_idx = cmb_processing_states->currentIndex();
    ApplyDeinterlacing(source_state_idx);
}

void SirveApp::ExecuteDeinterlaceCurrent()
{
    ApplyDeinterlacingCurrent();
}

void SirveApp::OpenProgressArea(QString message, int N)
{
    grpbox_progressbar_area->setEnabled(true);
    progress_bar_main->setRange(0,N);
    progress_bar_main->setTextVisible(true);
    lbl_progress_status->setText(message);   
}

void SirveApp::CloseProgressArea()
{
    grpbox_progressbar_area->setEnabled(false);
    progress_bar_main->setTextVisible(false);
    lbl_progress_status->setText(QString(""));
    progress_bar_main->setValue(0);
}

void SirveApp::ApplyDeinterlacing(int source_state_idx)
{
    // Apply de-interlace to the frames
    video_display->container.processing_states.push_back(video_display->container.processing_states[source_state_idx]);
    int endi = video_display->container.processing_states.size()-1;
    int number_video_frames = static_cast<int>(video_display->container.processing_states[source_state_idx].details.frames_16bit.size());

    ImageProcessing *ImageProcessor = new ImageProcessing();
    OpenProgressArea("Deinterlacing...", number_video_frames - 1);

    connect(ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, ImageProcessor, &ImageProcessing::CancelOperation);
    
    video_display->container.processing_states[endi].details.frames_16bit = ImageProcessor->DeinterlaceOpenCVPhaseCorrelation(osm_frames,video_display->container.processing_states[source_state_idx].details);

    if(video_display->container.processing_states[endi].details.frames_16bit.size()>0){

        // set new state ...
        progress_bar_main->setTextVisible(false);
        video_display->container.processing_states[endi].method = ProcessingMethod::deinterlace;
        video_display->container.processing_states[endi].source_state_ID = source_state_idx;

        // fetch max value
        uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
        for (const auto& row : video_display->container.processing_states[endi].details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }

        video_display->container.processing_states[endi].details.max_value = maxVal;
        video_display->container.processing_states[endi].state_ID = video_display->container.processing_states.size() - 1;
        video_display->container.processing_states[source_state_idx].descendants.push_back(video_display->container.processing_states[endi].state_ID);
        video_display->container.processing_states[source_state_idx].descendants = GetUniqueIntegerVector(video_display->container.processing_states[source_state_idx].descendants);
        video_display->container.processing_states[endi].ancestors = video_display->container.processing_states[source_state_idx].ancestors;
        video_display->container.processing_states[endi].ancestors.push_back(source_state_idx);

        // update state gui status
        std::string result;
        for (auto num : video_display->container.processing_states[endi].ancestors) {
            result += std::to_string(num) + " -> ";
        }
        result += std::to_string(video_display->container.processing_states[endi].state_ID);
        QString state_steps = QString::fromStdString(result);
        video_display->container.processing_states[endi].state_steps = state_steps;
        video_display->container.processing_states[endi].process_steps.push_back(" [Deinterlace] ");
        QString state_name = "State " + QString::number(endi) + ": " + video_display->container.processing_states[endi].get_friendly_description();
	    QString combobox_state_name = QString::number(endi) + ": " +video_display->container.processing_states[endi].get_combobox_description();
	    video_display->container.processing_states[endi].state_description = state_name;
        HandleNewProcessingState(state_name, combobox_state_name, endi);
        UpdateGlobalFrameVector();
    }

    CloseProgressArea();

    ImageProcessor->deleteLater();
}

void SirveApp::ApplyDeinterlacingCurrent()
{
    processingState original = video_display->container.CopyCurrentStateIdx(cmb_processing_states->currentIndex());
    ImageProcessing *ImageProcessor = new ImageProcessing();
    lbl_progress_status->setText(QString("Deinterlacing..."));
    int framei = video_display->counter;
    std::vector<uint16_t> current_frame_16bit = original.details.frames_16bit[framei];
    std::vector<uint16_t> current_frame_16bit_0 = current_frame_16bit;
    int nRows = original.details.y_pixels;
    int nCols = original.details.x_pixels;

    video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[video_display->counter] = ImageProcessor->DeinterlacePhaseCorrelationCurrent(framei, nRows, nCols, current_frame_16bit);
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
    QString trackFeaturePriority;
    bool continueTF = true;
    if (cmb_track_centering_priority->currentIndex()==0 || cmb_track_centering_priority->currentIndex()==2){
        if (cmb_OSM_track_IDs->currentIndex()==0){
            track_id = -1;
        }
        else{
            track_id = cmb_OSM_track_IDs->currentText().toInt();
        }
        trackFeaturePriority = "OSM";
    }
    else if(cmb_track_centering_priority->currentIndex()==1 || cmb_track_centering_priority->currentIndex()==3){
        if (cmb_manual_track_IDs->currentIndex()==0){
            track_id = -1;
        }
        else{
            track_id = cmb_manual_track_IDs->currentText().toInt();
        }
        trackFeaturePriority = "Manual";
    }
    if(cmb_track_centering_priority->currentIndex()==2 || cmb_track_centering_priority->currentIndex()==3){
        findAnyTrack = true;
    }

    std::vector<std::vector<int>> track_centered_offsets;
    int source_state_idx = cmb_processing_states->currentIndex();
    if (video_display->container.processing_states[source_state_idx].offsets.size()>0)
    {
        QtHelpers::LaunchMessageBox(QString("Not allowed"), "Current state is already centered.");            
        continueTF = false;
    }
    if (continueTF)
    { 
        CenterOnTracks(trackFeaturePriority, track_id, track_centered_offsets, findAnyTrack, source_state_idx);
    }
}

void SirveApp::CenterOnTracks(QString trackFeaturePriority, int track_id, std::vector<std::vector<int>> & track_centered_offsets, boolean find_any_tracks, int source_state_idx)
{
    int OSMPriority = QString::compare(trackFeaturePriority,"OSM",Qt::CaseInsensitive);

    // set new state ...
    video_display->container.processing_states.push_back(video_display->container.processing_states[source_state_idx]);
    int endi = video_display->container.processing_states.size()-1;
    int number_video_frames = static_cast<int>(video_display->container.processing_states[source_state_idx].details.frames_16bit.size());
    int min_frame = ConvertFrameNumberTextToInt(txt_start_frame->text());
    int max_frame = ConvertFrameNumberTextToInt(txt_stop_frame->text());
    std::vector<TrackFrame> osmFrames = track_info->get_osm_frames(min_frame - 1, max_frame);
    std::vector<TrackFrame> manualFrames = track_info->get_manual_frames(min_frame - 1, max_frame);
    video_display->container.processing_states[endi].track_id = track_id;
    if (OSMPriority==0){
        video_display->container.processing_states[endi].method = ProcessingMethod::center_on_OSM;
    }
    else{

        video_display->container.processing_states[endi].method = ProcessingMethod::center_on_manual;
    }
    video_display->container.processing_states[endi].find_any_tracks = find_any_tracks;

    ImageProcessing *ImageProcessor = new ImageProcessing();
    OpenProgressArea("Centering on tracks...",number_video_frames - 1);;

    connect(ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, ImageProcessor, &ImageProcessing::CancelOperation);

    video_display->container.processing_states[endi].details.frames_16bit = ImageProcessor->CenterOnTracks(trackFeaturePriority, video_display->container.processing_states[source_state_idx].details, track_id, osmFrames, manualFrames, find_any_tracks, track_centered_offsets);

    if (video_display->container.processing_states[endi].details.frames_16bit.size()>0){
        video_display->container.processing_states[endi].offsets = track_centered_offsets;
        video_display->container.processing_states[endi].source_state_ID = source_state_idx;

        // fetch max value
        uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
        for (const auto& row : video_display->container.processing_states[endi].details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        video_display->container.processing_states[endi].details.max_value = maxVal;
        video_display->container.processing_states[endi].state_ID = video_display->container.processing_states.size() - 1;
        video_display->container.processing_states[source_state_idx].descendants.push_back(video_display->container.processing_states[endi].state_ID);
        video_display->container.processing_states[source_state_idx].descendants = GetUniqueIntegerVector(video_display->container.processing_states[source_state_idx].descendants);
        video_display->container.processing_states[endi].ancestors = video_display->container.processing_states[source_state_idx].ancestors;
        video_display->container.processing_states[endi].ancestors.push_back(source_state_idx);

        // update state gui status
        std::string result;
        for (auto num : video_display->container.processing_states[endi].ancestors) {
            result += std::to_string(num) + " -> ";
        }
        result += std::to_string(video_display->container.processing_states[endi].state_ID);
        QString state_steps = QString::fromStdString(result);
        video_display->container.processing_states[endi].state_steps = state_steps;
        video_display->container.processing_states[endi].process_steps.push_back(" [Center on Tracks] ");
        QString state_name = "State " + QString::number(endi) + ": " + video_display->container.processing_states[endi].get_friendly_description();
	    QString combobox_state_name = QString::number(endi) + ": " +video_display->container.processing_states[endi].get_combobox_description();
	    video_display->container.processing_states[endi].state_description = state_name;
        HandleNewProcessingState(state_name, combobox_state_name, endi);
        UpdateGlobalFrameVector();
    }
    CloseProgressArea();

    ImageProcessor->deleteLater();
}

void SirveApp::CenterOnOffsets(QString trackFeaturePriority, int track_id, std::vector<std::vector<int>> & track_centered_offsets, boolean find_any_tracks, int source_state_idx)
{
    int OSMPriority = QString::compare(trackFeaturePriority,"OSM",Qt::CaseInsensitive);

    // set new state ...
    video_display->container.processing_states.push_back(video_display->container.processing_states[source_state_idx]);
    int endi = video_display->container.processing_states.size()-1;
    int number_video_frames = static_cast<int>(video_display->container.processing_states[source_state_idx].details.frames_16bit.size());
    int min_frame = ConvertFrameNumberTextToInt(txt_start_frame->text());
    int max_frame = ConvertFrameNumberTextToInt(txt_stop_frame->text());
    video_display->container.processing_states[endi].track_id = track_id;
    if (OSMPriority==0){
        video_display->container.processing_states[endi].method = ProcessingMethod::center_on_OSM;
    }
    else{

        video_display->container.processing_states[endi].method = ProcessingMethod::center_on_manual;
    }
    video_display->container.processing_states[endi].find_any_tracks = find_any_tracks;

    ImageProcessing *ImageProcessor = new ImageProcessing();
    OpenProgressArea("Centering on offsets...",number_video_frames - 1);

    connect(ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, ImageProcessor, &ImageProcessing::CancelOperation);

    video_display->container.processing_states[endi].details.frames_16bit = ImageProcessor->CenterImageFromOffsets(video_display->container.processing_states[source_state_idx].details, track_centered_offsets);

    if (video_display->container.processing_states[endi].details.frames_16bit.size()>0){
        video_display->container.processing_states[endi].offsets = track_centered_offsets;
        video_display->container.processing_states[endi].source_state_ID = source_state_idx;

        // fetch max value
        uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
        for (const auto& row : video_display->container.processing_states[endi].details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        video_display->container.processing_states[endi].details.max_value = maxVal;
        video_display->container.processing_states[endi].state_ID = video_display->container.processing_states.size() - 1;
        video_display->container.processing_states[source_state_idx].descendants.push_back(video_display->container.processing_states[endi].state_ID);
        video_display->container.processing_states[source_state_idx].descendants = GetUniqueIntegerVector(video_display->container.processing_states[source_state_idx].descendants);
        video_display->container.processing_states[endi].ancestors = video_display->container.processing_states[source_state_idx].ancestors;
        video_display->container.processing_states[endi].ancestors.push_back(source_state_idx);

        // update state gui status
        std::string result;
        for (auto num : video_display->container.processing_states[endi].ancestors) {
            result += std::to_string(num) + " -> ";
        }
        result += std::to_string(video_display->container.processing_states[endi].state_ID);
        QString state_steps = QString::fromStdString(result);
        video_display->container.processing_states[endi].state_steps = state_steps;
        video_display->container.processing_states[endi].process_steps.push_back(" [Center on Tracks] ");
        QString state_name = "State " + QString::number(endi) + ": " + video_display->container.processing_states[endi].get_friendly_description();
	    QString combobox_state_name = QString::number(endi) + ": " +video_display->container.processing_states[endi].get_combobox_description();
	    video_display->container.processing_states[endi].state_description = state_name;
        HandleNewProcessingState(state_name, combobox_state_name, endi);
        UpdateGlobalFrameVector();
    }
    CloseProgressArea();

    ImageProcessor->deleteLater();
}

void SirveApp::ExecuteCenterOnBrightest()
{
    std::vector<std::vector<int>> brightest_centered_offsets;
    int source_state_idx = cmb_processing_states->currentIndex();
    bool continueTF = true;

    if (video_display->container.processing_states[source_state_idx].offsets.size()>0)
    {
        QtHelpers::LaunchMessageBox(QString("Not allowed"), "Current state is already centered.");            
        continueTF = false;
    }
    if (continueTF)
    {       
        CenterOnBrightest(brightest_centered_offsets,source_state_idx);
    }
}

void SirveApp::CenterOnBrightest(std::vector<std::vector<int>> & brightest_centered_offsets, int source_state_idx)
{
    video_display->container.processing_states.push_back(video_display->container.processing_states[source_state_idx]);
    int endi = video_display->container.processing_states.size()-1;
    int number_video_frames = static_cast<int>(video_display->container.processing_states[source_state_idx].details.frames_16bit.size());
    video_display->container.processing_states[endi].method = ProcessingMethod::center_on_brightest;
    OpenProgressArea("Centering on brightest...",number_video_frames - 1);
    lbl_progress_status->setText(QString("Center on Brightest Object..."));
    ImageProcessing ImageProcessor;

    connect(&ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, &ImageProcessor, &ImageProcessing::CancelOperation);

    video_display->container.processing_states[endi].details.frames_16bit = ImageProcessor.CenterOnBrightest(video_display->container.processing_states[source_state_idx].details, brightest_centered_offsets);

    if (video_display->container.processing_states[endi].details.frames_16bit.size()>0){

        // set new state
        video_display->container.processing_states[endi].offsets = brightest_centered_offsets;
        video_display->container.processing_states[endi].source_state_ID = source_state_idx;

        // fetch max value
        uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
        for (const auto& row : video_display->container.processing_states[endi].details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }

        video_display->container.processing_states[endi].details.max_value = maxVal;
        video_display->container.processing_states[endi].state_ID = video_display->container.processing_states.size() - 1;
        video_display->container.processing_states[source_state_idx].descendants.push_back(video_display->container.processing_states[endi].state_ID);
        video_display->container.processing_states[source_state_idx].descendants = GetUniqueIntegerVector(video_display->container.processing_states[source_state_idx].descendants);
        video_display->container.processing_states[endi].ancestors = video_display->container.processing_states[source_state_idx].ancestors;
        video_display->container.processing_states[endi].ancestors.push_back(source_state_idx);

        // update state gui status
        std::string result;
        for (auto num : video_display->container.processing_states[endi].ancestors) {
            result += std::to_string(num) + " -> ";
        }
        result += std::to_string(video_display->container.processing_states[endi].state_ID);
        QString state_steps = QString::fromStdString(result);
        video_display->container.processing_states[endi].state_steps = state_steps;
        video_display->container.processing_states[endi].process_steps.push_back(" [Center on Brightest] ");
        QString state_name = "State " + QString::number(endi) + ": " + video_display->container.processing_states[endi].get_friendly_description();
	    QString combobox_state_name = QString::number(endi) + ": " +video_display->container.processing_states[endi].get_combobox_description();
	    video_display->container.processing_states[endi].state_description = state_name;
        HandleNewProcessingState(state_name, combobox_state_name, endi);
        UpdateGlobalFrameVector();
    }
    CloseProgressArea();
}

void SirveApp::HandleOsmTracksToggle()
{
    bool current_status = chk_show_OSM_tracks->isChecked();
    video_display->ToggleOsmTracks(current_status);
    if (current_status) {
        cmb_OSM_track_color->setEnabled(true);
        QStringList color_options = ColorScheme::get_track_colors();
        QColor color = color_options[cmb_OSM_track_color->currentIndex()];
        video_display->HandleTrackerColorUpdate(color);
        double xmax = data_plots->axis_x->max();
        double xmin = data_plots->axis_x->min();
        double ymax = data_plots->axis_y->max();
        double ymin = data_plots->axis_y->min();
        data_plots->RecolorOsmTrack(color);
        data_plots->set_xaxis_limits(xmin,xmax);
        data_plots->set_yaxis_limits(ymin,ymax);
    }
    else
    {
        cmb_OSM_track_color->setEnabled(false);
        QColor color = QColor(0,0,0,0);
        video_display->HandleTrackerColorUpdate(color);
        double xmax = data_plots->axis_x->max();
        double xmin = data_plots->axis_x->min();
        double ymax = data_plots->axis_y->max();
        double ymin = data_plots->axis_y->min();
        data_plots->RecolorOsmTrack(color);
        data_plots->set_xaxis_limits(xmin,xmax);
        data_plots->set_yaxis_limits(ymin,ymax);
    }
}

void SirveApp::HandlePlayerStateChanged(bool status)
{
    txt_start_frame->setDisabled(status);
    txt_stop_frame->setDisabled(status);
    btn_get_frames->setDisabled(status);
}

void SirveApp::HandleNewProcessingState(QString state_name, QString combobox_state_name, int index)
{
    cmb_processing_states->addItem(combobox_state_name);
    lbl_processing_description->setText(state_name);
    cmb_processing_states->setCurrentIndex(index);

    btn_undo_step->setEnabled(cmb_processing_states->count() > 1);
    btn_delete_state->setEnabled(cmb_processing_states->count()> 1);
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

    btn_undo_step->setEnabled(cmb_processing_states->count() > 1);
    btn_delete_state->setEnabled(cmb_processing_states->count() > 1);
}

void SirveApp::HandleProcessingNewStateSelected()
{
    if (cmb_processing_states->currentIndex() == -1)
	{
		return;
	}
    processingState state = video_display->container.processing_states[cmb_processing_states->currentIndex()];
    lbl_processing_description->setText(state.state_description);
    int num_pixels_replaced = state.replaced_pixels.size();
    ProcessingMethod procMethod = state.method;
    if (num_pixels_replaced>0) {
        lbl_bad_pixel_count->setText("Bad pixels currently replaced: " + QString::number(num_pixels_replaced));
    }
    if (procMethod == ProcessingMethod::fixed_noise_suppression){
        QString description = "File: " + state.FNS_file_path + "\n";
        description += "From frame " + QString::number(state.FNS_start_frame) + " to " + QString::number(state.FNS_stop_frame);
        lbl_fixed_suppression->setText(description);
    }
    else{
        lbl_fixed_suppression->setText("");
    }
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
    int source_state_idx = cmb_processing_states->currentIndex();
    FrameStacking(number_of_frames, source_state_idx);
}

void SirveApp::FrameStacking(int number_of_frames, int source_state_idx)
{
    // Pause the video if it's running
    playback_controller->StopTimer();

    video_display->container.processing_states.push_back(video_display->container.processing_states[source_state_idx]);
    int endi = video_display->container.processing_states.size()-1;
    int number_video_frames = static_cast<int>(video_display->container.processing_states[source_state_idx].details.frames_16bit.size());

    ImageProcessing *ImageProcessor = new ImageProcessing();
    OpenProgressArea("Frame stacking...",number_video_frames - 1);

    connect(ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, ImageProcessor, &ImageProcessing::CancelOperation);
    
    video_display->container.processing_states[endi].details.frames_16bit = ImageProcessor->FrameStacking(number_of_frames, video_display->container.processing_states[source_state_idx].details);

    if (video_display->container.processing_states[endi].details.frames_16bit.size()>0){
        video_display->container.processing_states[endi].method = ProcessingMethod::frame_stacking;
        video_display->container.processing_states[endi].frame_stack_num_frames = number_of_frames;
        video_display->container.processing_states[endi].source_state_ID = source_state_idx;

        // fetch max value
        uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
        for (const auto& row : video_display->container.processing_states[endi].details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }

        // create the new state
        video_display->container.processing_states[endi].details.max_value = maxVal;
        video_display->container.processing_states[endi].state_ID = video_display->container.processing_states.size() - 1;
        video_display->container.processing_states[source_state_idx].descendants.push_back(video_display->container.processing_states[endi].state_ID);
        video_display->container.processing_states[source_state_idx].descendants = GetUniqueIntegerVector(video_display->container.processing_states[source_state_idx].descendants);
        video_display->container.processing_states[endi].ancestors = video_display->container.processing_states[source_state_idx].ancestors;
        video_display->container.processing_states[endi].ancestors.push_back(source_state_idx);
        std::string result;
        for (auto num : video_display->container.processing_states[endi].ancestors) {
            result += std::to_string(num) + " -> ";
        }
        result += std::to_string(video_display->container.processing_states[endi].state_ID);
        QString state_steps = QString::fromStdString(result);
        video_display->container.processing_states[endi].state_steps = state_steps;
        video_display->container.processing_states[endi].process_steps.push_back(" [Frame Stacking] ");
        QString state_name = "State " + QString::number(endi) + ": " + video_display->container.processing_states[endi].get_friendly_description();
	    QString combobox_state_name = QString::number(endi) + ": " +video_display->container.processing_states[endi].get_combobox_description();
	    video_display->container.processing_states[endi].state_description = state_name;
        HandleNewProcessingState(state_name, combobox_state_name, endi);
        UpdateGlobalFrameVector();
    }
    CloseProgressArea();

    ImageProcessor->deleteLater();
}

void SirveApp::ExecuteAdaptiveNoiseSuppression()
{
    //-----------------------------------------------------------------------------------------------
    // get user selected frames for suppression

    int delta_frames = data_plots->index_sub_plot_xmax - data_plots->index_sub_plot_xmin;

    int relative_start_frame = txt_ANS_offset_frames->text().toInt();
    int number_of_frames = txt_ANS_number_frames->text().toInt();
    int source_state_idx = cmb_processing_states->currentIndex();
    bool continueTF = true;
    if (CheckCurrentStateisNoiseSuppressed(source_state_idx))
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Current state is already noise suppressed.", "Continue?");
        if (response != QMessageBox::Yes)
            {
                continueTF = false;
            }
    }
    if (continueTF)
    {

        ApplyAdaptiveNoiseSuppression(relative_start_frame, number_of_frames, source_state_idx);
    }
}

void SirveApp::ApplyAdaptiveNoiseSuppression(int relative_start_frame, int number_of_frames, int source_state_idx)
{
    //Pause the video if it's running
    playback_controller->StopTimer();

    video_display->container.processing_states.push_back(video_display->container.processing_states[source_state_idx]);
    int endi = video_display->container.processing_states.size()-1;
    int number_video_frames = static_cast<int>(video_display->container.processing_states[source_state_idx].details.frames_16bit.size());

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG availPhysMem = memInfo.ullAvailPhys;
    double available_memory_ratio = double(availPhysMem)/(double(number_video_frames)*16*SirveAppConstants::VideoDisplayWidth*SirveAppConstants::VideoDisplayHeight);

    ImageProcessing *ImageProcessor = new ImageProcessing();
    lbl_progress_status->setText(QString("Adaptive Noise Suppression..."));
    OpenProgressArea("Adaptive median background noise suppression...",number_video_frames - 1);

    connect(ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, ImageProcessor, &ImageProcessing::CancelOperation);

    if (available_memory_ratio >=1.5){
        video_display->container.processing_states[endi].details.frames_16bit = ImageProcessor->AdaptiveNoiseSuppressionMatrix(relative_start_frame, number_of_frames, video_display->container.processing_states[source_state_idx].details);
    }
    else{
        video_display->container.processing_states[endi].details.frames_16bit = ImageProcessor->AdaptiveNoiseSuppressionByFrame(relative_start_frame, number_of_frames, video_display->container.processing_states[source_state_idx].details);
    }

    if (video_display->container.processing_states[endi].details.frames_16bit.size()>0){
        QString description = "Filter starts at ";
        if (relative_start_frame > 0)
            description += "+";
        lbl_adaptive_noise_suppression_status->setWordWrap(true);
        description += QString::number(relative_start_frame) + " frames and averages " + QString::number(number_of_frames) + " frames";
        lbl_adaptive_noise_suppression_status->setText(description);

        // set new state
        video_display->container.processing_states[endi].method = ProcessingMethod::adaptive_noise_suppression;
        video_display->container.processing_states[endi].ANS_relative_start_frame = relative_start_frame;
        video_display->container.processing_states[endi].ANS_num_frames = number_of_frames;
        video_display->container.processing_states[endi].source_state_ID = source_state_idx;

        // fetch max value
        uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
        for (const auto& row : video_display->container.processing_states[endi].details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }

        video_display->container.processing_states[endi].details.max_value = maxVal;
        video_display->container.processing_states[endi].state_ID = video_display->container.processing_states.size() - 1;
        video_display->container.processing_states[source_state_idx].descendants.push_back(video_display->container.processing_states[endi].state_ID);
        video_display->container.processing_states[source_state_idx].descendants = GetUniqueIntegerVector(video_display->container.processing_states[source_state_idx].descendants);   
        video_display->container.processing_states[endi].ancestors = video_display->container.processing_states[source_state_idx].ancestors;
        video_display->container.processing_states[endi].ancestors.push_back(source_state_idx);

        // update gui status
        std::string result;
        for (auto num : video_display->container.processing_states[endi].ancestors) {
            result += std::to_string(num) + " -> ";
        }
        result += std::to_string(video_display->container.processing_states[endi].state_ID);

        QString state_steps = QString::fromStdString(result);
        video_display->container.processing_states[endi].state_steps = state_steps;
        video_display->container.processing_states[endi].process_steps.push_back(" [Adaptive Noise Suppression] ");
        QString state_name = "State " + QString::number(endi) + ": " + video_display->container.processing_states[endi].get_friendly_description();
	    QString combobox_state_name = QString::number(endi) + ": " +video_display->container.processing_states[endi].get_combobox_description();
	    video_display->container.processing_states[endi].state_description = state_name;
        HandleNewProcessingState(state_name, combobox_state_name, endi);
        UpdateGlobalFrameVector();
    }
    CloseProgressArea();

    ImageProcessor->deleteLater();
}

void SirveApp::ExecuteRPCPNoiseSuppression()
{
    int source_state_idx = cmb_processing_states->currentIndex();
    bool continueTF = true;
    if (CheckCurrentStateisNoiseSuppressed(source_state_idx))
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Current state is already noise suppressed.", "Continue?");
        if (response != QMessageBox::Yes)
            {
                continueTF = false;
            }
    }
    if (continueTF)
    {
        ApplyRPCPNoiseSuppression(source_state_idx);
    }
}

void SirveApp::ApplyRPCPNoiseSuppression(int source_state_idx)
{
    // Pause the video if it's running
    playback_controller->StopTimer();
    video_display->container.processing_states.push_back(video_display->container.processing_states[source_state_idx]);
    int endi = video_display->container.processing_states.size()-1;
    int number_video_frames = static_cast<int>(video_display->container.processing_states[source_state_idx].details.frames_16bit.size());

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG availPhysMem = memInfo.ullAvailPhys;
    double available_memory_ratio = double(availPhysMem)/(double(number_video_frames)*16*SirveAppConstants::VideoDisplayWidth*SirveAppConstants::VideoDisplayHeight);

    if(available_memory_ratio >=1.5){

        // set new state
        ImageProcessing ImageProcessor;
        OpenProgressArea("RPCP noise suppression...",number_video_frames - 1);

        connect(&ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
        connect(btn_cancel_operation, &QPushButton::clicked, &ImageProcessor, &ImageProcessing::CancelOperation);

        video_display->container.processing_states[endi].details.frames_16bit = ImageProcessor.RPCPNoiseSuppression(video_display->container.processing_states[source_state_idx].details);

        if (video_display->container.processing_states[endi].details.frames_16bit.size()>0){
            video_display->container.processing_states[endi].method = ProcessingMethod::RPCP_noise_suppression;
            video_display->container.processing_states[endi].source_state_ID = source_state_idx;
            uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
            for (const auto& row : video_display->container.processing_states[endi].details.frames_16bit) {
                maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
            }
            video_display->container.processing_states[endi].details.max_value = maxVal;
            video_display->container.processing_states[endi].state_ID =  video_display->container.processing_states.size() - 1;
            video_display->container.processing_states[endi].ancestors = video_display->container.processing_states[source_state_idx].ancestors;
            video_display->container.processing_states[endi].ancestors.push_back(source_state_idx);

            // update gui status
            std::string result;
            for (auto num : video_display->container.processing_states[endi].ancestors) {
                result += std::to_string(num) + " -> ";
            }
            result += std::to_string(video_display->container.processing_states[endi].state_ID);
            QString state_steps = QString::fromStdString(result);
            video_display->container.processing_states[endi].state_steps = state_steps;
            video_display->container.processing_states[endi].process_steps.push_back(" [RPCP Noise Suppression] ");
            QString state_name = "State " + QString::number(endi) + ": " + video_display->container.processing_states[endi].get_friendly_description();
            QString combobox_state_name = QString::number(endi) + ": " +video_display->container.processing_states[endi].get_combobox_description();
            video_display->container.processing_states[endi].state_description = state_name;
            HandleNewProcessingState(state_name, combobox_state_name, endi);
            UpdateGlobalFrameVector();
        }
    }
    else
    {
        QtHelpers::LaunchMessageBox(QString("Low memory"), "Insufficient memory for this operation. Please select fewer frames.");
    }
    CloseProgressArea();
}

void SirveApp::ExecuteAccumulatorNoiseSuppression()
{
    int source_state_idx = cmb_processing_states->currentIndex();
    bool continueTF = true;
    if (CheckCurrentStateisNoiseSuppressed(source_state_idx))
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Current state is already noise suppressed.", "Continue?");
        if (response != QMessageBox::Yes)
            {
                continueTF = false;
            }
    }
    if (continueTF)
    {
        double weight = txt_accumulator_weight->text().toDouble();
        if(weight<0 || weight >1){
            QtHelpers::LaunchMessageBox(QString("Invalid weight."), "Weight must be between 0 and 1.");
            return;
        }
        bool hide_shadow_choice = chk_hide_shadow->isChecked();
        int shadow_sigma_thresh = 6 - cmb_shadow_threshold->currentIndex();
        int offset = txt_accumulator_offset->text().toInt();
        ApplyAccumulatorNoiseSuppression(weight, offset, hide_shadow_choice, shadow_sigma_thresh, source_state_idx);
    }
}

void SirveApp::ApplyAccumulatorNoiseSuppression(double weight, int offset, bool hide_shadow_choice, int shadow_sigma_thresh, int source_state_idx)
{
    //Pause the video if it's running
    playback_controller->StopTimer();

    video_display->container.processing_states.push_back(video_display->container.processing_states[source_state_idx]);
    int endi = video_display->container.processing_states.size()-1;
    int number_video_frames = static_cast<int>(video_display->container.processing_states[source_state_idx].details.frames_16bit.size());

    ImageProcessing *ImageProcessor = new ImageProcessing();
    OpenProgressArea("Rolling mean noise suppression...",number_video_frames - 1);

    connect(ImageProcessor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, ImageProcessor, &ImageProcessing::CancelOperation);

    video_display->container.processing_states[endi].details.frames_16bit = ImageProcessor->AccumulatorNoiseSuppression(weight,offset,shadow_sigma_thresh,video_display->container.processing_states[source_state_idx].details,hide_shadow_choice);
    if(video_display->container.processing_states[endi].details.frames_16bit.size()>0){
        video_display->container.processing_states[endi].method = ProcessingMethod::accumulator_noise_suppression;
        video_display->container.processing_states[endi].source_state_ID = source_state_idx;
        video_display->container.processing_states[endi].weight = weight;
        video_display->container.processing_states[endi].offset = offset;
        video_display->container.processing_states[endi].hide_shadow = hide_shadow_choice;
        video_display->container.processing_states[endi].shadow_threshold = shadow_sigma_thresh;
        uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
        for (const auto& row : video_display->container.processing_states[endi].details.frames_16bit) {
            maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
        }
        video_display->container.processing_states[endi].details.max_value = maxVal;
        video_display->container.processing_states[endi].state_ID = video_display->container.processing_states.size() - 1;
        video_display->container.processing_states[endi].ancestors = video_display->container.processing_states[source_state_idx].ancestors;
        video_display->container.processing_states[endi].ancestors.push_back(source_state_idx);
        std::string result;
        for (auto num : video_display->container.processing_states[endi].ancestors) {
            result += std::to_string(num) + " -> ";
        }
        result += std::to_string(video_display->container.processing_states[endi].state_ID);
        QString state_steps = QString::fromStdString(result);
        video_display->container.processing_states[endi].state_steps = state_steps;
        video_display->container.processing_states[endi].process_steps.push_back(" [Rolling Mean Noise Suppression] ");
        QString state_name = "State " + QString::number(endi) + ": " + video_display->container.processing_states[endi].get_friendly_description();
        QString combobox_state_name = QString::number(endi) + ": " +video_display->container.processing_states[endi].get_combobox_description();
        video_display->container.processing_states[endi].state_description = state_name;
        HandleNewProcessingState(state_name, combobox_state_name, endi);
        UpdateGlobalFrameVector();

        ImageProcessor->deleteLater();
    }
  
    CloseProgressArea();
}


void SirveApp::ExecuteAutoTracking()
{
    playback_controller->StopTimer();
    processingState original = video_display->container.processing_states[video_display->container.current_idx];
    AutoTracking AutoTracker;

    int frame0 = txt_start_frame->text().toInt();
    
    uint start_frame = txt_auto_track_start_frame->text().toInt();
    uint stop_frame = txt_auto_track_stop_frame->text().toInt();

    int num_frames_to_track = stop_frame - start_frame + 1;
    OpenProgressArea("Generating track...",num_frames_to_track);

    connect(&AutoTracker, &AutoTracking::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, &AutoTracker, &AutoTracking::CancelOperation);

    if (start_frame < txt_start_frame->text().toInt() || stop_frame > txt_stop_frame->text().toInt() || stop_frame<start_frame){
        QtHelpers::LaunchMessageBox(QString("Invalid frame range."), "Min frame: " + txt_start_frame->text() + ". Max frame: " +txt_stop_frame->text() + ". Stop must be greater than start.");
        CloseProgressArea();
        return;
    }
    int start_frame_i = start_frame - frame0;
    int stop_frame_i = start_frame_i + num_frames_to_track - 1;
    bool ok;
    std::set<int> previous_manual_track_ids = track_info->get_manual_track_ids();
    int maxID = 0;
    if(previous_manual_track_ids.size()>0){
        maxID = *max_element(previous_manual_track_ids.begin(), previous_manual_track_ids.end());
    }
    u_int track_id = QInputDialog::getInt(this, tr("Select New Track Identifier"), tr("Track ID:"), maxID+1, 1, 1000000, 1, &ok);
    if (!ok || track_id < 0)
    {
        CloseProgressArea();
        return;
    }
    if (previous_manual_track_ids.find(track_id) != previous_manual_track_ids.end())
    {
        auto response = QtHelpers::LaunchYesNoMessageBox("Confirm Track Overwriting", "The track ID you have chosen already exists. You can edit this track without saving, but finalizing this track will overwrite it. Are you sure you want to proceed with editing the existing manual track?");
        if (response == QMessageBox::Yes)
        {
            std::vector<std::optional<TrackDetails>> existing_track_details = track_info->CopyManualTrack(track_id);
            PrepareForTrackCreation(track_id);
            video_display->EnterTrackCreationMode(existing_track_details);
        }
    }
    else
    {
        QString base_track_folder = config_values.workspace_folder;
        QDate today = QDate::currentDate();
        QTime currentTime = QTime::currentTime();;
        QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
        QString suggested_track_name = base_track_folder + "/auto_track_" + QString::number(track_id) + "_Frames_" + QString::number(start_frame) + "_" + QString::number(stop_frame) + "_" + formattedDate;
        QString new_track_file_name = QFileDialog::getSaveFileName(this, "Select a new file to save the track into", suggested_track_name, "CSV (*.csv)");
        if (new_track_file_name.isEmpty())
        {
            QtHelpers::LaunchMessageBox("Returning to Track Creation", "An invalid or empty file was chosen. To prevent data loss, edited tracks must be saved to disk to finish track creation. Returning to track editing mode.");
            CloseProgressArea();
            return;
        }
        string prefilter = "NONE";
        if (rad_autotrack_filter_gaussian->isChecked()){
            prefilter = "GAUSSIAN";
        }
        else if(rad_autotrack_filter_median->isChecked()){
            prefilter = "MEDIAN";
        }
        else if(rad_autotrack_filter_nlmeans->isChecked()){
            prefilter = "NLMEANS";
        }
        string trackFeature = "INTENSITY_WEIGHTED_CENTROID";
        if (rad_autotrack_feature_centroid->isChecked()){
            trackFeature = "CENTROID";
        }
        else if(rad_autotrack_feature_peak->isChecked()){
            trackFeature = "peak";
        }

        double clamp_low = txt_lift_sigma->text().toDouble();
        double clamp_high = txt_gain_sigma->text().toDouble();
        int threshold = 6 - cmb_autotrack_threshold->currentIndex();
        std::vector<std::optional<TrackDetails>>track_details = track_info->GetEmptyTrack();
        arma::u64_mat autotrack = AutoTracker.SingleTracker(track_id, clamp_low, clamp_high, threshold, prefilter, trackFeature, start_frame, start_frame_i, stop_frame_i, original.details, new_track_file_name);
        
        if (!autotrack.empty() && video_display->container.processing_states[video_display->container.current_idx].offsets.size()>0){
            arma::vec framei = arma::regspace(start_frame_i,start_frame_i + autotrack.n_rows - 1);
            arma::mat offset_matrix2(framei.n_elem,3,arma::fill::zeros);
            std::vector<std::vector<int>> offsets = video_display->container.processing_states[video_display->container.current_idx].offsets;
            arma::mat offset_matrix(offsets.size(),3,arma::fill::zeros);
            for (int rowi = 0; rowi < offsets.size(); rowi++){
                offset_matrix.row(rowi) = arma::conv_to<arma::rowvec>::from(offsets[rowi]);
            }
            for (int rowii = 0; rowii<framei.size(); rowii++){
                arma::uvec kk = arma::find(offset_matrix.col(0) == framei(rowii) + 1,0,"first");
                if (!kk.is_empty()){
                    offset_matrix2.row(rowii) = offset_matrix.row(kk(0));
                }
            }
            offset_matrix2.shed_col(0);
            arma::mat offset_matrix3 = offset_matrix2;
            offset_matrix2.insert_cols(0,2);
            offset_matrix2.insert_cols(offset_matrix2.n_cols,6);
            arma::mat offset_matrix4 = arma::join_rows(offset_matrix2,offset_matrix3);
            offset_matrix4.insert_cols(offset_matrix4.n_cols,2);
            arma::mat autotrack_d = arma::conv_to<arma::mat>::from(autotrack);
            autotrack_d += offset_matrix4;
            autotrack = arma::conv_to<arma::u64_mat>::from(autotrack_d);
        }

        if (!autotrack.empty()){

            // autotrack.save(new_track_file_name.toStdString(), arma::csv_ascii);

            // Create a CSV file
            std::ofstream outfile(new_track_file_name.toStdString());

            // Write headers (if needed)
            outfile << "TrackID,Frame,X,Y,PeakCounts,SumCounts,SumROICounts,NThresholdPixels,NROIPixels,Irradiance,ROI_x,ROI_y,ROI_Width,ROI_Height" << std::endl;

            autotrack.save(outfile, arma::csv_ascii);
            outfile.close();

            TrackDetails details;
            for (int rowii = 0; rowii<autotrack.n_rows; rowii++)
            {
                details.centroid_x = autotrack(rowii,2);
                details.centroid_y = autotrack(rowii,3);
                details.peak_counts = autotrack(rowii,4);
                details.sum_counts = autotrack(rowii,5);
                details.sum_ROI_counts = autotrack(rowii,6);
                details.N_threshold_pixels = autotrack(rowii,7);
                details.N_ROI_pixels = autotrack(rowii,8);
                details.irradiance =  autotrack(rowii,9);
                details.ROI_x = autotrack(rowii,10);
                details.ROI_y = autotrack(rowii,11);
                details.ROI_Width = autotrack(rowii,12);
                details.ROI_Height = autotrack(rowii,13);
                track_details[autotrack(rowii,1)] = details;
            }

            tm_widget->AddTrackControl(track_id);
            video_display->AddManualTrackIdToShowLater(track_id);
            track_info->AddCreatedManualTrack(track_id, track_details, new_track_file_name);

            int index0 = data_plots->index_sub_plot_xmin;
            int index1 = data_plots->index_sub_plot_xmax + 1;
            video_display->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
            data_plots->UpdateManualPlottingTrackFrames(track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());

            FramePlotSpace();

            cmb_manual_track_IDs->clear();
            cmb_manual_track_IDs->addItem("Primary");
            std::set<int> track_ids = track_info->get_manual_track_ids();
            for ( int track_id : track_ids )
            {
                cmb_manual_track_IDs->addItem(QString::number(track_id));
            }

            QStringList color_options = ColorScheme::get_track_colors();
            QWidget * existing_track_control = tm_widget->findChild<QWidget*>(QString("TrackControl_%1").arg(currently_editing_or_creating_track_id));
            if (existing_track_control != nullptr)
            {
                QLabel *lbl_track_description = existing_track_control->findChild<QLabel*>("track_description");
                const QFileInfo info(new_track_file_name);
                lbl_track_description->setText(info.fileName());
                int ind = existing_track_control->findChild<QComboBoxWithId*>()->currentIndex();
                HandleManualTrackRecoloring(track_id, color_options[ind]);
            }  
        }

    CloseProgressArea();
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
        lbl_fps->setText(" ");
    }
}

void SirveApp::EnableEngineeringPlotOptions()
{
    tab_plots->setCurrentIndex(1);

    rad_linear->setChecked(true);
    rad_linear->setChecked(true);

    cmb_plot_xaxis->clear();
    cmb_plot_xaxis->setFixedWidth(150);
    cmb_plot_xaxis->setEnabled(true);
    cmb_plot_xaxis->addItem(QString("Frames"));
    cmb_plot_xaxis->addItem(QString("Seconds from Midnight"));
    cmb_plot_xaxis->addItem(QString("Seconds from Epoch"));
    cmb_plot_xaxis->setCurrentIndex(0);

    cmb_plot_yaxis->clear();
    cmb_plot_yaxis->setEnabled(true);
    cmb_plot_yaxis->setFixedWidth(150);
    cmb_plot_yaxis->addItem(QString("ROI Counts"));
    cmb_plot_yaxis->addItem(QString("Azimuth"));
    cmb_plot_yaxis->addItem(QString("Elevation"));
    cmb_plot_yaxis->addItem(QString("IFOV - X"));
    cmb_plot_yaxis->addItem(QString("IFOV - Y"));
    cmb_plot_yaxis->addItem(QString("Boresight Azimuth"));
    cmb_plot_yaxis->addItem(QString("Boresight Elevation"));
    cmb_plot_yaxis->setCurrentIndex(2);


    // ------------------------------------------ Set Plot Options ------------------------------------------

    rad_decimal->setEnabled(true);
    rad_scientific->setEnabled(true);

    rad_log->setEnabled(true);
    rad_linear->setEnabled(true);

    btn_plot_menu->setEnabled(true);
    btn_save_plot->setEnabled(true);

    cmb_plot_yaxis->setEnabled(false);
    cmb_plot_xaxis->setEnabled(false);
}

void SirveApp::EnableYAxisOptions(bool enabled)
{
    this->rad_log->setEnabled(enabled);
    this->rad_linear->setEnabled(enabled);
    this->rad_decimal->setEnabled(enabled);
    this->rad_scientific->setEnabled(enabled);

    qDebug() << "enabled" << enabled;
    qDebug() << "------>>> enabled=" << enabled;

    if (!enabled)
    {
        qDebug() << "TOGGLING";

        data_plots->toggle_yaxis_log(false);
        data_plots->toggle_yaxis_scientific(false);
    }
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

std::vector<unsigned int> SirveApp::GetUniqueIntegerVector(std::vector<unsigned int> A)
{
    std::vector<unsigned int> uniqueVals;
    uniqueVals.insert( uniqueVals.end(),A.begin(), A.end() );
    std::sort(uniqueVals.begin(), uniqueVals.end()); 
    uniqueVals.erase(std::unique(uniqueVals.begin(), uniqueVals.end()), uniqueVals.end());
    return uniqueVals;
}

std::vector<unsigned int> SirveApp::GetUniqueUnionIntegerVector(std::vector<unsigned int> A, std::vector<unsigned int> B)
{
    std::vector<unsigned int> uniqueVals;
    uniqueVals.reserve(A.size() + B.size() ); 
    uniqueVals.insert( uniqueVals.end(), A.begin(), A.end() );
    uniqueVals.insert( uniqueVals.end(), B.begin(), B.end() );
    std::sort(uniqueVals.begin(), uniqueVals.end()); 
    uniqueVals.erase(std::unique(uniqueVals.begin(), uniqueVals.end()), uniqueVals.end());
    return uniqueVals;
}

void SirveApp::DeleteState()
{
    int current_state_idx0 = cmb_processing_states->currentIndex();
    processingState original = video_display->container.processing_states[current_state_idx0];
    std::vector<unsigned int> descendants = original.descendants;

    std::vector<u_int> delete_states_i;
    int current_state_id0 = original.state_ID;
    bool ischanged = false;

    if (descendants.size()>0){
        auto response = QtHelpers::LaunchYesNoMessageBox("Deletion Confirmation", "Deleting this state will delete all derived states. Are you sure you want to continue?");
        if (response == QMessageBox::Yes){
            delete_states_i.push_back(current_state_idx0);
            for (int i = 0; i < video_display->container.processing_states.size(); i++){
                std::vector<unsigned int> ancestors = video_display->container.processing_states[i].ancestors;
                auto it = std::find(ancestors.begin(), ancestors.end(), current_state_id0);
                if (it != ancestors.end()){
                    delete_states_i.push_back(i);
                }
            }
            std::sort(delete_states_i.begin(), delete_states_i.end(), std::greater<int>());
            for (auto i = 0; i <  delete_states_i.size() ;i++){
                cmb_processing_states->removeItem(delete_states_i[i]);
                video_display->container.processing_states.erase(video_display->container.processing_states.begin() + delete_states_i[i]);
            }
            ischanged = true;
        }
    }
    else {
        int delete_idx = cmb_processing_states->currentIndex();
        cmb_processing_states->removeItem(delete_idx);
        video_display->container.processing_states.erase(video_display->container.processing_states.begin() + delete_idx);
        ischanged = true;
    }

    if (ischanged){
        std::map<int,int> id_map;
        for (auto i = 0; i <  video_display->container.processing_states.size() ;i++){
            id_map[video_display->container.processing_states[i].state_ID] = i;
        }
        QList<QString> tmp_state_desc;
        for (auto i = 0; i < cmb_processing_states->count() ;i++){
            tmp_state_desc.append(cmb_processing_states->itemText(i));
        }
        QList<QString> new_labels;
        for (auto i = 0; i <  video_display->container.processing_states.size() ;i++){
            for (auto j = 0; j <  video_display->container.processing_states[i].ancestors.size() ;j++){
                video_display->container.processing_states[i].ancestors[j] = id_map[video_display->container.processing_states[i].ancestors[j]];
            }
            for (auto k = 0; k < video_display->container.processing_states[i].descendants.size(); k++){
                video_display->container.processing_states[i].descendants[k] = id_map[video_display->container.processing_states[i].descendants[k]];
            }
            QString desc = video_display->container.processing_states[i].state_description;
            desc.replace(QString::number(video_display->container.processing_states[i].state_ID) + ":",QString::number(id_map[video_display->container.processing_states[i].state_ID]) + ":");

            QString desc2 = desc;
            QString tmp0 = "<Previous State " + QString::number(video_display->container.processing_states[i].source_state_ID) + ">";
            desc2.replace(tmp0,"<Previous State " + QString::number(id_map[video_display->container.processing_states[i].source_state_ID]) + ">");

            std::string result;
            for (auto num : video_display->container.processing_states[i].ancestors) {
                result += std::to_string(num) + " -> ";
            }
            result += std::to_string(i);
            QString state_steps = QString::fromStdString(result);

            QString desc2b = desc2;
            int index = desc2b.lastIndexOf("State steps:");
            if (index !=-1){
                int nchars = desc2b.length() - (index+13);
                desc2b.replace(index + 13,nchars,state_steps);
            }
            QString desc3 = tmp_state_desc[id_map[video_display->container.processing_states[i].state_ID]];
            desc3.replace(QString::number(video_display->container.processing_states[i].state_ID) + ":",QString::number(id_map[video_display->container.processing_states[i].state_ID]) + ":");

            QString desc4 = desc3;
            desc4.replace(tmp0,"<Previous State " + QString::number(id_map[video_display->container.processing_states[i].source_state_ID]) + ">");
            new_labels.append(desc4);

            video_display->container.processing_states[i].state_ID = i;
            video_display->container.processing_states[i].source_state_ID = id_map[video_display->container.processing_states[i].source_state_ID];
            video_display->container.processing_states[i].state_description = desc2b;
            video_display->container.processing_states[i].state_steps = state_steps;
        }

        cmb_processing_states->clear();
        for (auto i = 0; i <  new_labels.size() ;i++){
            cmb_processing_states->addItem(new_labels[i]);
        }
    }
    cmb_processing_states->setCurrentIndex(cmb_processing_states->count()-1);

    btn_undo_step->setEnabled(cmb_processing_states->count() > 1);
    btn_delete_state->setEnabled(cmb_processing_states->count()> 1);
}

void SirveApp::HandleFrameChange()
{
    UpdateGlobalFrameVector();
}

void SirveApp::HandleAutoTrackStartChangeInput()
{
    unsigned int new_frame_number = txt_auto_track_start_frame->text().toUInt();
    if (new_frame_number >= txt_start_frame->text().toInt() && new_frame_number <= txt_stop_frame->text().toInt()){
        video_display->ViewFrame(new_frame_number-txt_start_frame->text().toInt());
        slider_video->setValue(new_frame_number-txt_start_frame->text().toInt());
        UpdateGlobalFrameVector();
    }
}

void SirveApp::HandleAutoTrackStopChangeInput()
{
    unsigned int new_frame_number = txt_auto_track_stop_frame->text().toUInt();
    if (new_frame_number >= txt_start_frame->text().toInt() && new_frame_number <= txt_stop_frame->text().toInt()){
        video_display->ViewFrame(new_frame_number-txt_start_frame->text().toInt());
        slider_video->setValue(new_frame_number-txt_start_frame->text().toInt());
        UpdateGlobalFrameVector();
    }
}

void SirveApp::HandleFrameNumberChangeInput()
{
    unsigned int new_frame_number = txt_goto_frame->text().toUInt();
    if (new_frame_number >= txt_start_frame->text().toInt() && new_frame_number <= txt_stop_frame->text().toInt()){
        video_display->ViewFrame(new_frame_number-txt_start_frame->text().toInt());
        slider_video->setValue(new_frame_number-txt_start_frame->text().toInt());
        UpdateGlobalFrameVector();
    }
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
    image_vector -= arma::mean(image_vector);
    image_vector /= (24*arma::stddev(image_vector));
    image_vector += (12*arma::stddev(image_vector));

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

    int max_val = std::round(image_max_value * gain);
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

void SirveApp::GetAboutTimeStamp()
{
    HMODULE hModule = GetModuleHandle(NULL);  // Get handle to current module (the .exe itself)
        if (hModule) {
            PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
            PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pDosHeader + pDosHeader->e_lfanew);
            DWORD timestamp = pNtHeaders->FileHeader.TimeDateStamp;
            std::cout << "Timestamp: " << timestamp << std::endl;
            std::cout << "Build date: " << ctime((time_t*)&timestamp);

        }

}
