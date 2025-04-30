#include "SirveApp.h"

#include "color_correction.h"
#include "data_export.h"
#include "osm_reader.h"
#include "wait_cursor.h"

SirveApp::SirveApp(QWidget *parent)
    : QMainWindow(parent)
{
    setObjectName("sirve_application");

    QScreen *screen = QApplication::primaryScreen();

    config_values = configReaderWriter::ExtractWorkspaceConfigValues();

    workspace = new Workspace(config_values.workspace_folder);

    file_processor = new ProcessFile();
    state_manager_ = new StateManager(this);

    color_map_display = new ColorMapDisplay(video_colors.maps[0].colors, 0, 1);
    video_player_ = new VideoPlayer(this, state_manager_, video_colors.maps[0].colors);

    histogram_plot = new HistogramLinePlot();

    SetupUi();
    popout_histogram = new PopoutDialog();
    popout_engineering = new PopoutDialog();

    //---------------------------------------------------------------------------
    // setup container to store all videos
    eng_data = nullptr;

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
    SetupConnections();

    HandleRelativeHistogramToggle(false);
    video_player_->SetPlaybackEnabled(false);

    CreateMenuActions();

    this->resize(0, 0);
    screenResolution = screen->size();
    osmDataLoaded = false;
    UpdateGuiPostDataLoad(osmDataLoaded);

    connect(this->file_processor, &ProcessFile::forwardProgress, this->progress_bar_main, &QProgressBar::setValue);
}


QPoint SirveApp::GetWindowPosition() const {
    return this->pos();  // Get the current position of the window
}

QSize SirveApp::GetWindowSize() const {
    return this->size();  // Get the current size of the window
}

void SirveApp::SetupUi() {

    QHBoxLayout* main_layout = new QHBoxLayout();
    QVBoxLayout* main_layout_col1 = new QVBoxLayout();
    QVBoxLayout* main_layout_col2 = new QVBoxLayout();
    QVBoxLayout* main_layout_col3 = new QVBoxLayout();

    main_layout_col1->setSpacing(0);
    main_layout_col2->setSpacing(0);
    main_layout_col3->setSpacing(0);

    main_layout_col1->setContentsMargins(10,10,5, 10);
    main_layout_col2->setContentsMargins(5,10,5, 10);
    main_layout_col3->setContentsMargins(5,10,10, 10);

    QWidget* leftWidget = new QWidget();
    QWidget* centralWidget = new QWidget();
    QWidget* rightWidget = new QWidget();


    leftWidget->setContentsMargins(0,0,0, 0);
    centralWidget->setContentsMargins(0,0,0, 0);
    rightWidget->setContentsMargins(0,0,0, 0);

    // Create a splitter
    splitter = new QSplitter(Qt::Horizontal, this);


    // Set minimum sizes
    leftWidget->setMinimumWidth(leftWidgetStartingSize);
    centralWidget->setMinimumWidth(centralWidgetStartingSize);
    rightWidget->setMinimumWidth(rightWidgetStartingSize);

    leftWidget->setLayout(main_layout_col1);
    centralWidget->setLayout(main_layout_col2);
    rightWidget->setLayout(main_layout_col3);

    // Add widgets to splitter
    splitter->addWidget(leftWidget);
    splitter->addWidget(centralWidget);
    splitter->addWidget(rightWidget);

    // Set initial size ratios
    splitter->setStretchFactor(0, 1);  // Left widget small
    splitter->setStretchFactor(1, 2);  // Central widget larger
    splitter->setStretchFactor(2, 1);  // Right widget small

    // removes the added extra padding around widgets
    splitter->setHandleWidth(0);


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
    auto *hlayout_load_frames_area = new QHBoxLayout(grpbox_load_frames_area);
    hlayout_load_frames_area->addWidget(lbl_max_frames);
    hlayout_load_frames_area->addLayout(form_start_frame);
    hlayout_load_frames_area->addLayout(form_stop_frame);
    hlayout_load_frames_area->addWidget(btn_get_frames);
    hlayout_load_frames_area->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout
    grpbox_load_frames_area->setFixedHeight(50);

    grpbox_progressbar_area = new QGroupBox();
    grpbox_progressbar_area->setObjectName("grpbox_progressbar_area");
    auto *hlayout_progressbar_area = new QHBoxLayout(grpbox_progressbar_area);
    grpbox_progressbar_area->setLayout(hlayout_progressbar_area);
    grpbox_progressbar_area->setEnabled(false);
    progress_bar_main = new QProgressBar();
    progress_bar_main->setMinimumWidth(300); // 300? Why not!?
    progress_bar_main->setAlignment(Qt::AlignLeft);
    progress_bar_main->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    btn_cancel_operation = new QPushButton("Cancel");
    btn_cancel_operation->setFixedWidth(60);
    hlayout_progressbar_area->addWidget(progress_bar_main);
    hlayout_progressbar_area->addStretch(2);
    hlayout_progressbar_area->addWidget(btn_cancel_operation);

    grpbox_status_area = new QGroupBox("State Control");
    grpbox_status_area->setObjectName("grpbox_status_area");
    grpbox_status_area->setFixedHeight(200);
    auto *vlayout_status_area = new QVBoxLayout();
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

    auto *vlayout_processing_description = new QVBoxLayout(scrollarea_processing_description);
    vlayout_processing_description->addWidget(lbl_processing_description);
    auto * d_bottom_vertical_spacer = new QSpacerItem(10, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vlayout_processing_description->addItem(d_bottom_vertical_spacer);
    vlayout_status_area->addWidget(scrollarea_processing_description);
    auto *form_processing_state = new QFormLayout;
    form_processing_state->addRow(tr("&Processing State:"),cmb_processing_states);
    form_processing_state->setFormAlignment(Qt::AlignHCenter | Qt::AlignCenter);
    vlayout_status_area->addLayout(form_processing_state);
    auto *hlayout_processing_state_buttons = new QHBoxLayout();
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

    // removes extra padding from layout
    main_layout->setContentsMargins(0,0,0,0);
    main_layout->setSpacing(0);

    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // removes extra padding from layout
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(splitter);
    mainWidget->setLayout(mainLayout);

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

    grpbox_auto_lift_gain->setEnabled(false);

    cmb_processing_states->setEnabled(false);

    btn_import_tracks->setEnabled(false);
    btn_create_track->setEnabled(false);
    chk_auto_lift_gain->setChecked(true);
    btn_reset_color_correction->setEnabled(false);
    grpbox_auto_lift_gain->setEnabled(true);
    // ------------------------------------------------------------------------

    this->setCentralWidget(mainWidget);

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
    lbl_progress_status->setFixedWidth(200); // adjust this number to give progress bar room to expand (or not)
    lbl_progress_status->setWordWrap(true);
    auto *grpbox_status_bar = new QGroupBox();
    grpbox_status_bar->setMinimumWidth(1050);
    auto *hlayout_status_bar1 = new QHBoxLayout();
    auto *hlayout_status_bar2 = new QHBoxLayout();
    auto *grpbox_status_permanent = new QGroupBox();
    grpbox_status_permanent->setMinimumWidth(600);
    auto *hlayout_status_permanent = new QHBoxLayout();
    grpbox_status_permanent->setLayout(hlayout_status_permanent);

    auto *hspacer_item10 = new QSpacerItem(10,1);
    auto *vlayout_status_lbl = new QVBoxLayout();


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
    //hlayout_status_permanent->addItem(hspacer_item1);
    hlayout_status_permanent->addStretch(1);
    hlayout_status_permanent->addWidget(grpbox_progressbar_area);
    //hlayout_status_permanent->addItem(hspacer_item1);
    //hlayout_status_permanent->insertStretch(-1,0);

    status_bar->addPermanentWidget(grpbox_status_permanent,0);
    status_bar->setContentsMargins(10,0,10, 10);

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

    video_player_->ToggleOsmTracks(true);

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
    connect(cmb_outlier_processing_type, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &SirveApp::HandleOutlierProcessingChange);

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
    connect(cmb_bad_pixel_color, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &SirveApp::EditBadPixelColor);

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

    toolbox_image_processing = new QToolBox();
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
    QLabel *lbl_track = new QLabel("User Defined Tracks");
    lbl_create_track_message = new QLabel("");
    btn_create_track = new QPushButton("Create Manual Track");
    btn_create_track->setFixedWidth(160);
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
    hlayout_workspace->addWidget(btn_create_track,Qt::AlignLeft);
    hlayout_workspace->addWidget(btn_finish_create_track,Qt::AlignLeft);
    btn_auto_track_target = new QPushButton("Auto Tracker");
    connect(btn_auto_track_target, &QPushButton::clicked, this, &SirveApp::ExecuteAutoTracking);
    hlayout_workspace->addWidget(btn_auto_track_target);
    hlayout_workspace->addWidget(btn_import_tracks,Qt::AlignLeft);
    //hlayout_workspace->addStretch();
    vlayout_workspace->addLayout(hlayout_workspace);

    QGroupBox * grpbox_autotrack = new QGroupBox("Tracking Parameters");
    QVBoxLayout *vlayout_auto_track_control = new QVBoxLayout(grpbox_autotrack);
    QHBoxLayout *hlayout_auto_track_control = new QHBoxLayout;
    txt_auto_track_start_frame = new QLineEdit("1");
    txt_auto_track_start_frame->setFixedWidth(60);
    txt_auto_track_stop_frame = new QLineEdit("");
    txt_auto_track_stop_frame->setFixedWidth(60);
    txt_pixel_buffer = new QLineEdit("0");
    QIntValidator *validator = new QIntValidator(0, 10, this);
    txt_pixel_buffer->setValidator(validator);
    txt_auto_track_stop_frame->setFixedWidth(60);
    QFormLayout *form_auto_track_frame_limits = new QFormLayout;
    form_auto_track_frame_limits->addRow(tr("&Frame Start:"), txt_auto_track_start_frame);
    form_auto_track_frame_limits->addRow(tr("&Frame Stop:"), txt_auto_track_stop_frame);
    cmb_autotrack_threshold = new QComboBox;
    cmb_autotrack_threshold->addItem("10 dB",10);
    cmb_autotrack_threshold->addItem("9 dB",9);
    cmb_autotrack_threshold->addItem("8 dB",8);
    cmb_autotrack_threshold->addItem("7 dB",7);
    cmb_autotrack_threshold->addItem("6 dB",6);
    cmb_autotrack_threshold->addItem("5 dB",5);
    cmb_autotrack_threshold->addItem("4 dB",4);
    cmb_autotrack_threshold->addItem("3 dB",3);
    cmb_autotrack_threshold->addItem("2 dB",2);
    cmb_autotrack_threshold->addItem("1 dB",1);
    cmb_autotrack_threshold->setCurrentIndex(4);
    // connect(cmb_autotrack_threshold, qOverload<int>(&QComboBox::currentIndexChanged), video_display, &VideoDisplay::GetThreshold);

    connect(cmb_autotrack_threshold, qOverload<int>(&QComboBox::currentIndexChanged), this, &SirveApp::onThresholdComboBoxIndexChanged);
    connect(this, &SirveApp::itemDataSelected, video_player_, &VideoPlayer::SetThreshold);

    form_auto_track_frame_limits->addRow(tr("&Threshold:"), cmb_autotrack_threshold);
    form_auto_track_frame_limits->addRow(tr("&Pixel Buffer:"), txt_pixel_buffer);
    QVBoxLayout *vlayout_auto_track = new QVBoxLayout;
    vlayout_auto_track->addLayout(form_auto_track_frame_limits);

    tm_widget = new TrackManagementWidget(widget_tab_tracks);
    QScrollArea *track_management_scroll_area = new QScrollArea();
    track_management_scroll_area->setMinimumHeight(275);
    track_management_scroll_area->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    track_management_scroll_area->setWidgetResizable(true);
    track_management_scroll_area->setWidget(tm_widget);
    track_management_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vlayout_workspace->addWidget(track_management_scroll_area);
    vlayout_workspace->insertStretch(-1,0);
    vlayout_workspace->insertStretch(0,0);
    QStringList colors = ColorScheme::get_track_colors();
    chk_show_OSM_tracks = new QCheckBox("OSM Tracks");
    chk_show_OSM_tracks->setChecked(true);
    cmb_OSM_track_color = new QComboBox();
    cmb_OSM_track_color->addItems(colors);
    cmb_OSM_track_color->setEnabled(true);
    cmb_OSM_track_color->setCurrentIndex(2);

    QGroupBox * grpbox_OSM_track_display = new QGroupBox;
    QHBoxLayout *hlayout_OSM_track_display = new QHBoxLayout(grpbox_OSM_track_display);
    hlayout_OSM_track_display->addWidget(chk_show_OSM_tracks);
    hlayout_OSM_track_display->addWidget(cmb_OSM_track_color);
    hlayout_OSM_track_display->insertStretch(-1,0);
    hlayout_OSM_track_display->insertStretch(0,0);

    QGroupBox* grpbox_autotrack_filters = new QGroupBox("Pre Filter Options");
    QGridLayout *grid_autotrack_filters = new QGridLayout(grpbox_autotrack_filters);
    rad_autotrack_filter_none = new QRadioButton("None");
    rad_autotrack_filter_none->setChecked(true);
    rad_autotrack_filter_gaussian = new QRadioButton("Gaussian");
    rad_autotrack_filter_median = new QRadioButton("Median");
    rad_autotrack_filter_nlmeans = new QRadioButton("Non Local Means");
    QSpacerItem *vspacer_item20 = new QSpacerItem(10,10,QSizePolicy::Expanding,QSizePolicy::Minimum);

    QButtonGroup * buttongrp_autotrack_filters = new QButtonGroup();

    buttongrp_autotrack_filters->addButton(rad_autotrack_filter_none,1);
    buttongrp_autotrack_filters->addButton(rad_autotrack_filter_gaussian,2);
    buttongrp_autotrack_filters->addButton(rad_autotrack_filter_median,3);
    buttongrp_autotrack_filters->addButton(rad_autotrack_filter_nlmeans,4);
    connect(buttongrp_autotrack_filters, &QButtonGroup::idClicked, video_player_, &VideoPlayer::OnFilterRadioButtonClicked);

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
    buttongrp_autotrack_feature->addButton(rad_autotrack_feature_weighted_centroid,1);
    buttongrp_autotrack_feature->addButton(rad_autotrack_feature_centroid,2);
    buttongrp_autotrack_feature->addButton(rad_autotrack_feature_peak,3);
    connect(buttongrp_autotrack_feature, &QButtonGroup::idClicked, video_player_, &VideoPlayer::OnTrackFeatureRadioButtonClicked);

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

    vlayout_tab_workspace->addWidget(grpbox_autotrack);
    vlayout_tab_workspace->insertStretch(0,0);
    vlayout_tab_workspace->addLayout(vlayout_workspace);
    vlayout_tab_workspace->addWidget(grpbox_OSM_track_display);
    vlayout_tab_workspace->insertStretch(-1, 0);

    return widget_tab_tracks;
}
void SirveApp::ResetEngineeringDataAndSliderGUIs()
{
    video_player_->ResetSlider();
    if (eng_data != nullptr){
        std::set<int> previous_manual_track_ids = track_info->get_manual_track_ids();
        for ( int track_id : previous_manual_track_ids )
        {
            HandleTrackRemoval(track_id);
            video_player_->DeleteManualTrack(track_id);
        }
    }
}

void SirveApp::SetupVideoFrame()
{

    frame_video_player->setObjectName("frame_video_player");
    // ------------------------------------------------------------------------
    // Adds all elements to main UI

    frame_video_player->setFrameShape(QFrame::Box);
    frame_video_player->setLayout(new QGridLayout(frame_video_player));
    video_player_->setParent(frame_video_player);
    frame_video_player->layout()->addWidget(video_player_);
}

void SirveApp::SetupPlotFrame() {

    tab_plots->setTabPosition(QTabWidget::South);

    auto* widget_tab_histogram = new QWidget();

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

    // set layout for engineering plots tab
    auto* widget_plots_tab_color = new QWidget();
    auto* vlayout_widget_plots_tab_color = new QVBoxLayout(widget_plots_tab_color);

    vlayout_widget_plots_tab_color->addWidget(frame_plots);

    // ------------------------------------------------------------------------
    // Add all to tab widget

    // set ratio of first to second elements on engineering plot tab
    vlayout_widget_plots_tab_color->setStretch(0, 7);
    vlayout_widget_plots_tab_color->setStretch(1, 1);

    tab_plots->addTab(widget_tab_histogram, "Histogram");
    tab_plots->addTab(widget_plots_tab_color, "Plots");

    tab_plots->tabBar()->hide();
}

void SirveApp::SetupConnections() {

    connect(this, &SirveApp::updateVideoDisplayPinpointControls, this->video_player_, &VideoPlayer::HandlePinpointControlActivation);

    //---------------------------------------------------------------------------

    connect(state_manager_, &StateManager::updateDisplayVideo, this, &SirveApp::HandleFrameChange);
    connect(btn_undo_step, &QPushButton::clicked, state_manager_, &StateManager::PopProcessingState);

    connect(state_manager_, &StateManager::stateAdded, this, &SirveApp::HandleNewProcessingState);
    connect(state_manager_, &StateManager::stateRemoved, this, &SirveApp::HandleProcessingStateRemoval);
    connect(state_manager_, &StateManager::statesCleared, this, &SirveApp::HandleProcessingStatesCleared);

    connect(cmb_processing_states, qOverload<int>(&QComboBox::currentIndexChanged), video_player_, &VideoPlayer::SetCurrentIdx);
    connect(cmb_processing_states, qOverload<int>(&QComboBox::currentIndexChanged), state_manager_, &StateManager::SelectState);
    connect(cmb_processing_states, qOverload<int>(&QComboBox::currentIndexChanged), this, &SirveApp::HandleProcessingNewStateSelected);

    connect(histogram_plot, &HistogramLinePlot::clickDragHistogram, this, &SirveApp::HandleHistogramClick);
    connect(video_player_, &VideoPlayer::addNewBadPixels, this, &SirveApp::ReceiveNewBadPixels);
    connect(video_player_, &VideoPlayer::removeBadPixels, this, &SirveApp::ReceiveNewGoodPixels);

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
    connect(chk_sensor_track_data, &QCheckBox::stateChanged, video_player_, &VideoPlayer::SetDisplayBoresight);
    connect(chk_show_time, &QCheckBox::stateChanged, video_player_, &VideoPlayer::SetFrameTimeToggle);
    connect(cmb_color_maps, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::EditColorMap);
    connect(cmb_text_color, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::EditBannerColor);

    connect(btn_add_annotations, &QPushButton::clicked, this, &SirveApp::AnnotateVideo);
    connect(btn_change_banner_text, &QPushButton::clicked, this, &SirveApp::EditBannerText);

    //---------------------------------------------------------------------------

    //Link buttons to functions
    connect(btn_get_frames, &QPushButton::clicked, this, &SirveApp::UiLoadAbirData);
    connect(txt_stop_frame, &QLineEdit::returnPressed, this, &SirveApp::UiLoadAbirData);
    connect(chk_highlight_bad_pixels, &QPushButton::clicked, video_player_, &VideoPlayer::HighlightBadPixels);

    connect(btn_FNS, &QPushButton::clicked, this, &SirveApp::ExecuteFixedNoiseSuppression);

    connect(btn_ANS, &QPushButton::clicked, this, &SirveApp::ExecuteAdaptiveNoiseSuppression);

    //---------------------------------------------------------------------------
    connect(btn_import_tracks, &QPushButton::clicked, this, &SirveApp::ImportTracks);
    connect(btn_create_track, &QPushButton::clicked, this, &SirveApp::HandleCreateTrackClick);
    connect(btn_finish_create_track, &QPushButton::clicked, this, &SirveApp::HandleFinishCreateTrackClick);
    connect(video_player_, &VideoPlayer::finishTrackCreation, this, &SirveApp::HandleFinishCreateTrackClick);

    connect(tm_widget, &TrackManagementWidget::displayTrack, this, &SirveApp::HandleShowManualTrackId);
    connect(tm_widget, &TrackManagementWidget::hideTrack, this, &SirveApp::HandleHideManualTrackId);
    connect(tm_widget, &TrackManagementWidget::deleteTrack, this, &SirveApp::HandleTrackRemoval);
    connect(tm_widget, &TrackManagementWidget::recolorTrack, this, &SirveApp::HandleManualTrackRecoloring);

    // Connect epoch button click to function
    connect(btn_apply_epoch, &QPushButton::clicked, this, &SirveApp::ApplyEpochTime);

    //Enable saving frame TODO: Was this a requirement?
    //connect(btn_frame_save, &QPushButton::clicked, this, &SirveApp::SaveFrame);

    connect(btn_popout_histogram, &QPushButton::clicked, this, &SirveApp::HandlePopoutHistogramClick);

}

// ReSharper disable once CppMemberFunctionMayBeConst
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
        video_player_->AddManualTrackIdToShowLater(track_id);
        tm_widget->AddTrackControl(track_id);
        if (previous_manual_track_ids.find(track_id) != previous_manual_track_ids.end())
        {
            auto response = QtHelpers::LaunchYesNoMessageBox("Confirm Track Overwriting", "Warning: Overwriting track ID: " + QString::number(track_id));
            if (response == QMessageBox::Yes)
            {
                video_player_->AddManualTrackIdToShowLater(track_id);
                tm_widget->AddTrackControl(track_id);

                QWidget * existing_track_control = tm_widget->findChild<QWidget*>(QString("TrackControl_%1").arg(track_id));
                if (existing_track_control != nullptr)
                {
                    QLabel *lbl_track_description = existing_track_control->findChild<QLabel*>("track_description");
                    const QFileInfo info(file_selection);
                    lbl_track_description->setText(info.fileName());
                }
                track_info->AddManualTracks(result.frames);

                int index0 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin();
                int index1 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmax() + 1;
                video_player_->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));

                for (int i = 0; i < plot_palette->tabBar()->count(); i++)
                {
                    plot_palette->UpdateManualPlottingTrackFrames(i, track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
                }

                FramePlotSpace();
            }
        }
        else
        {

            video_player_->AddManualTrackIdToShowLater(track_id);
            tm_widget->AddTrackControl(track_id);

            track_info->AddManualTracks(result.frames);
            cmb_manual_track_IDs->clear();
            cmb_manual_track_IDs->addItem("Primary");
            std::set<int> track_ids = track_info->get_manual_track_ids();

            for ( int tid : track_ids ){
                cmb_manual_track_IDs->addItem(QString::number(tid));

            }

            QWidget * existing_track_control = tm_widget->findChild<QWidget*>(QString("TrackControl_%1").arg(track_id));
            if (existing_track_control != nullptr)
            {
                QLabel *lbl_track_description = existing_track_control->findChild<QLabel*>("track_description");
                const QFileInfo info(file_selection);
                lbl_track_description->setText(info.fileName());
            }

            int index0 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin();
            int index1 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmax() + 1;
            video_player_->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));;
            for (int i = 0; i < plot_palette->tabBar()->count(); i++)
            {
                plot_palette->UpdateManualPlottingTrackFrames(i, track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
            }

            FramePlotSpace();
        }

    }
}

void SirveApp::HandleCreateTrackClick()
{
    bool ok;
    QPoint appPos = this->GetWindowPosition();
    int bbox_buffer_pixels = txt_pixel_buffer->text().toInt();
    std::set<int> previous_manual_track_ids = track_info->get_manual_track_ids();
    int maxID = 0;
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

    double clamp_low_coeff = txt_lift_sigma->text().toDouble();
    double clamp_high_coeff = txt_gain_sigma->text().toDouble();
    int threshold = cmb_autotrack_threshold->itemData(cmb_autotrack_threshold->currentIndex(),Qt::UserRole).toInt();
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
            video_player_->EnterTrackCreationMode(appPos,existing_track_details, threshold, bbox_buffer_pixels, clamp_low_coeff, clamp_high_coeff, trackFeature, prefilter);
        }
    }
    else
    {
        std::vector<std::optional<TrackDetails>> empty_track_details = track_info->GetEmptyTrack();
        PrepareForTrackCreation(track_id);
        video_player_->EnterTrackCreationMode(appPos,empty_track_details, threshold, bbox_buffer_pixels, clamp_low_coeff, clamp_high_coeff, trackFeature, prefilter);
    }
}

void SirveApp::PrepareForTrackCreation(int track_id)
{
    currently_editing_or_creating_track_id = track_id;
    btn_create_track->setHidden(true);
    btn_finish_create_track->setHidden(false);
    lbl_create_track_message->setText("Editing Track: " + QString::number(currently_editing_or_creating_track_id));
    tab_menu->setTabEnabled(0, false);
}

void SirveApp::HandleFinishCreateTrackClick()
{
    const auto& created_track_details = video_player_->GetCreatedTrackDetails();
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
        video_player_->AddManualTrackIdToShowLater(currently_editing_or_creating_track_id);
        track_info->AddCreatedManualTrack(eng_data->get_plotting_frame_data(),currently_editing_or_creating_track_id, created_track_details, new_track_file_name);

        int index0 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin();
        int index1 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmax() + 1;
        video_player_->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
        plot_palette->UpdateManualPlottingTrackFrames(0, track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());

        for (int i = 0; i < plot_palette->tabBar()->count(); i++)
        {
            plot_palette->UpdateManualPlottingTrackFrames(i, track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
        }


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
    video_player_->ExitTrackCreationMode();
}

void SirveApp::HandleHideManualTrackId(int track_id)
{
    QColor new_color(0,0,0,0);

    // TODO: Check this!
    video_player_->HideManualTrackId(track_id);
    plot_palette->RecolorManualTrack(0, track_id, new_color); // Why painting black here?
    FramePlotSpace();
}

void SirveApp::HandleShowManualTrackId(int track_id, const QColor& new_color)
{
    video_player_->ShowManualTrackId(track_id);
    plot_palette->RecolorManualTrack(0, track_id, new_color);
    FramePlotSpace();
}

void SirveApp::HandleTrackRemoval(int track_id)
{
    tm_widget->RemoveTrackControl(track_id);
    track_info->RemoveManualTrack(track_id);
    cmb_manual_track_IDs->clear();
    cmb_manual_track_IDs->addItem("Primary");
    std::set<int> track_ids = track_info->get_manual_track_ids();

    for ( int tid : track_ids ){
        cmb_manual_track_IDs->addItem(QString::number(tid));
    }
    track_info->RemoveManualTrackPlotting(track_id);
    track_info->RemoveManualTrackImage(track_id);
    int index0 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin();
    int index1 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmax() + 1;
    video_player_->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
    video_player_->DeleteManualTrack(track_id);

    // WARNING: This should be amended later to adjust for tabs that have been hidden by the user:
    for (int i = 0; i < plot_palette->tabBar()->count(); i++)
    {
        plot_palette->DeleteGraphIfExists(i, track_id);
        plot_palette->RedrawPlot(i);
        plot_palette->UpdateManualPlottingTrackFrames(i, track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
    }

    FramePlotSpace();
}

void SirveApp::HandleManualTrackRecoloring(int track_id, const QColor& new_color)
{
    video_player_->RecolorManualTrack(track_id, new_color);
    for (int index = 0; index < plot_palette->tabBar()->count(); index++)
    {
        plot_palette->RecolorManualTrack(index, track_id, new_color);
    }

    FramePlotSpace();
}

void SirveApp::FramePlotSpace()
{
    plot_palette->PlotAllSirveTracks();
}

std::once_flag once_flag_main_window;

SirveApp* SirveApp::GetMainWindow()
{
    static QPointer<SirveApp> main_window;
    std::call_once(once_flag_main_window, []() {
        QWidgetList widgets = QApplication::topLevelWidgets();
        for (auto widget : widgets) {
            if (widget->objectName() == "sirve_application") {
                main_window = qobject_cast<SirveApp*>(widget);
                break;
            }
        }
    });

    return main_window;
}

const QVector<QRgb>& SirveApp::GetStartingColorTable() const
{
    static QVector<QRgb> empty;
    return video_colors.maps.isEmpty() ? empty : video_colors.maps[0].colors;
}

void SirveApp::SaveWorkspace()
{
    if (abp_file_metadata.image_path == "" || state_manager_->empty()) {
        QtHelpers::LaunchMessageBox(QString("Issue Saving Workspace"), "No frames are loaded, unable to save workspace.");
    }
    else {
        QString current_workspace_name = lbl_workspace_name_field->text();
        QDate today = QDate::currentDate();
        QTime currentTime = QTime::currentTime();
        QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
        QString start_frame = QString::number(plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin() + 1);
        QString stop_frame = QString::number(plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmax() + 1);
        QString initial_name = abpimage_file_base_name + "_" + start_frame + "-"+ stop_frame + "_" + formattedDate;

        QString selectedUserFilePath = QFileDialog::getSaveFileName(this, tr("Workspace File"), config_values.workspace_folder + "/" + initial_name, tr("Workspace Files *.json"));

        if (selectedUserFilePath.length() > 0) {
            QFileInfo fileInfo(selectedUserFilePath);

            workspace->SaveState(selectedUserFilePath,
                                 abp_file_metadata.image_path,
                                 plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin() + 1,
                                 plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmax() + 1,
                                 eng_data->get_offset_time(),
                                 *state_manager_,
                                 video_player_->GetAnnotations(),
                                 classification_list);
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

        compare = QString::compare(workspace_vals.image_path, "", Qt::CaseInsensitive);
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

        LoadAbirData(workspace_vals.start_frame, workspace_vals.end_frame);

        ProcessingState original = workspace_vals.all_states[0];

        for (auto i = 1; i < workspace_vals.all_states.size(); i++)
        {
            ProcessingState current_state = workspace_vals.all_states[i];

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

        video_player_->ClearAnnotations();

        for (auto i = 0; i < workspace_vals.annotations.size(); i++)
        {
            AnnotationInfo anno = workspace_vals.annotations[i];
            video_player_->AddAnnotation(std::move(anno));
        }

        for (auto i = 0; i < workspace_vals.classifications.size(); i++)
        {
            Classification classification = workspace_vals.classifications[i];

            // Set classification to the appropriate widget, depending on its type.
            if (classification.type == "Plot")
            {
                // TODO: Fix this to use new PlotSpace API
                // data_plots->SetPlotTitle(classification.text);
            } else
            {
                video_player_->UpdateBannerText(classification.text);
            }

            classification_list.push_back(classification);
        }

        eng_data->set_offset_time(workspace_vals.timing_offset);
    }
}

void SirveApp::HandleAbpBFileSelected()
{
    abp_file_type = ABPFileType::ABP_B;
    HandleAbpFileSelected();
    for (int i = 0; i < toolbox_image_processing->count(); ++i) {
        if (QString::compare(toolbox_image_processing->itemText(i), "Deinterlacing", Qt::CaseInsensitive) == 0)
        {
            toolbox_image_processing->setItemEnabled(i, true);
            toolbox_image_processing->setItemIcon(i, QIcon());
        }
    }
}

void SirveApp::HandleAbpDFileSelected()
{
    abp_file_type = ABPFileType::ABP_D;
    HandleAbpFileSelected();
    for (int i = 0; i < toolbox_image_processing->count(); ++i) {
        if (QString::compare(toolbox_image_processing->itemText(i), "Deinterlacing", Qt::CaseInsensitive) == 0)
        {
            QIcon icon(":/icons/no-ghosts.png");

            // Set same pixmap for Disabled mode
            icon.addPixmap(QPixmap(":/icons/no-ghosts.png"), QIcon::Disabled);

            toolbox_image_processing->setItemIcon(i, icon);
            toolbox_image_processing->setItemEnabled(i, false);
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

    WaitCursor cursor;

    lbl_processing_description->setText("");

    bool validated = ValidateAbpFiles(file_selection);
    if (validated)
    {
        LoadOsmData();
        QFileInfo fileInfo(file_selection);
        abpimage_file_base_name = fileInfo.baseName();
    }
};

bool SirveApp::ValidateAbpFiles(const QString& path_to_image_file)
{
    AbpFileMetadata possible_abp_file_metadata = file_processor->LocateAbpFiles(path_to_image_file);

    if (!possible_abp_file_metadata.error_msg.isEmpty())
    {
        if (eng_data != NULL) { // previously this was nullptr??
            // if eng_data already initialized, allow user to re-select frames
            txt_start_frame->setEnabled(true);
            txt_stop_frame->setEnabled(true);
            btn_get_frames->setEnabled(true);
            txt_start_frame->setStyleSheet(orange_styleSheet);
            txt_stop_frame->setStyleSheet(orange_styleSheet);
        }
        else
        {
            txt_start_frame->setEnabled(false);
            txt_stop_frame->setEnabled(false);
            btn_get_frames->setEnabled(false);
        }

        QtHelpers::LaunchMessageBox(QString("Issue Finding File"), possible_abp_file_metadata.error_msg);

        return false;
    }

    abp_file_metadata = possible_abp_file_metadata;

    return true;
}

void SirveApp::LoadOsmData()
{
    ResetEngineeringDataAndSliderGUIs();

    OSMReader reader;
    if (!reader.Open(abp_file_metadata.osm_path))
    {
        QtHelpers::LaunchMessageBox(QString("Error loading OSM file"),
                                    QString("Error reading OSM file. Close program and open logs for details."));
        return;
    }

    osm_frames = reader.ReadFrames(abp_file_type);
    if (osm_frames.empty())
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
    if (eng_data != nullptr)
    {
        video_player_->ResetSlider();
        video_player_->SetPlaybackEnabled(false);

        // Reset video frame
        video_player_->StopTimer();
        ResetColorCorrection();

        // delete objects with existing data within them
        delete eng_data;
        delete track_info;
        delete engineering_plot_layout;

        state_manager_->clear();
        video_player_->RemoveFrame();

        histogram_plot->RemoveHistogramPlots();

        tab_menu->setTabEnabled(0, false);
        tab_menu->setTabEnabled(1, false);
        tab_menu->setTabEnabled(2, false);
        cmb_processing_states->setEnabled(false);
        txt_start_frame->setStyleSheet(orange_styleSheet);
        txt_stop_frame->setStyleSheet(orange_styleSheet);
    }

    eng_data = new EngineeringData(osm_frames);
    track_info = new TrackInformation(osm_frames, abp_file_type);

    plot_palette = new PlotPalette();

    //  Set up new plots as we do in the plot designer class:
    HandleParamsSelected("Azimuth", {Quantity("Azimuth", Enums::PlotUnit::Degrees), Quantity("Frames", Enums::PlotUnit::Undefined_PlotUnit)});
    HandleParamsSelected("Elevation",{Quantity("Elevation", Enums::PlotUnit::Degrees), Quantity("Frames", Enums::PlotUnit::Undefined_PlotUnit)});
    HandleParamsSelected("Irradiance",{Quantity("Irradiance", Enums::PlotUnit::Photons), Quantity("Frames", Enums::PlotUnit::Undefined_PlotUnit)});

    osmDataLoaded = true;

    connect(plot_palette, &PlotPalette::paletteParamsSelected, this, &SirveApp::HandleParamsSelected);

    size_t num_tracks = track_info->get_track_count();
    if (num_tracks == 0)
    {
        QtHelpers::LaunchMessageBox(QString("No Tracking Data"), "No tracking data was found within the file. No data will be plotted.");
    }

    plot_palette->GetEngineeringPlotReference(0)->past_midnight = eng_data->get_seconds_from_midnight();

    plot_palette->GetEngineeringPlotReference(0)->set_plotting_track_frames(track_info->get_osm_plotting_track_frames(), track_info->get_track_count());

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

    connect(plot_palette, &PlotPalette::editClassification, this, &SirveApp::EditClassificationText);
    connect(plot_palette, &PlotPalette::popoutPlot, this, &SirveApp::OpenPopoutEngineeringPlot);
    connect(plot_palette, &PlotPalette::toggleUseSubInterval, this, &SirveApp::HandlePlotFullDataToggle);
    connect(plot_palette, &PlotPalette::currentChanged, this, &SirveApp::HandlePlotFocusChanged);

    engineering_plot_layout->addWidget(plot_palette);

    frame_plots->setLayout(engineering_plot_layout);

    video_player_->SetRadianceCalculationEnabled(false);

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
    video_player_->InitializeToggles();

    // Reset setting engineering plot defaults
    menu_plot_all_data->setIconVisibleInMenu(true);
    menu_plot_primary->setIconVisibleInMenu(false);

    tab_plots->setCurrentIndex(1);

    for (int i = 0; i < plot_palette->tabBar()->count(); i++)
    {
        plot_palette->GetEngineeringPlotReference(i)->DefineFullPlotInterval();
    }

    UpdateGuiPostDataLoad(osmDataLoaded);
}

void SirveApp::HandleParamsSelected(QString plotTitle, const std::vector<Quantity> &quantities)
{
    EngineeringPlot *data_plot = new EngineeringPlot(osm_frames, plotTitle, quantities);
    data_plot->set_plotting_track_frames(track_info->get_osm_plotting_track_frames(), track_info->get_track_count());
    UpdatePlots(data_plot);

    int tab_count = plot_palette->tabBar()->count();
    if (tab_count > 0)
    {
        data_plot->set_use_subinterval(plot_palette->GetEngineeringPlotReference(0)->get_use_subinterval());
        if (data_plot->get_use_subinterval()){
            data_plot->SetPlotterXAxisMinMax(plot_palette->GetEngineeringPlotReference(0)->get_subinterval_min(),
                                             plot_palette->GetEngineeringPlotReference(0)->get_subinterval_max());
            data_plot->DefinePlotSubInterval(plot_palette->GetEngineeringPlotReference(0)->get_subinterval_min(),
                                             plot_palette->GetEngineeringPlotReference(0)->get_subinterval_max());
        }
        data_plot->DefineFullPlotInterval();
    }

    plot_palette->AddPlotTab(data_plot, quantities);
}

void SirveApp::UpdateGuiPostDataLoad(bool osm_data_status)
{
    // Enable plot capabilities
    //btn_plot_menu->setEnabled(osm_data_status);

    // Enable setting of epoch
    dt_epoch->setEnabled(osm_data_status);
    btn_apply_epoch->setEnabled(osm_data_status);

    // Enable frame range selection
    btn_get_frames->setEnabled(osm_data_status);
    txt_start_frame->setEnabled(osm_data_status);
    txt_stop_frame->setEnabled(osm_data_status);

    plot_groupbox->setEnabled(osm_data_status);
    //cmb_plot_yaxis->setEnabled(osm_data_status);
    //cmb_plot_xaxis->setEnabled(osm_data_status);

    osm_data_status ? tab_plots->tabBar()->show() : tab_plots->tabBar()->hide();
}

void SirveApp::UpdateGuiPostFrameRangeLoad(bool frame_range_status)
{
    video_player_->SetGotoFrameEnabled(frame_range_status);
    btn_popout_histogram->setEnabled(frame_range_status);
    action_show_calibration_dialog->setEnabled(frame_range_status);

    frame_range_status ? tab_menu->tabBar()->show() : tab_menu->tabBar()->hide();

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

    video_player_->ClearAnnotations();

    LoadAbirData(min_frame, max_frame);
    lbl_workspace_name->setText("Workspace File: ");

    plot_palette->SetAbirDataLoaded(true);

    QList<int> sizes = splitter->sizes();
    // expand frame for d data
    if (abp_file_type == ABPFileType::ABP_D)
    {
        // collapse right panel and let video take its space
        sizes[1] += sizes[2];
        sizes[2] = 0;

    }else
    {
        sizes[1] = centralWidgetStartingSize;
        sizes[2] = rightWidgetStartingSize;
    }
    splitter->setSizes(sizes);
    video_player_->SetVideoDimensions();
}

void SirveApp::LoadAbirData(int min_frame, int max_frame)
{
    DeleteAbirData();
    ResetEngineeringDataAndSliderGUIs();
    AllocateAbirData(min_frame, max_frame);
}

void SirveApp::DeleteAbirData()
{
    abir_frames.reset();
}

void SirveApp::AllocateAbirData(int min_frame, int max_frame)
{
    video_player_->StopTimer();
    state_manager_->clear();

    grpbox_progressbar_area->setEnabled(true);
    progress_bar_main->setTextVisible(true);
    progress_bar_main->setRange(0,100);

    // Task 1:
    progress_bar_main->setValue(0);
    lbl_progress_status->setText(QString("Loading ABIR data frames..."));

    WaitCursor cursor;
    abir_frames = file_processor->LoadImageFile(abp_file_metadata.image_path, min_frame, max_frame, abp_file_type);
    if (abir_frames == nullptr)
    {
        QtHelpers::LaunchMessageBox(QString("Error Reading ABIR Frames"), "Error reading .abpimage file. See log for more details.");
        btn_get_frames->setEnabled(true);
        return;
    }

    if (abir_frames->video_frames_16bit.empty())
    {
        QtHelpers::LaunchMessageBox(QString("Error Reading ABIR Frames"),
                                    "No valid frames were found in the .abpimage file. See log for more details.");
        btn_get_frames->setEnabled(true);
        return;
    }

    // Task 2:
    QCoreApplication::processEvents();
    progress_bar_main->setValue(0);
    lbl_progress_status->setText(QString("Deriving processing state..."));
    this->repaint();
    ProcessingState primary;
    auto number_frames = static_cast<uint32_t>(abir_frames->video_frames_16bit.size());
    auto x_pixels = abir_frames->x_pixels;
    progress_bar_main->setValue(20);
    auto y_pixels = abir_frames->y_pixels;
    progress_bar_main->setValue(40);
    auto max_value = abir_frames->max_value;
    progress_bar_main->setValue(60);

    primary.details = {
        x_pixels,
        y_pixels,
        max_value, abir_frames->video_frames_16bit
    };
    progress_bar_main->setValue(80);
    max_frame = abir_frames->last_valid_frame;
    progress_bar_main->setValue(100);

    // Task 3:
    QCoreApplication::processEvents();
    progress_bar_main->setValue(30);
    lbl_progress_status->setText(QString("Adding processing state..."));
    this->repaint();

    state_manager_->push_back(std::move(primary));
    progress_bar_main->setValue(80);

    this->repaint();

    txt_start_frame->setText(QString::number(min_frame));
    txt_stop_frame->setText(QString::number(max_frame));

    // Task 4:
    QCoreApplication::processEvents();
    progress_bar_main->setValue(0);
    lbl_progress_status->setText(QString("Configuring the chart plotter..."));
    this->repaint();

    int index0 = min_frame - 1;
    int index1 = max_frame;
    std::vector<PlottingFrameData> temp = eng_data->get_subset_plotting_frame_data(index0, index1);

    video_player_->InitializeTrackData(track_info->get_osm_frames(index0, index1), track_info->get_manual_frames(index0, index1));
    cmb_OSM_track_IDs->clear();
    cmb_OSM_track_IDs->addItem("Primary");
    cmb_manual_track_IDs->clear();
    cmb_manual_track_IDs->addItem("Primary");
    std::set<int> track_ids = track_info->get_OSM_track_ids();
    for ( int track_id : track_ids ){
        cmb_OSM_track_IDs->addItem(QString::number(track_id));
    }

    video_player_->InitializeFrameData(min_frame, std::move(temp), std::move(abir_frames->ir_data));
    // DeleteAbirData();
    video_player_->ReceiveVideoData(x_pixels, y_pixels);
    UpdateGlobalFrameVector();

    for (int i= 0; i < plot_palette->tabBar()->count(); i++)
    {
        plot_palette->GetEngineeringPlotReference(i)->SetPlotterXAxisMinMax(min_frame - 1, max_frame - 1);
        plot_palette->GetEngineeringPlotReference(i)->set_sub_plot_xmin(min_frame - 1);
        plot_palette->GetEngineeringPlotReference(i)->set_sub_plot_xmax(max_frame - 1);
        plot_palette->GetEngineeringPlotReference(i)->DefinePlotSubInterval(min_frame-1, max_frame - 1);
    }


    connect(video_player_, &VideoPlayer::frameNumberChanged, plot_palette, &PlotPalette::RouteFramelineUpdate);

    video_player_->UpdateFps();

    progress_bar_main->setValue(100);
    this->repaint();

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
    txt_auto_track_stop_frame->setText(QString::number(min_frame + 1));
    connect(txt_auto_track_start_frame, &QLineEdit::editingFinished,this, &SirveApp::HandleAutoTrackStartChangeInput);

    video_player_->SetPlaybackEnabled(true);
    UpdateGuiPostFrameRangeLoad(true);

    progress_bar_main->setValue(0);
    progress_bar_main->setTextVisible(false);
    grpbox_progressbar_area->setEnabled(false);
    lbl_progress_status->setText(QString(""));

    txt_FNS_start_frame->setText(txt_start_frame->text());
    int istop = txt_start_frame->text().toInt() + 50;
    txt_FNS_stop_frame->setText(QString::number(istop));
}

void SirveApp::OpenPopoutEngineeringPlot(int tab_index, QString plotTitle, std::vector<Quantity> quantities)
{
    QDialog *popoutDialog = new QDialog(this);
    QVBoxLayout *popoutDialogLayout = new QVBoxLayout(popoutDialog);
    EngineeringPlot *dialogPlotter = new EngineeringPlot(osm_frames, plotTitle, quantities);

    popoutDialogLayout->addWidget(plot_palette->GetEngineeringPlotReference(tab_index));

    if (plot_palette->HasSyncedTabWithIndex(tab_index))
    {
        plot_palette->GetEngineeringPlotReference(tab_index)->synchronizeToMaster(plot_palette->GetEngineeringPlotReference(plot_palette->GetLowestSyncedTabIndex()), JKQTBasePlotter::sdXAxis);
    }

    popoutDialog->setWindowTitle((*plot_palette->GetEngineeringPlotReference(tab_index)).get_plot_title() + "Tab" + QString::number(tab_index));
    popoutDialog->setAttribute(Qt::WA_DeleteOnClose);
    popoutDialog->resize(plot_palette->width(), plot_palette->height());
    popoutDialog->show();

    connect(popoutDialog, &QDialog::finished, this, &SirveApp::ClosePopoutEngineeringPlot);

    dialogPlotter->setToolbarAlwaysOn(true);

    plot_palette->AddPoppedTabIndex(tab_index);
}

void SirveApp::ClosePopoutEngineeringPlot()
{
    // Implementing Close this way avoids the cost of inheriting QDialog and making a new signal/slot...
    QObject *sendingObject = sender();
    QDialog *dialog = qobject_cast<QDialog *>(sendingObject);

    // Find the position of the word "Tab"
    QString title = dialog->windowTitle();
    int tabIndex = title.indexOf("Tab");
    QString afterTab = title.mid(tabIndex + 3).trimmed();

    bool ok;
    int tab_index = afterTab.toInt(&ok);

    QWidget* tabWidgetContent = plot_palette->widget(tab_index);
    tabWidgetContent->layout()->addWidget(plot_palette->GetEngineeringPlotReference(tab_index));

    plot_palette->tabBar()->setTabVisible(tab_index, true);
    plot_palette->tabBar()->setCurrentIndex(tab_index);
    plot_palette->update();
    plot_palette->adjustSize();
    plot_palette->RemovePoppedTabIndex(tab_index);
}

void SirveApp::HandlePlotFocusChanged(int tab_index)
{
    plot_palette->RouteFramelineUpdate(video_player_->GetCurrentFrameNumber());
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

void SirveApp::HandleZoomAfterSlider()
{
    // TODO: Re-enable this (using loop convention?)
    //data_plots->PlotCurrentStep(video_player_->GetCurrentFrameNumber());
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
    for (int i= 0; i < plot_palette->tabBar()->count(); i++)
    {
        plot_palette->GetEngineeringPlotReference(i)->ToggleUseSubInterval();
    }
}

void SirveApp::HandlePlotPrimaryOnlyToggle()
{
    plot_palette->GetEngineeringPlotReference(0)->set_plot_primary_only(!plot_palette->GetEngineeringPlotReference(0)->get_plot_primary_only());
    menu_plot_primary->setIconVisibleInMenu(plot_palette->GetEngineeringPlotReference(0)->get_plot_primary_only());
}

void SirveApp::SetDataTimingOffset()
{
    if (!eng_data)
        return;

    bool ok;
    double d = QInputDialog::getDouble(this, "Set Offset Time for Data", "Offset (seconds):", eng_data->get_offset_time(), -86400, 86400, 3, &ok, Qt::WindowFlags(), 1);
    if (ok) {
        eng_data->set_offset_time(d);

        plot_palette->GetEngineeringPlotReference(0)->past_midnight = eng_data->get_seconds_from_midnight();
        plot_palette->GetEngineeringPlotReference(0)->past_epoch = eng_data->get_seconds_from_epoch();

        int index0 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin();
        int index1 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmax();

        std::vector<PlottingFrameData> temp = eng_data->get_subset_plotting_frame_data(index0, index1);
        video_player_->UpdateFrameData(temp);
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

void SirveApp::EnableBinaryExport()
{
    bool ok;
    QString input_text = QInputDialog::getText(nullptr, "Enable Binary Export", "Enter Password", QLineEdit::Normal, "", &ok);

    if (!ok)
    {
        return;
    }
    int check = QString::compare(input_text, "KateRocks", Qt::CaseSensitive);
    if (check == 0)
    {
        action_export_current_frame->setEnabled(true);

        action_export_frame_range->setEnabled(true);

        action_export_all_frames->setEnabled(true);
    }
    else
    {
        return;
    }
}

void SirveApp::ExportFrame()
{
    if(video_player_->IsRunning()) {
        video_player_->StopTimer();
    }

    int nRows = state_manager_->front().details.y_pixels;
    int nCols = state_manager_->front().details.x_pixels;
    const auto& original_frame_vector = state_manager_->GetCurrentState().details.frames_16bit[video_player_->GetCounter()];
    arma::u32_mat frame_matrix = arma::reshape(arma::conv_to<arma::u32_vec>::from(original_frame_vector), nCols, nRows).t();
    QDate today = QDate::currentDate();
    QTime currentTime = QTime::currentTime();;
    QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");

    QString current_frame = QString::number(video_player_->GetCounter() + plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin() + 1);

    QString initial_name = abpimage_file_base_name + "_Frame_" + current_frame + "_" + formattedDate;

    QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save File Name"), config_values.workspace_folder + "/" + initial_name, tr("Data files *.bin"));
    (void)frame_matrix.save(savefile_name.toStdString(), arma::arma_binary);

    if(video_player_->IsRunning()) {
        video_player_->StartTimer();
    }
}

void SirveApp::ExportFrameRange()
{
    if(video_player_->IsRunning()) {
        video_player_->StopTimer();
    }

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
        int nRows = state_manager_->front().details.y_pixels;
        int nCols = state_manager_->front().details.x_pixels;
        QString start_frame = fields.at(0)->text();
        QString end_frame = fields.at(1)->text();
        int startframe = start_frame.toInt() - 1;
        int endframe = end_frame.toInt() - 1;
        int num_video_frames = endframe - startframe + 1;
        int start_framei = startframe - plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin();
        int end_framei = start_framei + num_video_frames - 1;
        arma::u32_cube frame_cube(nRows,nCols,num_video_frames);
        int k = 0;
        const auto& current_state = state_manager_->GetCurrentState();
        for (int framei = start_framei; framei < end_framei; framei++){
            const auto& original_frame_vector = current_state.details.frames_16bit[framei];
            frame_cube.slice(k) = arma::reshape(arma::conv_to<arma::u32_vec>::from(original_frame_vector),nCols,nRows).t();
            k+=1;
        }

        QDate today = QDate::currentDate();
        QTime currentTime = QTime::currentTime();;
        QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
        QString initial_name = abpimage_file_base_name + "_Frames_" + start_frame + "_" + end_frame + "_" + formattedDate;
        QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save File Name"), config_values.workspace_folder + "/" + initial_name, tr("Data files *.bin"));
        (void)frame_cube.save(savefile_name.toStdString(),arma::arma_binary);
    }

    if(video_player_->IsRunning()) {
        video_player_->StartTimer();
    }
}

void SirveApp::ExportAllFrames()
{
    if(video_player_->IsRunning()) {
        video_player_->StopTimer();
    }

    int min_frame = ConvertFrameNumberTextToInt(txt_start_frame->text());
    int max_frame = ConvertFrameNumberTextToInt(txt_stop_frame->text());
    QString start_frame = QString::number(min_frame);
    QString end_frame = QString::number(max_frame);
    size_t num_video_frames = state_manager_->front().details.frames_16bit.size();
    int nRows = state_manager_->front().details.y_pixels;
    int nCols = state_manager_->front().details.x_pixels;
    arma::u32_cube frame_cube(nRows,nCols,num_video_frames);
    const auto& current_state = state_manager_->GetCurrentState();
    for (size_t framei = 0; framei < num_video_frames ; framei++){
        const auto& original_frame_vector = current_state.details.frames_16bit[framei];
        frame_cube.slice(framei) = arma::reshape(arma::conv_to<arma::u32_vec>::from(original_frame_vector),nCols,nRows).t();
    }

    QDate today = QDate::currentDate();
    QTime currentTime = QTime::currentTime();;
    QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
    QString initial_name = abpimage_file_base_name + "_Frames_" + start_frame + "_" + end_frame + "_" + formattedDate;
    QString savefile_name = QFileDialog::getSaveFileName(this, tr("Save File Name"), config_values.workspace_folder + "/" + initial_name, tr("Data files *.bin"));
    (void)frame_cube.save(savefile_name.toStdString(),arma::arma_binary);

    if(video_player_->IsRunning()) {
        video_player_->StartTimer();
    }
}

void SirveApp::CreateMenuActions()
{
    QIcon on(":/icons/check.png");

    action_load_OSM_B = new QAction("Load B Data");
    action_load_OSM_B->setStatusTip("Load OSM abpimage file");
    connect(action_load_OSM_B, &QAction::triggered, this, &SirveApp::HandleAbpBFileSelected);

    action_load_OSM_D = new QAction("Load D Data");
    action_load_OSM_D->setStatusTip("Load OSM abpimage file");
    connect(action_load_OSM_D, &QAction::triggered, this, &SirveApp::HandleAbpDFileSelected);

    action_show_calibration_dialog = new QAction("Setup Calibration");
    connect(action_show_calibration_dialog, &QAction::triggered, this, &SirveApp::ShowCalibrationDialog);
    action_show_calibration_dialog->setEnabled(false);

    action_close = new QAction("Close");
    action_close->setStatusTip("Close main window");
    connect(action_close, &QAction::triggered, this, &SirveApp::CloseWindow);

    action_load_workspace = new QAction("Load Workspace File");
    connect(action_load_workspace, &QAction::triggered, this, &SirveApp::LoadWorkspace);

    action_save_workspace = new QAction("Save Workspace File");
    connect(action_save_workspace, &QAction::triggered, this, &SirveApp::SaveWorkspace);

    action_change_workspace_directory = new QAction("Change Workspace Directory");
    action_change_workspace_directory->setStatusTip("Customize workspace directory so it points to your own folder.");
    connect(action_change_workspace_directory, &QAction::triggered, this, &SirveApp::ChangeWorkspaceDirectory);

    action_enable_binary_export = new QAction("Enable Binary Export");
    connect(action_enable_binary_export, &QAction::triggered, this, &SirveApp::EnableBinaryExport);
    action_enable_binary_export->setEnabled(true);

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
	file_menu->addAction(action_load_OSM_B);
	file_menu->addAction(action_load_OSM_D);
    file_menu->addAction(action_show_calibration_dialog);
    file_menu->addAction(action_close);
    menu_workspace = menuBar()->addMenu(tr("&Workspace"));
    menu_workspace->addAction(action_load_workspace);
    menu_workspace->addAction(action_save_workspace);
    menu_workspace->addAction(action_change_workspace_directory);

    menu_export = menuBar()->addMenu(tr("&Export"));
    menu_export->addAction(action_export_tracking_data);
    menu_export->addAction(action_enable_binary_export);
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

    menu_plot_primary = new QAction(tr("&Plot Primary Data Only"), this);
    menu_plot_primary->setIcon(on);
    menu_plot_primary->setStatusTip(tr("Plot only the primary object"));
    menu_plot_primary->setIconVisibleInMenu(false);
    connect(menu_plot_primary, &QAction::triggered, this, &SirveApp::HandlePlotPrimaryOnlyToggle);

    // ---------------------- Set Acctions to Menu --------------------

    plot_menu = new QMenu(this);
    plot_menu->addAction(menu_plot_all_data);
    plot_menu->addAction(menu_plot_primary);
}

ImageProcessing* SirveApp::CreateImageProcessor()
{
    QPointer image_processor = new ImageProcessing(abp_file_type);
    connect(image_processor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, image_processor, &ImageProcessing::CancelOperation);

    return image_processor;
}

void SirveApp::EditBannerText()
{
    bool ok;
    QString input_text = QInputDialog::getText(nullptr, "Banner Text", "Input Banner Text", QLineEdit::Normal,
        video_player_->GetBannerText(), &ok);

    if (!ok)
    {
        return;
    }

    video_player_->UpdateBannerText(input_text);

    if (!UpdateClassificationIfExists("VideoDisplay", input_text, &classification_list))
    {
        Classification classification(QString(input_text), QString("VideoDisplay"));
        classification_list.push_back(classification);
    }

    auto response = QtHelpers::LaunchYesNoMessageBox("Update All Banners", "Video and plot banners do not match. Would you like to set both to the same banner?");
    if (response == QMessageBox::Yes)
    {
        plot_palette->GetEngineeringPlotReference(0)->SetPlotClassification(input_text);
    }
}

void SirveApp::EditClassificationText(int plot_tab_index, QString current_value)
{
    bool ok;

    QString input_text = QInputDialog::getText(0, "Plot Classification", "Input Plot Classification Text", QLineEdit::Normal, current_value, &ok);

    if (ok)
    {
        // Implementing Close this way avoids the cost of inheriting QDialog and making a new signal/slot...
        QObject *sendingObject = sender();
        PlotPalette *palette = qobject_cast<PlotPalette *>(sendingObject);
        palette->UpdatePlotLabel(plot_tab_index, input_text);

    }
}

void SirveApp::ShowCalibrationDialog()
{
	CalibrationDialog calibrate_dialog(calibration_model, abp_file_type);

	auto response = calibrate_dialog.exec();

	if (response == 0) {

		return;
	}

	calibration_model = calibrate_dialog.model;
	video_player_->SetCalibrationModel(std::move(calibrate_dialog.model));
	video_player_->SetRadianceCalculationEnabled(true);
}

void SirveApp::ExportPlotData()
{
    QString start_frame = txt_start_frame->text();
    QString stop_frame = txt_stop_frame->text();
    QStringList items;
    items << "Export Only Selected Data" << "Export All Data" ;

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

    if (item == "Export All Data")
    {
        DataExport::WriteTrackDataToCsv(save_path, eng_data->get_plotting_frame_data(), track_info->get_osm_plotting_track_frames(), track_info->get_manual_plotting_frames());
    }
    else {

        int min_frame = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin();
        int max_frame = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmax();

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

int SirveApp::GetCurrentColorIndex(const QVector<QString>& colors, const QColor& input_color) {

    int index_current_color = -1;
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

StateManager& SirveApp::GetStateManager()
{
    return *state_manager_;
}

const StateManager& SirveApp::GetStateManager() const
{
    return *state_manager_;
}

void SirveApp::EditColorMap()
{
    // QString color = cmb_color_maps->currentText();

    int i = cmb_color_maps->currentIndex();

    color_map_display->set_color_map(video_colors.maps[i].colors,lbl_lift_value->text().toDouble(),lbl_gain_value->text().toDouble());
    video_player_->SetColorMap(video_colors.maps[i].colors);

}

void SirveApp::EditBannerColor()
{
    QString color = cmb_text_color->currentText();
    video_player_->UpdateBannerColor(std::move(color));
}

void SirveApp::EditOSMTrackColor()
{
    // QString tracker_color = cmb_OSM_track_color->currentText();
    QStringList color_options = ColorScheme::get_track_colors();
    QColor color = color_options[cmb_OSM_track_color->currentIndex()];

    video_player_->SetTrackerColor(color);

    // UpdatePlots(); //Note: Engineering_Plots does not yet control its own graphical updates like VideoDisplay
}

void SirveApp::HandleOutlierProcessingChange()
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

void SirveApp::EditBadPixelColor()
{
    QString bad_pixel_color = cmb_bad_pixel_color->currentText();
    video_player_->HighlightBadPixelsColors(bad_pixel_color);
}

void SirveApp::UpdatePlots(EngineeringPlot *engineering_plot)
{
    if (eng_data)
    {
        engineering_plot->PlotChart();
        engineering_plot->PlotCurrentFrameline(video_player_->GetCurrentFrameNumber());
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
            }
        }
    }

    yAxisChanged = false;
}

void SirveApp::AnnotateVideo()
{
    VideoInfo standard_info;
    standard_info.x_pixels = video_player_->GetImageWidth();
    standard_info.y_pixels = video_player_->GetImageHeight();

    standard_info.min_frame = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin() + 1;
    standard_info.max_frame = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmax() + 1;

    standard_info.x_correction = video_player_->GetXCorrection();
    standard_info.y_correction = video_player_->GetYCorrection();

    annotation_dialog = new AnnotationListDialog(video_player_->GetAnnotations(), standard_info, abp_file_type);
    connect(annotation_dialog, &AnnotationListDialog::showAnnotationStencil, video_player_, &VideoPlayer::ShowStencil);
    connect(annotation_dialog, &AnnotationListDialog::hideAnnotationStencil, video_player_, &VideoPlayer::HideStencil);
    connect(annotation_dialog, &AnnotationListDialog::updateAnnotationStencil, video_player_, &VideoPlayer::InitializeStencilData);
    connect(annotation_dialog, &AnnotationListDialog::accepted, this, &SirveApp::HandleAnnotationDialogClosed); // Ok

    connect(video_player_, &VideoPlayer::annotationStencilMouseMoved, annotation_dialog, &AnnotationListDialog::UpdateStencilPosition, Qt::UniqueConnection);
    connect(video_player_, &VideoPlayer::annotationStencilMouseReleased, annotation_dialog, &AnnotationListDialog::SetStencilLocation, Qt::UniqueConnection);

    btn_add_annotations->setDisabled(true);

    annotation_dialog->show();
}

void SirveApp::HandleAnnotationDialogClosed()
{
    btn_add_annotations->setDisabled(false);
    annotation_dialog = nullptr;
}

int SirveApp::ConvertFrameNumberTextToInt(const QString& input)
{
    bool convert_value_numeric;
    int value = input.toInt(&convert_value_numeric);

    if (convert_value_numeric) {
        return value;
    }

    return -1;
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

    plot_palette->GetEngineeringPlotReference(0)->past_epoch = eng_data->get_seconds_from_epoch();
    UpdatePlots(plot_palette->GetEngineeringPlotReference(0));
}

void SirveApp::HandleBadPixelReplacement()
{
    const ProcessingState& cur_state = GetStateManager()[cmb_processing_states->currentIndex()];
    std::vector<std::vector<uint16_t>> test_data;
    int number_video_frames = static_cast<int>(cur_state.details.frames_16bit.size());

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

        WaitCursor cursor;
        abir_frames = file_processor->LoadImageFile(abp_file_metadata.image_path, start_frame, stop_frame, abp_file_type);
        if (abir_frames == nullptr)
        {
            QtHelpers::LaunchMessageBox(QString("Error loading frames."),
                                        "Check that the file exists and is not corrupted.");
            return;
        }
        test_data = abir_frames->video_frames_16bit;
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
        test_data = {cur_state.details.frames_16bit.begin()+ start_offset, cur_state.details.frames_16bit.begin()+stop_offset};
    }

    OpenProgressArea("Finding bad pixels",number_video_frames - 1);
    QPointer image_processor = CreateImageProcessor();

    std::vector<unsigned int> dead_pixels;

    if(type_choice == 0){
        lbl_progress_status->setText(QString("Finding dead pixels..."));
        arma::uvec index_dead0 = image_processor->FindDeadBadscalePixels(test_data);
        lbl_progress_status->setText(QString("Finding outlier pixels..."));
        if (outlier_method == 0){
            arma::uvec index_outlier0 = image_processor->IdentifyBadPixelsMedian(N,test_data);
            index_outlier0 = arma::unique(arma::join_vert(index_outlier0,index_dead0));
            dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier0);
        }
        else{
            u_int window_length = txt_moving_median_N->text().toUInt();
            arma::uvec index_outlier0 = image_processor->IdentifyBadPixelsMovingMedian(window_length,N,test_data);
            index_outlier0 = arma::unique(arma::join_vert(index_outlier0,index_dead0));
            dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier0);
        }
    } else if (type_choice == 1){
        lbl_progress_status->setText(QString("Finding outlier pixels..."));
        arma::uvec index_dead1 = image_processor->FindDeadBadscalePixels(test_data);
        dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_dead1);
    } else {
        lbl_progress_status->setText(QString("Finding outlier pixels..."));
        if (outlier_method == 0){
            arma::uvec index_outlier2 = image_processor->IdentifyBadPixelsMedian(N,test_data);
            dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier2);
        }
        else{
            u_int window_length = txt_moving_median_N->text().toUInt();
            arma::uvec index_outlier2 = image_processor->IdentifyBadPixelsMovingMedian(window_length,N,test_data);
            dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier2);
        }
    }

    if(dead_pixels.size()>0){
        ReplaceBadPixels(dead_pixels,cmb_processing_states->currentIndex());
    }

    CloseProgressArea();
}

void SirveApp::ReplaceBadPixels(std::vector<unsigned int> pixels_to_replace, int source_state_idx)
{
    if (pixels_to_replace.empty()) {
        return;
    }

    int num_frames = (int)GetStateManager()[source_state_idx].details.frames_16bit.size();
    OpenProgressArea("Replacing bad pixels...",num_frames - 1);

    auto new_state = GetStateManager()[source_state_idx];

    QPointer image_processor = CreateImageProcessor();
    image_processor->ReplacePixelsWithNeighbors(
        new_state.details.frames_16bit, pixels_to_replace, new_state.details.x_pixels);

    state_manager_->push_back(std::move(new_state), ProcessingMethod::replace_bad_pixels);
    auto& last = state_manager_->back();
    last.replaced_pixels = std::move(pixels_to_replace);

    // TODO: Shouldn't this be handled by a signal?
    UpdateGlobalFrameVector();

    lbl_bad_pixel_count->setText("Bad pixels currently replaced: " + QString::number(pixels_to_replace.size()));
    chk_highlight_bad_pixels->setEnabled(true);

    CloseProgressArea();
}

void SirveApp::ReceiveNewBadPixels(const std::vector<unsigned int>& new_pixels)
{
    auto& current_state = state_manager_->GetCurrentState();
    int current_state_idx = current_state.state_ID;
    ProcessingMethod method = current_state.method;
    if (method == ProcessingMethod::replace_bad_pixels)
    //If current state is already a new state with replaced pixels, just add new pixels to the replaced pixels
    {
        auto bad_pixels = current_state.replaced_pixels;

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
            QPointer image_processor = CreateImageProcessor();
            image_processor->ReplacePixelsWithNeighbors(current_state.details.frames_16bit, new_pixels, current_state.details.x_pixels);

            uint16_t maxVal = std::numeric_limits<uint>::min(); // Initialize with the smallest possible int
            for (const auto& row : current_state.details.frames_16bit) {
                maxVal = std::max(maxVal, *std::max_element(row.begin(), row.end()));
            }
            current_state.details.max_value = maxVal;
            current_state.replaced_pixels = bad_pixels;
            lbl_bad_pixel_count->setText("Bad pixels currently replaced: " + QString::number(bad_pixels.size()));

            current_state.UpdateDescription();
            lbl_processing_description->setText(current_state.state_description);

            UpdateGlobalFrameVector();
        }
    }
    else
    //Create a new replaced pixel state
    {
        ReplaceBadPixels(new_pixels, current_state_idx);
    }
}

void SirveApp::ReceiveNewGoodPixels(const std::vector<unsigned int>& pixels)
{
    // Note: This was streamlined in a big way.
    auto& current_state = state_manager_->GetCurrentState();
    auto& source_state = GetStateManager()[current_state.source_state_ID];

    //If current state is already a new state with replaced pixels, just replace pixels from source state
    if (current_state.method == ProcessingMethod::replace_bad_pixels) {
        auto& replaced_pixels = current_state.replaced_pixels;
        for (size_t i = 0; i < pixels.size(); i++) {
            unsigned int candidate_pixel = pixels[i];
            auto position = std::find(replaced_pixels.begin(), replaced_pixels.end(), candidate_pixel);
            if (position != replaced_pixels.end()) {
                replaced_pixels.erase(position);
                for (size_t framei = 0; framei < current_state.details.frames_16bit.size(); framei++)
                {
                    current_state.details.frames_16bit[framei][pixels[i]] = source_state.details.frames_16bit[framei][pixels[i]];
                }
            }
        }
        lbl_bad_pixel_count->setText("Bad pixels currently replaced: " + QString::number(replaced_pixels.size()));
        current_state.UpdateDescription();
        lbl_processing_description->setText(current_state.state_description);
        UpdateGlobalFrameVector();
    }
    else
    {
        QtHelpers::LaunchMessageBox("No Action Taken", "Pixels can only be recovered from 'Replace Bad Pixels' states.");
    }
}

void SirveApp::ReceiveProgressBarUpdate(int percent) const
{
    progress_bar_main->setValue(percent);
}

bool SirveApp::CheckCurrentStateisNoiseSuppressed(int source_state_idx) const
{
    static const std::set test_set = {
        ProcessingMethod::fixed_noise_suppression,
        ProcessingMethod::accumulator_noise_suppression,
        ProcessingMethod::adaptive_noise_suppression,
        ProcessingMethod::RPCP_noise_suppression
    };

    ProcessingMethod currentMethod = GetStateManager()[source_state_idx].method;
    if (test_set.count(currentMethod) > 0) {
        return true;
    }
    return false;
}

void SirveApp::ApplyFixedNoiseSuppressionFromExternalFile()
{
    ExternalNUCInformationWidget external_nuc_dialog(abp_file_type);

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
            response = QtHelpers::LaunchYesNoMessageBox("Current state is already noise suppressed.", "Continue?");
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
    catch (const std::exception&)
    {
        // catch any errors when loading frames. try-catch not needed when loading frames from same file since no errors originally occurred
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
    video_player_->StopTimer();

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

void SirveApp::ApplyFixedNoiseSuppression(const QString& image_path, const QString& file_path, unsigned int frame0, unsigned int start_frame, unsigned int stop_frame, int source_state_idx)
{
    int compare = QString::compare(file_path, image_path, Qt::CaseInsensitive);
    if (compare!=0){
        if (!VerifyFrameSelection(start_frame, stop_frame))
        {
            QtHelpers::LaunchMessageBox(QString("Invalid Frame Selection"), "Fixed noise suppression not completed, invalid frame selection");
            return;
        }
    }

    int num_frames = (int)GetStateManager()[source_state_idx].details.frames_16bit.size();

    OpenProgressArea("Fixed median noise suppression...", num_frames - 1);
    QPointer image_processor = CreateImageProcessor();

    auto new_state = GetStateManager()[source_state_idx];

    new_state.details.frames_16bit = image_processor->FixedNoiseSuppression(abp_file_metadata.image_path, file_path,
        frame0, start_frame, stop_frame, abp_file_type, new_state.details);

    if(new_state.details.frames_16bit.size() > 0) {
        state_manager_->push_back(std::move(new_state), ProcessingMethod::fixed_noise_suppression);
        auto& last = state_manager_->back();
        last.FNS_file_path = file_path;
        last.frame0 = frame0;
        last.FNS_start_frame = start_frame;
        last.FNS_stop_frame = stop_frame;
        last.source_state_ID = source_state_idx;

        // Shouldn't this be handled by a signal?
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

void SirveApp::OpenProgressArea(const QString& message, int N)
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

    int num_frames = (int)GetStateManager()[source_state_idx].details.frames_16bit.size();
    OpenProgressArea("Deinterlacing...", num_frames - 1);
    auto new_state = GetStateManager()[source_state_idx];

    QPointer image_processor = CreateImageProcessor();

    connect(image_processor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, image_processor, &ImageProcessing::CancelOperation);

    new_state.details.frames_16bit = image_processor->DeinterlaceOpenCVPhaseCorrelation(new_state.details);

    if(new_state.details.frames_16bit.size() > 0) {
        state_manager_->push_back(std::move(new_state), ProcessingMethod::deinterlace);

        // TODO: Shouldn't this be handled by a signal?
        UpdateGlobalFrameVector();
    }

    CloseProgressArea();
}

void SirveApp::ApplyDeinterlacingCurrent()
{
    ProcessingState& original = GetStateManager()[cmb_processing_states->currentIndex()];
    int num_frames = (int)original.details.frames_16bit.size();
    OpenProgressArea("Deinterlacing...", num_frames - 1);
    QPointer image_processor = CreateImageProcessor();
    lbl_progress_status->setText(QString("Deinterlacing..."));
    int framei = video_player_->GetCounter();
    const auto& current_frame_16bit = original.details.frames_16bit[framei];

    auto& current = state_manager_->GetCurrentState();
    current.details.frames_16bit[video_player_->GetCounter()] = image_processor->DeinterlacePhaseCorrelationCurrent(framei, current_frame_16bit);
    lbl_progress_status->setText(QString(""));
    UpdateGlobalFrameVector();

    if(chk_deinterlace_confirmation->isChecked()) {
        auto response = QtHelpers::LaunchYesNoMessageBox("Deinterlace Frame Confirmation", "Keep result? (can not be undone after accepted)");
        if (response != QMessageBox::Yes) {
            current.details.frames_16bit[video_player_->GetCounter()] = current_frame_16bit;
            UpdateGlobalFrameVector();
        }
    }
}

void SirveApp::ExecuteCenterOnTracks()
{
    int OSM_track_id, manual_track_id;
    boolean findAnyTrack = false;
    QString trackFeaturePriority;
    bool continueTF = true;

    if (cmb_OSM_track_IDs->currentIndex()==0){
        OSM_track_id = -1;
    }
    else{
        OSM_track_id = cmb_OSM_track_IDs->currentText().toInt();
    }

    if (cmb_manual_track_IDs->currentIndex()==0){
        manual_track_id = -1;
    }
    else{
        manual_track_id = cmb_manual_track_IDs->currentText().toInt();
    }

    if (cmb_track_centering_priority->currentIndex()==0 || cmb_track_centering_priority->currentIndex()==2){
        trackFeaturePriority = "OSM";
    }

    if(cmb_track_centering_priority->currentIndex()==1 || cmb_track_centering_priority->currentIndex()==3){
        trackFeaturePriority = "Manual";
    }

    if(cmb_track_centering_priority->currentIndex()==2 || cmb_track_centering_priority->currentIndex()==3){
        findAnyTrack = true;
    }

    std::vector<std::vector<int>> track_centered_offsets;
    int source_state_idx = cmb_processing_states->currentIndex();
    if (GetStateManager()[source_state_idx].offsets.size()>0)
    {
        QtHelpers::LaunchMessageBox(QString("Not allowed"), "Current state is already centered.");
        continueTF = false;
    }
    if (continueTF)
    {
        CenterOnTracks(trackFeaturePriority, OSM_track_id, manual_track_id, track_centered_offsets, findAnyTrack, source_state_idx);
    }
}

void SirveApp::CenterOnTracks(const QString& trackFeaturePriority, int OSM_track_id, int manual_track_id, std::vector<std::vector<int>> & track_centered_offsets, boolean find_any_tracks, int source_state_idx)
{
    int num_frames = (int)GetStateManager()[source_state_idx].details.frames_16bit.size();
    OpenProgressArea("Centering on tracks...",num_frames - 1);

    auto new_state = GetStateManager()[source_state_idx];

    int osm_priority = QString::compare(trackFeaturePriority,"OSM",Qt::CaseInsensitive);
    int min_frame = ConvertFrameNumberTextToInt(txt_start_frame->text());
    int max_frame = ConvertFrameNumberTextToInt(txt_stop_frame->text());

    std::vector<TrackFrame> osmFrames = track_info->get_osm_frames(min_frame - 1, max_frame);
    std::vector<TrackFrame> manualFrames = track_info->get_manual_frames(min_frame - 1, max_frame);

    new_state.find_any_tracks = find_any_tracks;

    QPointer image_processor = CreateImageProcessor();

    new_state.details.frames_16bit = image_processor->CenterOnTracks(trackFeaturePriority, new_state.details,
        OSM_track_id, manual_track_id, osmFrames, manualFrames, find_any_tracks, track_centered_offsets);

    if (new_state.details.frames_16bit.size() > 0)
    {
        auto method = osm_priority == 0 ? ProcessingMethod::center_on_OSM : ProcessingMethod::center_on_manual;
        state_manager_->push_back(std::move(new_state), method);
        auto& last = state_manager_->back();
        last.offsets = track_centered_offsets;

        osm_priority = QString::compare(trackFeaturePriority, "OSM", Qt::CaseInsensitive);
        int track_id = osm_priority == 0 ? OSM_track_id : manual_track_id;
        last.track_id = track_id;

        arma::mat offsets_matrix;
        SharedTrackingFunctions::CreateOffsetMatrix(0,num_frames-1,last, offsets_matrix);
        last.offsets_matrix = offsets_matrix;

        // TODO: Shouldn't this be handled by a signal?
        UpdateGlobalFrameVector();
    }
    CloseProgressArea();
}

void SirveApp::CenterOnOffsets(const QString& trackFeaturePriority, int track_id, const std::vector<std::vector<int>> & track_centered_offsets, boolean find_any_tracks, int source_state_idx)
{
    int num_frames = (int)GetStateManager()[source_state_idx].details.frames_16bit.size();
    OpenProgressArea("Centering on offsets...",num_frames - 1);

    auto new_state = GetStateManager()[source_state_idx];

    QPointer image_processor = CreateImageProcessor();

    new_state.details.frames_16bit = image_processor->CenterImageFromOffsets(new_state.details, track_centered_offsets);

    if (new_state.details.frames_16bit.size()>0) {
        int OSMPriority = QString::compare(trackFeaturePriority,"OSM",Qt::CaseInsensitive);
        auto method = OSMPriority == 0 ? ProcessingMethod::center_on_OSM : ProcessingMethod::center_on_manual;

        state_manager_->push_back(std::move(new_state), method);
        auto& last = state_manager_->back();

        last.find_any_tracks = find_any_tracks;
        last.track_id = track_id;
        last.offsets = track_centered_offsets;

        arma::mat offsets_matrix;
        SharedTrackingFunctions::CreateOffsetMatrix(0,num_frames-1,last, offsets_matrix);
        last.offsets_matrix = std::move(offsets_matrix);

        // TODO: Shouldn't this be handled by a signal?
        UpdateGlobalFrameVector();
    }
    CloseProgressArea();
}

void SirveApp::ExecuteCenterOnBrightest()
{
    std::vector<std::vector<int>> brightest_centered_offsets;
    int source_state_idx = cmb_processing_states->currentIndex();
    bool continueTF = true;

    if (GetStateManager()[source_state_idx].offsets.size()>0)
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
    int num_frames = (int)GetStateManager()[source_state_idx].details.frames_16bit.size();
    OpenProgressArea("Centering on brightest...",num_frames - 1);
    auto new_state = GetStateManager()[source_state_idx];

    lbl_progress_status->setText(QString("Center on Brightest Object..."));

    QPointer image_processor = new ImageProcessing(abp_file_type);

    connect(image_processor, &ImageProcessing::signalProgress, progress_bar_main, &QProgressBar::setValue);
    connect(btn_cancel_operation, &QPushButton::clicked, image_processor, &ImageProcessing::CancelOperation);

    new_state.details.frames_16bit = image_processor->CenterOnBrightest(new_state.details, brightest_centered_offsets);

    if (new_state.details.frames_16bit.size() > 0){
        state_manager_->push_back(std::move(new_state), ProcessingMethod::center_on_brightest);
        auto& last = state_manager_->back();
        last.offsets = brightest_centered_offsets;

        arma::mat offsets_matrix;
        SharedTrackingFunctions::CreateOffsetMatrix(0,num_frames-1,last, offsets_matrix);
        last.offsets_matrix = offsets_matrix;

        // TODO: Shouldn't this be handled by a signal?
        UpdateGlobalFrameVector();
    }
    CloseProgressArea();
}

// TODO:Verify this works as expected
void SirveApp::HandleOsmTracksToggle()
{
    bool current_status = chk_show_OSM_tracks->isChecked();
    video_player_->ToggleOsmTracks(current_status);
    if (current_status) {
        cmb_OSM_track_color->setEnabled(true);
        QStringList color_options = ColorScheme::get_track_colors();
        QColor color = color_options[cmb_OSM_track_color->currentIndex()];

        video_player_->SetTrackerColor(std::move(color));
    }
    else
    {
        cmb_OSM_track_color->setEnabled(false);
        QColor color = QColor(0,0,0,0);

        video_player_->SetTrackerColor(std::move(color));
    }
}

void SirveApp::HandlePlayerStateChanged(bool status)
{
    txt_start_frame->setDisabled(status);
    txt_stop_frame->setDisabled(status);
    btn_get_frames->setDisabled(status);
}

void SirveApp::HandleNewProcessingState(const QString& state_name, const QString& combobox_state_name, int index)
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

    const auto& state = GetStateManager()[cmb_processing_states->currentIndex()];

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
    video_player_->StopTimer();

    int num_frames = (int)GetStateManager()[source_state_idx].details.frames_16bit.size();
    OpenProgressArea("Frame stacking...",num_frames - 1);
    auto new_state = GetStateManager()[source_state_idx];

    QPointer image_processor = CreateImageProcessor();
    new_state.details.frames_16bit = image_processor->FrameStacking(number_of_frames, new_state.details);

    if (new_state.details.frames_16bit.size() > 0){
        state_manager_->push_back(std::move(new_state), ProcessingMethod::frame_stacking);
        auto& last = state_manager_->back();
        last.frame_stack_num_frames = number_of_frames;

        UpdateGlobalFrameVector();
    }

    CloseProgressArea();
}

void SirveApp::ExecuteAdaptiveNoiseSuppression()
{
    //-----------------------------------------------------------------------------------------------
    // get user selected frames for suppression

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
    video_player_->StopTimer();

    int num_frames = (int)GetStateManager()[source_state_idx].details.frames_16bit.size();
    OpenProgressArea("Adaptive median background noise suppression...",num_frames - 1);

    auto new_state = GetStateManager()[source_state_idx];
    QPointer image_processor = CreateImageProcessor();

    if (GetAvailableMemoryRatio(num_frames, abp_file_type) >=1.5) {
        new_state.details.frames_16bit =
            image_processor->AdaptiveNoiseSuppressionMatrix(relative_start_frame, number_of_frames, new_state.details);
    } else {
        new_state.details.frames_16bit =
            image_processor->AdaptiveNoiseSuppressionByFrame(relative_start_frame, number_of_frames, new_state.details);
    }

    if (new_state.details.frames_16bit.size() > 0){
        state_manager_->push_back(std::move(new_state), ProcessingMethod::adaptive_noise_suppression);
        auto& last = state_manager_->back();
        last.ANS_relative_start_frame = relative_start_frame;
        last.ANS_num_frames = number_of_frames;

        QString description = "Filter starts at ";
        if (relative_start_frame > 0) {
            description += "+";
        }
        description += QString::number(relative_start_frame) + " frames and averages " +
            QString::number(number_of_frames) + " frames";

        lbl_adaptive_noise_suppression_status->setWordWrap(true);
        lbl_adaptive_noise_suppression_status->setText(description);

        // TODO: Shouldn't this be handled by a signal?
        UpdateGlobalFrameVector();
    }
    CloseProgressArea();
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
    video_player_->StopTimer();

    int num_frames = (int)GetStateManager()[source_state_idx].details.frames_16bit.size();
    if(GetAvailableMemoryRatio(num_frames, abp_file_type) < 1.5) {
        QtHelpers::LaunchMessageBox("Low memory", "Insufficient memory for this operation. Please select fewer frames.");
        return;
    }

    OpenProgressArea("RPCP noise suppression...",num_frames - 1);

    auto new_state = GetStateManager()[source_state_idx];

    QPointer image_processor = CreateImageProcessor();
    new_state.details.frames_16bit = image_processor->RPCPNoiseSuppression(new_state.details);

    if (new_state.details.frames_16bit.size() > 0){
        state_manager_->push_back(std::move(new_state), ProcessingMethod::RPCP_noise_suppression);

        // TODO: Shouldn't this be handled by a signal?
        UpdateGlobalFrameVector();
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
    video_player_->StopTimer();

    int num_frames = (int)GetStateManager()[source_state_idx].details.frames_16bit.size();
    OpenProgressArea("Rolling mean noise suppression...",num_frames - 1);

    auto new_state = GetStateManager()[source_state_idx];

    QPointer image_processor = CreateImageProcessor();
    new_state.details.frames_16bit = image_processor->AccumulatorNoiseSuppression(weight, offset, shadow_sigma_thresh,
        new_state.details, hide_shadow_choice);

    if(new_state.details.frames_16bit.size() > 0) {
        state_manager_->push_back(std::move(new_state), ProcessingMethod::accumulator_noise_suppression);
        auto& last = state_manager_->back();
        last.weight = weight;
        last.offset = offset;
        last.hide_shadow = hide_shadow_choice;
        last.shadow_threshold = shadow_sigma_thresh;

        // TODO: Shouldn't this be handled by a signal?
        UpdateGlobalFrameVector();
    }

    CloseProgressArea();
}


void SirveApp::ExecuteAutoTracking()
{
    video_player_->StopTimer();
    QPoint appPos = this->GetWindowPosition();
    ProcessingState& current_processing_state = state_manager_->GetCurrentState();
    ProcessingState* base_processing_state = &state_manager_->front();

    for (auto& test_state : *state_manager_)
    {
        if (test_state.method == ProcessingMethod::replace_bad_pixels)
        {
            base_processing_state = &test_state;
            break;
        }
    }

    AutoTracking AutoTracker(abp_file_type);

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

    double clamp_low_coeff = txt_lift_sigma->text().toDouble();
    double clamp_high_coeff = txt_gain_sigma->text().toDouble();
    int threshold = cmb_autotrack_threshold->itemData(cmb_autotrack_threshold->currentIndex(),Qt::UserRole).toInt();
    int bbox_buffer_pixels = txt_pixel_buffer->text().toInt();
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
            video_player_->EnterTrackCreationMode(appPos,existing_track_details, threshold, bbox_buffer_pixels, clamp_low_coeff, clamp_high_coeff, trackFeature, prefilter);
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
        std::vector<std::optional<TrackDetails>>track_details = track_info->GetEmptyTrack();
        auto frame_headers = abir_frames->ir_data;
        appPos = this->GetWindowPosition();
        arma::s32_mat autotrack = AutoTracker.SingleTracker(screenResolution, appPos, track_id, clamp_low_coeff, clamp_high_coeff, threshold, bbox_buffer_pixels, prefilter, trackFeature, start_frame, start_frame_i, stop_frame_i, current_processing_state, base_processing_state->details, video_player_->GetFrameHeaders(), calibration_model);

        auto currentState = state_manager_->GetCurrentState();
        if (!autotrack.empty() && currentState.offsets.size()>0){

            arma::vec framei = arma::regspace(start_frame_i,start_frame_i + autotrack.n_rows - 1);
            arma::mat offset_matrix2(framei.n_elem,3,arma::fill::zeros);
            std::vector<std::vector<int>> offsets = currentState.offsets;
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
            offset_matrix2.insert_cols(0,4);
            offset_matrix2.insert_cols(offset_matrix2.n_cols,12);
            arma::mat autotrack_d = arma::conv_to<arma::mat>::from(autotrack);
            autotrack_d += offset_matrix2;
            autotrack = arma::conv_to<arma::s32_mat>::from(autotrack_d);
        }

        if (!autotrack.empty()){

            TrackDetails details;
            for (int rowii = 0; rowii<autotrack.n_rows; rowii++)
            {
                details.centroid_x_boresight = autotrack(rowii,2);
                details.centroid_y_boresight = autotrack(rowii,3);
                details.centroid_x = autotrack(rowii,4);
                details.centroid_y = autotrack(rowii,5);
                details.number_pixels = autotrack(rowii,6);
                details.peak_counts = autotrack(rowii,7);
                details.mean_counts = autotrack(rowii,8);
                details.sum_counts = autotrack(rowii,9);
                details.sum_relative_counts =  autotrack(rowii,10);
                details.peak_irradiance = autotrack(rowii,11);
                details.mean_irradiance = autotrack(rowii,12);
                details.sum_irradiance = autotrack(rowii,13);
                details.bbox_x = autotrack(rowii,14);
                details.bbox_y = autotrack(rowii,15);
                details.bbox_width = autotrack(rowii,16);
                details.bbox_height = autotrack(rowii,17);
                track_details[autotrack(rowii,1)-1] = details;
            }

            tm_widget->AddTrackControl(track_id);
            video_player_->AddManualTrackIdToShowLater(track_id);
            track_info->AddCreatedManualTrack(eng_data->get_plotting_frame_data(),track_id, track_details, new_track_file_name);

            int index0 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmin();
            int index1 = plot_palette->GetEngineeringPlotReference(0)->get_index_sub_plot_xmax() + 1;
            video_player_->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
            for (int i = 0; i < plot_palette->tabBar()->count(); i++)
            {
                plot_palette->UpdateManualPlottingTrackFrames(i, track_info->get_manual_plotting_frames(), track_info->get_manual_track_ids());
            }
            FramePlotSpace();

            cmb_manual_track_IDs->clear();
            cmb_manual_track_IDs->addItem("Primary");
            std::set<int> track_ids = track_info->get_manual_track_ids();
            for ( int tid : track_ids )
            {
                cmb_manual_track_IDs->addItem(QString::number(tid));
            }

            QStringList color_options = ColorScheme::get_track_colors();
            QWidget * existing_track_control = tm_widget->findChild<QWidget*>(QString("TrackControl_%1").arg(track_id));
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

void SirveApp::UpdateEpochString(const QString& new_epoch_string)
{
    QString out = "Applied Epoch: ";
    out = out +new_epoch_string;
    lbl_current_epoch->setText(out);

}

void SirveApp::DisplayOriginalEpoch(const QString& new_epoch_string)
{
    QString out = "Original Epoch: ";
    out = out + new_epoch_string;
    lbl_current_epoch->setToolTip(out);
}

QString SirveApp::CreateEpochString(const std::vector<double>& new_epoch) {

    QString out = "";

    int length = static_cast<int>(new_epoch.size());
    for (int i = 0; i < length; i++)
    {
        if (i == 0)
        {
            out = out + QString::number(new_epoch[i]);
        }
        else {
            int number = new_epoch[i];
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

bool SirveApp::VerifyFrameSelection(int min_frame, int max_frame) const
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

    return true;
}

std::vector<unsigned int> SirveApp::GetUniqueIntegerVector(std::vector<unsigned int> A)
{
    std::vector<unsigned int> uniqueVals;
    uniqueVals.insert( uniqueVals.end(),A.begin(), A.end() );
    std::sort(uniqueVals.begin(), uniqueVals.end());
    // Assume this works
    uniqueVals.erase(std::unique(uniqueVals.begin(), uniqueVals.end()), uniqueVals.end());
    return uniqueVals;
}

void SirveApp::DeleteState()
{
    int current_state_idx0 = cmb_processing_states->currentIndex();
    ProcessingState original = GetStateManager()[current_state_idx0];
    std::vector<unsigned int> descendants = original.descendants;

    std::vector<size_t> delete_states_i;
    int current_state_id0 = original.state_ID;
    bool ischanged = false;

    if (descendants.size()>0){
        auto response = QtHelpers::LaunchYesNoMessageBox("Deletion Confirmation", "Deleting this state will delete all derived states. Are you sure you want to continue?");
        if (response == QMessageBox::Yes){
            delete_states_i.push_back(current_state_idx0);
            for (int i = 0; i < state_manager_->size(); i++) {
                std::vector<unsigned int> ancestors = GetStateManager()[i].ancestors;
                auto it = std::find(ancestors.begin(), ancestors.end(), current_state_id0);
                if (it != ancestors.end()){
                    delete_states_i.push_back(i);
                }
            }
            std::sort(delete_states_i.begin(), delete_states_i.end(), std::greater<size_t>());
            for (auto i = 0; i <  delete_states_i.size() ;i++){
                cmb_processing_states->removeItem(delete_states_i[i]);
                state_manager_->erase(delete_states_i[i]);
            }
            ischanged = true;
        }
    }
    else {
        int delete_idx = cmb_processing_states->currentIndex();
        cmb_processing_states->removeItem(delete_idx);
        state_manager_->erase(delete_idx);
        ischanged = true;
    }

    if (ischanged){
        std::map<int,int> id_map;
        for (auto i = 0; i <  state_manager_->size(); i++){
            id_map[GetStateManager()[i].state_ID] = i;
        }
        QList<QString> tmp_state_desc;
        for (auto i = 0; i < cmb_processing_states->count() ;i++){
            tmp_state_desc.append(cmb_processing_states->itemText(i));
        }
        QList<QString> new_labels;
        uint32_t i = 0;
        for (auto& state : *state_manager_) {
            for (auto& ancestor : state.ancestors) {
                ancestor = id_map[ancestor];
            }
            for (auto& descendant : state.descendants) {
                descendant = id_map[descendant];
            }
            QString desc = state.state_description;
            desc.replace(QString::number(state.state_ID) + ":",QString::number(id_map[state.state_ID]) + ":");

            QString desc2 = desc;
            QString tmp0 = "<Previous State " + QString::number(state.source_state_ID) + ">";
            desc2.replace(tmp0,"<Previous State " + QString::number(id_map[state.source_state_ID]) + ">");

            std::string result;
            for (auto num : state.ancestors) {
                result += std::to_string(num) + " -> ";
            }
            result += std::to_string(i++);
            QString state_steps = QString::fromStdString(result);

            QString desc2b = desc2;
            int index = desc2b.lastIndexOf("State steps:");
            if (index !=-1){
                int nchars = desc2b.length() - (index+13);
                desc2b.replace(index + 13,nchars,state_steps);
            }
            QString desc3 = tmp_state_desc[id_map[state.state_ID]];
            desc3.replace(QString::number(state.state_ID) + ":",QString::number(id_map[state.state_ID]) + ":");

            QString desc4 = desc3;
            desc4.replace(tmp0,"<Previous State " + QString::number(id_map[state.source_state_ID]) + ">");
            new_labels.append(desc4);

            state.state_ID = i;
            state.source_state_ID = id_map[state.source_state_ID];
            state.state_description = desc2b;
            state.state_steps = state_steps;
        }

        cmb_processing_states->clear();
        for (const auto& label : new_labels) {
            cmb_processing_states->addItem(label);
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
        video_player_->ViewFrame(new_frame_number-txt_start_frame->text().toInt());
        UpdateGlobalFrameVector();
    }
}

void SirveApp::UpdateGlobalFrameVector()
{
    if (!state_manager_->empty())
    {
        const auto& current = state_manager_->GetCurrentState();

        std::vector<double> original_frame_vector = {
            current.details.frames_16bit[video_player_->GetCounter()].begin(),
            current.details.frames_16bit[video_player_->GetCounter()].end()
        };

        //Convert current frame to armadillo matrix
        arma::vec image_vector(original_frame_vector);

        int image_max_value = image_vector.max();
        if (!rad_scale_by_frame->isChecked()){
            image_max_value = current.details.max_value;
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
        arma::mat offsets_matrix(current.details.frames_16bit.size(),3,arma::fill::zeros);
        if (!current.offsets_matrix.empty()){
            offsets_matrix = current.offsets_matrix;
        }

        video_player_->UpdateFrameVector(original_frame_vector, display_ready_converted_values, offsets_matrix);
    }
}

void SirveApp::closeEvent(QCloseEvent *event) {
    if (annotation_dialog) {
        // Simulate pressing the OK button
        annotation_dialog->accept();  // This will trigger the connected slot for OK
    }
    cv::destroyAllWindows();

    if (video_player_) {
        video_player_->Close();
    }

    event->accept();  // Proceed with closing the main window
}

void SirveApp::GetAboutTimeStamp()
{
    if (HMODULE hModule = GetModuleHandle(nullptr)) {
            PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
            PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pDosHeader + pDosHeader->e_lfanew);
            DWORD timestamp = pNtHeaders->FileHeader.TimeDateStamp;
            std::cout << "Timestamp: " << timestamp << std::endl;
            constexpr size_t kTimeStampSize = 26;
            char buf[kTimeStampSize];
            ctime_s(buf, kTimeStampSize, (time_t*)&timestamp);
            std::cout << "Build date: " << buf;
        }
}

double SirveApp::GetAvailableMemoryRatio(int num_frames, ABPFileType file_type)
{
    int nRows = 480;
    int nCols = 640;
    if (file_type == ABPFileType::ABP_D)
    {
        nRows = 720;
        nCols = 1280;
    }
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG availPhysMem = memInfo.ullAvailPhys;
    return static_cast<double>(availPhysMem) /
        (num_frames * 16 * nCols * nRows);
}
