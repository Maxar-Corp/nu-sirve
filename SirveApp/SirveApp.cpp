#include "SirveApp.h"

SirveApp::SirveApp(QWidget *parent)
	: QMainWindow(parent)
{
    config_values = configReaderWriter::ExtractWorkspaceConfigValues();

    workspace = new Workspace(config_values.workspace_folder);

	// establish object that will hold video and connect it to the playback thread
	color_map_display = new ColorMapDisplay(video_colors.maps[0].colors);
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

	// establish copy copy
	clipboard = QApplication::clipboard();
    connect(btn_copy_directory, &QPushButton::clicked, this, &SirveApp::CopyOsmDirectory);

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

	QHBoxLayout* main_layout = new QHBoxLayout();

	// Define main widgets in UI
	tab_menu = new QTabWidget();
	frame_video_player = new QFrame();
	tab_plots = new QTabWidget();

	//tab_menu->setMinimumWidth(200);
	tab_menu->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

	// ------------------------------------------------------------------------
	// Define complete tab widget
	// ------------------------------------------------------------------------

	// Add all to tab widget
    tab_menu->addTab(SetupFileImportTab(), "Import");
	tab_menu->addTab(SetupFilterTab(), "Processing");
    tab_menu->addTab(SetupColorCorrectionTab(), "Color/Overlays");
    tab_menu->addTab(SetupWorkspaceTab(), "Workspace");

	QSizePolicy fixed_width;
	fixed_width.setHorizontalPolicy(QSizePolicy::Minimum);
	fixed_width.setVerticalPolicy(QSizePolicy::Preferred);
	tab_menu->setSizePolicy(fixed_width);

    SetupVideoFrame();
    SetupPlotFrame();

	// set size policy for video controls
	QSizePolicy fixed_width_video;
	fixed_width_video.setHorizontalPolicy(QSizePolicy::Fixed);
	fixed_width_video.setVerticalPolicy(QSizePolicy::Minimum);
	frame_video_player->setSizePolicy(fixed_width_video);

	// ------------------------------------------------------------------------
	// Adds all elements to main UI

	main_layout->addWidget(tab_menu);
	main_layout->addWidget(frame_video_player);
	main_layout->addWidget(tab_plots);

	QFrame* frame_main = new QFrame();
	frame_main->setLayout(main_layout);

	// ------------------------------------------------------------------------
	// initialize ui elements

	tab_menu->setCurrentIndex(0);
	tab_menu->setTabEnabled(1, false);
	tab_menu->setTabEnabled(2, false);

	btn_load_osm->setEnabled(true);
	btn_copy_directory->setEnabled(true);

	txt_start_frame->setEnabled(false);
	txt_end_frame->setEnabled(false);
	btn_get_frames->setEnabled(false);
	btn_calibration_dialog->setEnabled(false);

	dt_epoch->setEnabled(false);
	btn_apply_epoch->setEnabled(false);

	rad_decimal->setChecked(true);
	rad_linear->setChecked(true);

	grpbox_auto_lift_gain->setEnabled(false);

	btn_workspace_save->setEnabled(false);
	btn_workspace_load->setEnabled(true);
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

QWidget* SirveApp::SetupFileImportTab() {

	QWidget* widget_tab_import = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_import = new QVBoxLayout(widget_tab_import);

	// ------------------------------------------------------------------------

	lbl_file_name = new QLabel("File Name:");
	btn_load_osm = new QPushButton("Load Image File");

	btn_copy_directory = new QPushButton("Copy File Path");

	btn_calibration_dialog = new QPushButton("Setup Calibration");

	//btn_load_osm->setMinimumWidth(30);
	//btn_copy_directory->setMinimumWidth(30);

	QGridLayout* grid_import_file = new QGridLayout();
	grid_import_file->addWidget(lbl_file_name, 0, 0, 1, 2);
	grid_import_file->addWidget(btn_load_osm, 1, 0);
	grid_import_file->addWidget(btn_copy_directory, 1, 1);
	grid_import_file->addWidget(btn_calibration_dialog, 2, 0, 1, 2);
	grid_import_file->addWidget(QtHelpers::HorizontalLine(), 3, 0, 1, 2);

	vlayout_tab_import->addLayout(grid_import_file);

	// ------------------------------------------------------------------------

	QLabel* label_start_frame = new QLabel("Start Frame");
	QLabel* label_stop_frame = new QLabel("Stop Frame");
	lbl_max_frames = new QLabel("Max Frames: ");

	txt_start_frame = new QLineEdit("0");
	txt_start_frame->setAlignment(Qt::AlignHCenter);
	txt_end_frame = new QLineEdit();
	txt_end_frame->setAlignment(Qt::AlignHCenter);
	btn_get_frames = new QPushButton("Load Frames");

	QGridLayout* grid_tab_import_frames = new QGridLayout();
	grid_tab_import_frames->addWidget(label_start_frame, 0, 0);
	grid_tab_import_frames->addWidget(label_stop_frame, 0, 1);
	grid_tab_import_frames->addWidget(txt_start_frame, 1, 0);
	grid_tab_import_frames->addWidget(txt_end_frame, 1, 1);
	grid_tab_import_frames->addWidget(lbl_max_frames, 2, 0, 1, 2);
	grid_tab_import_frames->addWidget(btn_get_frames, 3, 0, 1, 2);
	grid_tab_import_frames->addWidget(QtHelpers::HorizontalLine(), 4, 0, 1, 2);

	grid_tab_import_frames->setColumnMinimumWidth(0, 30);
	grid_tab_import_frames->setColumnMinimumWidth(1, 30);

	vlayout_tab_import->addLayout(grid_tab_import_frames);

	// ------------------------------------------------------------------------

	QLabel* label_epoch = new QLabel("Epoch");
	QLabel* label_date_format = new QLabel("Format is:    YYYY/MM/DD HH:MM:SS");

	dt_epoch = new QDateTimeEdit(QDateTime(QDate(2001, 01, 01), QTime(0, 0, 0, 0)));
	dt_epoch->setDisplayFormat("yyyy/MM/dd hh:mm:ss.zzz");
	dt_epoch->setAlignment(Qt::AlignHCenter);

	lbl_current_epoch = new QLabel("Applied Epoch: ");
	btn_apply_epoch = new QPushButton("Apply Epoch");
	//btn_apply_epoch->setMinimumWidth(30);

	QVBoxLayout* vlayout_tab_import_epoch = new QVBoxLayout();
	vlayout_tab_import_epoch->addWidget(label_epoch);
	vlayout_tab_import_epoch->addWidget(dt_epoch);
	vlayout_tab_import_epoch->addWidget(label_date_format);
	vlayout_tab_import_epoch->addWidget(lbl_current_epoch);
	vlayout_tab_import_epoch->addWidget(btn_apply_epoch);
	vlayout_tab_import_epoch->addWidget(QtHelpers::HorizontalLine());

	vlayout_tab_import->addLayout(vlayout_tab_import_epoch);

	// ------------------------------------------------------------------------

	vlayout_tab_import->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout

	return widget_tab_import;
}

QWidget* SirveApp::SetupColorCorrectionTab()
{
	QWidget* widget_tab_color = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_color = new QVBoxLayout(widget_tab_color);

	QLabel* label_lift = new QLabel("Dark \nSet Point");
	label_lift->setToolTip("Dark Set Point pushes the image darker");
	QLabel* label_gain = new QLabel("Light \nSet Point");
	label_gain->setToolTip("Light Set Point pushes the image lighter");
	lbl_lift_value = new QLabel("0.0");
	lbl_gain_value = new QLabel("1.0");

	slider_lift = new QSlider();
	slider_lift->setOrientation(Qt::Horizontal);
	slider_lift->setMinimum(0);
	slider_lift->setMaximum(1000);
	slider_lift->setSingleStep(1);
	slider_lift->setPageStep(10);
	slider_lift->setValue(0);
	slider_lift->setTickPosition(QSlider::TicksAbove);
	slider_lift->setTickInterval(100);

	slider_gain = new QSlider();
	slider_gain->setOrientation(Qt::Horizontal);
	slider_gain->setMinimum(0);
	slider_gain->setMaximum(1000);
	slider_gain->setSingleStep(1);
	slider_gain->setPageStep(10);
	slider_gain->setValue(1000);
	slider_gain->setTickPosition(QSlider::TicksAbove);
	slider_gain->setTickInterval(100);

	QGridLayout* grid_tab_color_sliders = new QGridLayout(widget_tab_color);
	grid_tab_color_sliders->addWidget(label_lift, 0, 0);
	grid_tab_color_sliders->addWidget(slider_lift, 0, 1);
	grid_tab_color_sliders->addWidget(lbl_lift_value, 0, 2);
	grid_tab_color_sliders->addWidget(label_gain, 1, 0);
	grid_tab_color_sliders->addWidget(slider_gain, 1, 1);
	grid_tab_color_sliders->addWidget(lbl_gain_value, 1, 2);

	QHBoxLayout* hlayout_additional_color_settings = new QHBoxLayout();
	chk_auto_lift_gain = new QCheckBox("Enable Auto Lift/Gain", widget_tab_color);
	hlayout_additional_color_settings->addWidget(chk_auto_lift_gain);
	btn_reset_color_correction = new QPushButton("Reset Set Points", widget_tab_color);
	hlayout_additional_color_settings->addWidget(btn_reset_color_correction);

	grpbox_auto_lift_gain = new QGroupBox("Auto Lift/Gain Options");
	QDoubleValidator* ensure_double = new QDoubleValidator(widget_tab_color);
	QLabel* lbl_auto_lift = new QLabel("Lift adjustment\n(sigma below mean)", grpbox_auto_lift_gain);
	txt_lift_sigma = new QLineEdit("3", grpbox_auto_lift_gain);
	txt_lift_sigma->setValidator(ensure_double);
	QLabel* lbl_auto_gain = new QLabel("Gain adjustment\n(sigma above mean)", grpbox_auto_lift_gain);
	txt_gain_sigma = new QLineEdit("3", grpbox_auto_lift_gain);
	txt_gain_sigma->setValidator(ensure_double);

	QGridLayout* grid_grpbox_auto_lift_gain = new QGridLayout(grpbox_auto_lift_gain);
	grid_grpbox_auto_lift_gain->addWidget(lbl_auto_lift, 0, 0);
	grid_grpbox_auto_lift_gain->addWidget(txt_lift_sigma, 0, 1);
	grid_grpbox_auto_lift_gain->addWidget(lbl_auto_gain, 1, 0);
	grid_grpbox_auto_lift_gain->addWidget(txt_gain_sigma, 1, 1);

	chk_relative_histogram = new QCheckBox("Relative Histogram");

	vlayout_tab_color->addLayout(grid_tab_color_sliders);
	vlayout_tab_color->addLayout(hlayout_additional_color_settings);
	vlayout_tab_color->addWidget(grpbox_auto_lift_gain);
	vlayout_tab_color->addWidget(chk_relative_histogram);
	vlayout_tab_color->addWidget(QtHelpers::HorizontalLine());

	 // --------------------------------------------------------------------------

	QHBoxLayout* hlayout_osm_tracks = new QHBoxLayout(widget_tab_color);
	QHBoxLayout* hlayout_text_color = new QHBoxLayout(widget_tab_color);
	QHBoxLayout* hlayout_color_map = new QHBoxLayout(widget_tab_color);

	QLabel* lbl_colormap = new QLabel("Set Colormap:");
	QLabel* lbl_text_color = new QLabel("Set Text Color:");

	chk_show_tracks = new QCheckBox("Show OSM Tracks");
	chk_sensor_track_data = new QCheckBox("Show Sensor Info");
	chk_show_time = new QCheckBox("Show Zulu Time");

	btn_change_banner_text = new QPushButton("Change Banner Text");
	// btn_change_banner_text->setStyleSheet("color: black;"
	// 							"font-weight: bold;"
    //                     		);
	btn_add_annotations = new QPushButton("Add/Edit Annotations");
	cmb_color_maps = new QComboBox();
	int number_maps = video_colors.maps.size();
	for (int i = 0; i < number_maps; i++)
		cmb_color_maps->addItem(video_colors.maps[i].name);

	QStringList colors = ColorScheme::get_track_colors();

	cmb_tracker_color = new QComboBox();
	cmb_text_color = new QComboBox();

	cmb_tracker_color->addItems(colors);
	cmb_text_color->addItems(colors);

	cmb_tracker_color->setEnabled(false);

	// --------------------------------------------------------------------------
	hlayout_osm_tracks->addWidget(chk_show_tracks);
	hlayout_osm_tracks->addStretch();
	hlayout_osm_tracks->addWidget(cmb_tracker_color);

	hlayout_text_color->addWidget(lbl_text_color);
	hlayout_text_color->addStretch();
	hlayout_text_color->addWidget(cmb_text_color);

	hlayout_color_map->addWidget(lbl_colormap);
	hlayout_color_map->addStretch();
	hlayout_color_map->addWidget(cmb_color_maps);

	vlayout_tab_color->addLayout(hlayout_osm_tracks);
	vlayout_tab_color->addWidget(QtHelpers::HorizontalLine());
	vlayout_tab_color->addWidget(chk_sensor_track_data);
	vlayout_tab_color->addWidget(chk_show_time);

	// ------------------------------------------------------------------------

	vlayout_tab_color->addLayout(hlayout_text_color);
	vlayout_tab_color->addWidget(btn_change_banner_text);
	vlayout_tab_color->addWidget(QtHelpers::HorizontalLine());
	vlayout_tab_color->addLayout(hlayout_color_map);
	color_map_display->setMinimumHeight(20);
	vlayout_tab_color->addWidget(color_map_display);
	vlayout_tab_color->addWidget(QtHelpers::HorizontalLine());
	vlayout_tab_color->addWidget(btn_add_annotations);

	vlayout_tab_color->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout

	return widget_tab_color;
}

QWidget* SirveApp::SetupFilterTab() {

	QWidget* widget_tab_processing = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_processing = new QVBoxLayout(widget_tab_processing);
	QStringList colors = ColorScheme::get_track_colors();
	// ------------------------------------------------------------------------
	QGridLayout* grid_bad_pixels = new QGridLayout();
	grid_bad_pixels->addWidget(QtHelpers::HorizontalLine(), 0, 0, 1, 3);
	QLabel* label_bad_pixel = new QLabel("Bad Pixels ");
	label_bad_pixel->setStyleSheet(bold_large_styleSheet);
	grid_bad_pixels->addWidget(label_bad_pixel, 1, 0, 1, 1);
	lbl_bad_pixel_count = new QLabel("");
	grid_bad_pixels->addWidget(lbl_bad_pixel_count, 1, 1, 1, 2);

	lbl_bad_pixel_type = new QLabel("Replace Pixels ");
	grid_bad_pixels->addWidget(lbl_bad_pixel_type, 2, 0, 1, 1);
	cmb_bad_pixels_type = new QComboBox();
	cmb_bad_pixels_type->addItem("All Bad Pixels");
	cmb_bad_pixels_type->addItem("Dead/Bad Scale Only");
	cmb_bad_pixels_type->addItem("Outlier Only");
	grid_bad_pixels->addWidget(cmb_bad_pixels_type, 3, 0, 1, 1);

	lbl_bad_pixel_method = new QLabel("Method:");
	grid_bad_pixels->addWidget(lbl_bad_pixel_method, 2, 1, 1, 1);
	cmb_outlier_processing_type = new QComboBox();
	cmb_outlier_processing_type->addItem("Median");
	cmb_outlier_processing_type->addItem("Moving Median");
	grid_bad_pixels->addWidget(cmb_outlier_processing_type, 3, 1, 1, 1);
	connect(cmb_outlier_processing_type, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &SirveApp::handle_outlier_processing_change);

	lbl_bad_pixel_start_frame = new QLabel("Start:");
	grid_bad_pixels->addWidget(lbl_bad_pixel_start_frame, 4, 0, 1, 1);
	txt_bad_pixel_start_frame = new QLineEdit("1");
	grid_bad_pixels->addWidget(txt_bad_pixel_start_frame, 5, 0, 1, 1);
	lbl_bad_pixel_stop_frame = new QLabel("Stop:");
	grid_bad_pixels->addWidget(lbl_bad_pixel_stop_frame, 4, 1, 1, 1);
	txt_bad_pixel_end_frame = new QLineEdit("500");
	grid_bad_pixels->addWidget(txt_bad_pixel_end_frame, 5, 1, 1, 1);

	lbl_moving_median_window_length = new QLabel("Window Length:");
	grid_bad_pixels->addWidget(lbl_moving_median_window_length, 6, 0, 1, 1);
	txt_moving_median_N = new QLineEdit("30");
	grid_bad_pixels->addWidget(txt_moving_median_N, 7, 0, 1, 1);
	txt_moving_median_N->setEnabled(false);
	
	lbl_bad_pixel_sensitivity = new QLabel("Sensitivity:");
	grid_bad_pixels->addWidget(lbl_bad_pixel_sensitivity, 6, 1, 1, 1);
	cmb_outlier_processing_sensitivity = new QComboBox();
	cmb_outlier_processing_sensitivity->addItem("Low 6 sigma");
	cmb_outlier_processing_sensitivity->addItem("Medium 5 sigma");
	cmb_outlier_processing_sensitivity->addItem("High 4 sigma");
	cmb_outlier_processing_sensitivity->addItem("Max 3 sigma");
	grid_bad_pixels->addWidget(cmb_outlier_processing_sensitivity, 7, 1, 1, 1);

	chk_highlight_bad_pixels = new QCheckBox("Highlight Bad Pixels");
	grid_bad_pixels->addWidget(chk_highlight_bad_pixels, 3, 2, 1, 1);

	lbl_bad_pixel_color = new QLabel("Color:");
	grid_bad_pixels->addWidget(lbl_bad_pixel_color, 4, 2, 1, 1);
	cmb_bad_pixel_color = new QComboBox();
	cmb_bad_pixel_color->addItems(colors);
	grid_bad_pixels->addWidget(cmb_bad_pixel_color,5, 2, 1,1);
	cmb_bad_pixel_color->setCurrentIndex(2);
	connect(cmb_bad_pixel_color, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &SirveApp::edit_bad_pixel_color);

	btn_bad_pixel_identification = new QPushButton("Replace Bad Pixels");
    connect(btn_bad_pixel_identification, &QPushButton::clicked, this, &SirveApp::HandleBadPixelReplacement);
	grid_bad_pixels->addWidget(btn_bad_pixel_identification, 7, 2, 1, 1);

	grid_bad_pixels->addWidget(QtHelpers::HorizontalLine(), 8, 0, 1, 3);

	vlayout_tab_processing->addLayout(grid_bad_pixels);

	// ------------------------------------------------------------------------
	QGridLayout* grid_tab_processing_fns = new QGridLayout();

	QLabel* label_FNS = new QLabel("Fixed Noise Suppression ");
	label_FNS->setStyleSheet(bold_large_styleSheet);
	grid_tab_processing_fns->addWidget(label_FNS, 0, 0, 1, 2);
	lbl_fixed_suppression = new QLabel("No Frames Selected");
	grid_tab_processing_fns->addWidget(lbl_fixed_suppression,1, 0, 1, 4);

	chk_FNS_external_file = new QCheckBox("External File");
	grid_tab_processing_fns->addWidget(chk_FNS_external_file, 3, 0, 1, 1);

	lbl_FNS_start_frame = new QLabel("Start:");
	grid_tab_processing_fns->addWidget(lbl_FNS_start_frame, 2, 1, 1, 1);
	txt_FNS_start_frame = new QLineEdit("1");
	grid_tab_processing_fns->addWidget(txt_FNS_start_frame, 3, 1, 1, 1);
	lbl_FNS_stop_frame = new QLabel("Stop:");
	grid_tab_processing_fns->addWidget(lbl_FNS_stop_frame, 2, 2, 1, 1);
	txt_FNS_end_frame = new QLineEdit("50");
	grid_tab_processing_fns->addWidget(txt_FNS_end_frame, 3, 2, 1, 1);
	
	btn_FNS = new QPushButton("Fixed Noise Suppression");
	grid_tab_processing_fns->addWidget(btn_FNS,3,3,1,1);
	grid_tab_processing_fns->addWidget(QtHelpers::HorizontalLine(), 4, 0, 1, 4);
	vlayout_tab_processing->addLayout(grid_tab_processing_fns);

	// ------------------------------------------------------------------------
	QGridLayout* grid_tab_processing_bgs = new QGridLayout();
	QLabel* label_adaptive_noise_suppression = new QLabel("Adaptive Noise Suppression ");
	label_adaptive_noise_suppression->setStyleSheet(bold_large_styleSheet);
	grid_tab_processing_bgs->addWidget(label_adaptive_noise_suppression, 0, 0, 1, 4);
	lbl_adaptive_noise_suppression_status = new QLabel("No Frames Setup");
	grid_tab_processing_bgs->addWidget(lbl_adaptive_noise_suppression_status, 1, 0, 1, 4);
	
	lbl_ANS_offset_frames = new QLabel("Offset:");
	grid_tab_processing_bgs->addWidget(lbl_ANS_offset_frames, 2, 0, 1, 1);
	txt_ANS_offset_frames = new QLineEdit("-30");
	grid_tab_processing_bgs->addWidget(txt_ANS_offset_frames, 3, 0, 1, 1);
	lbl_ANS_number_frames = new QLabel("Number:");
	grid_tab_processing_bgs->addWidget(lbl_ANS_number_frames, 2, 1, 1, 1);
	txt_ANS_number_frames = new QLineEdit("5");
	grid_tab_processing_bgs->addWidget(txt_ANS_number_frames, 3, 1, 1, 1);
	chk_hide_shadow = new QCheckBox("Hide Shadow");
	chk_hide_shadow->setChecked(true);
	grid_tab_processing_bgs->addWidget(chk_hide_shadow, 4, 0, 1, 1);
	lbl_ANS_shadow_threshold = new QLabel("Shadow Threshold:");
	grid_tab_processing_bgs->addWidget(lbl_ANS_shadow_threshold, 2, 2, 1, 1);
	cmb_shadow_threshold = new QComboBox();
	cmb_shadow_threshold->addItem("1 sigma");
	cmb_shadow_threshold->addItem("2 sigma");
	cmb_shadow_threshold->addItem("3 sigma");
	grid_tab_processing_bgs->addWidget(cmb_shadow_threshold, 3, 2, 1, 1);
	btn_ANS = new QPushButton("Adaptive Noise Suppression");
	grid_tab_processing_bgs->addWidget(btn_ANS, 4, 2, 1, 2);

	grid_tab_processing_bgs->addWidget(QtHelpers::HorizontalLine(), 5, 0, 1, 5);

	vlayout_tab_processing->addLayout(grid_tab_processing_bgs);

	// ------------------------------------------------------------------------
	QGridLayout* grid_tab_processing_deinterlace = new QGridLayout();
	QLabel* label_deinterlace = new QLabel("Deinterlace ");
	label_deinterlace->setStyleSheet(bold_large_styleSheet);
	grid_tab_processing_deinterlace->addWidget(label_deinterlace, 0, 0, 1,1);
	// cmb_deinterlace_options = new QComboBox();
	// cmb_deinterlace_options->addItem("Max Absolute Value");
	// cmb_deinterlace_options->addItem("Centroid");
	// cmb_deinterlace_options->addItem("Avg Cross Correlation");
	// grid_tab_processing_deinterlace->addWidget(cmb_deinterlace_options, 1, 0, 1, 2);
	btn_deinterlace = new QPushButton("Deinterlace");
	grid_tab_processing_deinterlace->addWidget(btn_deinterlace, 0,2,1,1);
	grid_tab_processing_deinterlace->addWidget(QtHelpers::HorizontalLine(), 2, 0, 1, 3);

	vlayout_tab_processing->addLayout(grid_tab_processing_deinterlace);
	// ------------------------------------------------------------------------
	QGridLayout* grid_tab_processing_extra = new QGridLayout();
	cmb_processing_states = new QComboBox();
	btn_undo_step = new QPushButton("Undo One Step");
	grid_tab_processing_extra->addWidget(cmb_processing_states, 0, 0, 1, 4);
	grid_tab_processing_extra->addWidget(btn_undo_step, 1, 3, 1, 1);
	// grid_tab_processing_extra->addWidget(QtHelpers::HorizontalLine(), 1, 0, 1, 4);
	// lbl_progress_status = new QLabel("Status:");
	// grid_tab_processing_extra->addWidget(btn_undo_step, 2, 0, 1,3);
	// progress_bar = new QProgressBar();
	// grid_tab_processing_extra->addWidget(progress_bar,3,0,1,3);
	vlayout_tab_processing->addLayout(grid_tab_processing_extra);

	// ------------------------------------------------------------------------

	vlayout_tab_processing->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout

	return widget_tab_processing;
}

QWidget* SirveApp::SetupWorkspaceTab(){

	QWidget* widget_tab_workspace = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_workspace = new QVBoxLayout(widget_tab_workspace);

    lbl_current_workspace_folder = new QLabel("Current Workspace Folder: " + config_values.workspace_folder);
    lbl_current_workspace_folder->setWordWrap(true);

	cmb_workspace_name = new QComboBox();
    cmb_workspace_name->addItems(workspace->get_workspace_names(config_values.workspace_folder));

    btn_change_workspace_directory = new QPushButton("Change Workspace Directory");
    btn_change_workspace_directory->setStyleSheet("color: black;"
                                      "background-color: rgb(255,140,0);"
                                      "font-weight: bold;"
                                      );

	btn_workspace_load = new QPushButton("Load Workspace");
	btn_workspace_load->setStyleSheet("color: black;"
                        		"background-color: rgb(0,150,0);"
								"font-weight: bold;"
                        		);
	
	btn_workspace_save = new QPushButton("Save Workspace");

	// cmb_processing_states = new QComboBox();
	// btn_undo_step = new QPushButton("Undo One Step");
	// btn_undo_step->setStyleSheet(olive_green_button_styleSheet);

	QLabel *lbl_track = new QLabel("Manual Track Management");
	lbl_create_track_message = new QLabel("");
	// lbl_create_track_message->setStyleSheet("QLabel { color: red }");
	// QFont large_font;
	// large_font.setPointSize(16);
	// lbl_create_track_message->setFont(large_font);
	btn_create_track = new QPushButton("Create Track");
	// btn_create_track->setStyleSheet(track_button_styleSheet);
	btn_finish_create_track = new QPushButton("Finish");
	// btn_finish_create_track->setStyleSheet(dark_red_stop_styleSheet);
	btn_finish_create_track->setHidden(true);
	// btn_finish_create_track->setFont(large_font);
	btn_import_tracks = new QPushButton("Import Tracks");
	// btn_import_tracks->setStyleSheet(track_button_styleSheet);

	QGridLayout* grid_workspace = new QGridLayout();
    grid_workspace->addWidget(lbl_current_workspace_folder, 0, 0, 1, 1);
    grid_workspace->addWidget(cmb_workspace_name, 1, 0, 1, 1);
    grid_workspace->addWidget(btn_change_workspace_directory, 1, 1, 1, 1);
    grid_workspace->addWidget(btn_workspace_load, 2, 0, 1, 1);
    grid_workspace->addWidget(btn_workspace_save, 2, 1, 1, 1);
    grid_workspace->addWidget(QtHelpers::HorizontalLine(), 3, 0, 1, -1);
    // grid_workspace->addWidget(cmb_processing_states, 4, 0, 1, 1);
    // grid_workspace->addWidget(btn_undo_step, 4, 1, 1, 1);
    grid_workspace->addWidget(QtHelpers::HorizontalLine(), 5, 0, 1, -1);
    grid_workspace->addWidget(lbl_track, 6, 0, 1, -1, Qt::AlignCenter);
    grid_workspace->addWidget(lbl_create_track_message, 7, 0, 1, 1);
    grid_workspace->addWidget(btn_create_track, 7, 1, 1, 1);
    grid_workspace->addWidget(btn_finish_create_track, 7, 1, 1, 1);
    grid_workspace->addWidget(btn_import_tracks, 8, 0, 1, -1);

	tm_widget = new TrackManagementWidget(widget_tab_workspace);
	QScrollArea *track_management_scroll_area = new QScrollArea();
    track_management_scroll_area->setWidgetResizable( true );
	track_management_scroll_area->setWidget(tm_widget);
	track_management_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    grid_workspace->addWidget(track_management_scroll_area, 9, 0, 1, -1);

	vlayout_tab_workspace->addLayout(grid_workspace);
	vlayout_tab_workspace->insertStretch(-1, 0);
	return widget_tab_workspace;
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
	btn_play->setToolTip("Play Video");

	QPixmap pause_image("icons/pause.png");
	QIcon pause_icon(pause_image);
	btn_pause = new QPushButton();
	btn_pause->resize(button_video_width, button_video_height);
	btn_pause->setIcon(pause_icon);
	btn_pause->setToolTip("Pause Video");

	QPixmap reverse_image("icons/reverse.png");
	QIcon reverse_icon(reverse_image);
	btn_reverse = new QPushButton();
	btn_reverse->resize(button_video_width, button_video_height);
	btn_reverse->setIcon(reverse_icon);
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
	vlayout_tab_histogram->addWidget(btn_popout_histogram);
	
	vlayout_tab_histogram->addWidget(frame_histogram_abs);
	vlayout_tab_histogram->addWidget(frame_histogram_rel);

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

    connect(histogram_plot, &HistogramLinePlot::clickDragHistogram, this, &SirveApp::HandleHistogramClick);

    connect(video_display, &VideoDisplay::addNewBadPixels, this, &SirveApp::ReceiveNewBadPixels);
    connect(video_display, &VideoDisplay::removeBadPixels, this, &SirveApp::ReceiveNewGoodPixels);

	//---------------------------------------------------------------------------

    connect(tab_menu, &QTabWidget::currentChanged, this, &SirveApp::HandlePlotDisplayAutoChange);
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
    connect(video_display, &VideoDisplay::advanceFrame, playback_controller, &FramePlayer::GotoNextFrame);
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
    connect(btn_load_osm, &QPushButton::clicked, this, &SirveApp::HandleAbpFileSelected);
    connect(btn_calibration_dialog, &QPushButton::clicked, this, &SirveApp::ShowCalibrationDialog);
    connect(btn_get_frames, &QPushButton::clicked, this, &SirveApp::UiLoadAbirData);
    connect(txt_end_frame, &QLineEdit::returnPressed, this, &SirveApp::UiLoadAbirData);

    connect(chk_highlight_bad_pixels, &QPushButton::clicked, video_display, &VideoDisplay::HighlightBadPixels);

    connect(btn_FNS, &QPushButton::clicked, this, &SirveApp::ExecuteNonUniformityCorrectionSelectionOption);

    connect(btn_ANS, &QPushButton::clicked, this, &SirveApp::ExecuteNoiseSuppression);

    connect(btn_deinterlace, &QPushButton::clicked, this, &SirveApp::ExecuteDeinterlace);

	//---------------------------------------------------------------------------

    connect(btn_change_workspace_directory, &QPushButton::clicked, this, &SirveApp::ChangeWorkspaceDirectory);
    connect(btn_workspace_save, &QPushButton::clicked, this, &SirveApp::SaveWorkspace);
    connect(btn_workspace_load, &QPushButton::clicked, this, &SirveApp::LoadWorkspace);
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
	}

	track_info->AddManualTracks(result.frames);

	int index0 = data_plots->index_sub_plot_xmin;
	int index1 = data_plots->index_sub_plot_xmax + 1;
    video_display->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
	data_plots->UpdateManualPlottingTrackFrames(track_info->get_manual_plotting_tracks(), track_info->get_manual_track_ids());
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
	tab_menu->setTabEnabled(2, false);
	btn_workspace_load->setDisabled(true);
	btn_workspace_save->setDisabled(true);

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
		data_plots->UpdateManualPlottingTrackFrames(track_info->get_manual_plotting_tracks(), track_info->get_manual_track_ids());
        UpdatePlots();
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
	btn_workspace_load->setDisabled(false);
	btn_workspace_save->setDisabled(false);

    video_display->ExitTrackCreationMode();
}

void SirveApp::HandleTrackRemoval(int track_id)
{
    tm_widget->RemoveTrackControl(track_id);
	track_info->RemoveManualTrack(track_id);
	int index0 = data_plots->index_sub_plot_xmin;
	int index1 = data_plots->index_sub_plot_xmax + 1;
    video_display->UpdateManualTrackData(track_info->get_manual_frames(index0, index1));
    video_display->DeleteManualTrack(track_id);
	data_plots->UpdateManualPlottingTrackFrames(track_info->get_manual_plotting_tracks(), track_info->get_manual_track_ids());
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
		bool ok;
		QString current_workspace_name = cmb_workspace_name->currentText();
		QString workspace_name = QInputDialog::getText(0, "Workspace Name", "Choose workspace file name (reusing a file will overwrite the workspace)", QLineEdit::Normal, current_workspace_name, &ok);
		if (!ok)
			return;
		if (!workspace_name.endsWith(".json")){
			workspace_name.append(".json");
		}
		if (!workspace_name.endsWith(".json")) {
			QtHelpers::LaunchMessageBox(QString("Issue Saving Workspace"), "Please provide a file name ending with .json.");
			return;
		}
        workspace->SaveState(workspace_name, config_values.workspace_folder, abp_file_metadata.image_path, data_plots->index_sub_plot_xmin + 1, data_plots->index_sub_plot_xmax + 1, video_display->container.get_processing_states(), video_display->annotation_list);
		cmb_workspace_name->clear();
        cmb_workspace_name->addItems(workspace->get_workspace_names(config_values.workspace_folder));
		cmb_workspace_name->setCurrentText(workspace_name);
    }
}

void SirveApp::LoadWorkspace()
{
	QString current_workspace_name = cmb_workspace_name->currentText();
    WorkspaceValues workspace_vals = workspace->LoadState(current_workspace_name, config_values.workspace_folder);

	int compare = QString::compare(workspace_vals.image_path, "", Qt::CaseInsensitive);
	if (compare == 0) {
		QtHelpers::LaunchMessageBox(QString("Issue Loading Workspace"), "The workspace is empty.");
		return;
	}

    bool validated = ValidateAbpFiles(workspace_vals.image_path);
	if (validated) {
        LoadOsmData();
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
		QString ANS_hide_shadow_str = "Hide Shadow";
		if (!current_state.ANS_hide_shadow){
			ANS_hide_shadow_str = "Show Shadow";
		}

		switch (current_state.method)
		{
			case ProcessingMethod::adaptive_noise_suppression:
                ApplyAdaptiveNoiseCorrection(current_state.ANS_relative_start_frame, current_state.ANS_num_frames, ANS_hide_shadow_str);
				break;

			case ProcessingMethod::deinterlace:
                ApplyDeinterlacing(current_state.deint_type);
				break;

			case ProcessingMethod::fixed_noise_suppression:

                ApplyFixedNoiseSuppression(workspace_vals.image_path, current_state.FNS_file_path, current_state.FNS_start_frame, current_state.FNS_stop_frame);
				break;

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
		// txt_start_frame->setStyleSheet(orange_styleSheet);
		// txt_end_frame->setStyleSheet(orange_styleSheet);				
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
			btn_calibration_dialog->setEnabled(true);
			// txt_start_frame->setStyleSheet(orange_styleSheet);
			// txt_end_frame->setStyleSheet(orange_styleSheet);		
		}
		else{
			txt_start_frame->setEnabled(false);
			txt_end_frame->setEnabled(false);
			btn_get_frames->setEnabled(false);
			btn_calibration_dialog->setEnabled(false);	
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

	lbl_file_name->setText("File: " + abp_file_metadata.file_name);
	lbl_file_name->setToolTip(abp_file_metadata.directory_path);

	txt_start_frame->setEnabled(true);
	txt_end_frame->setEnabled(true);
	btn_get_frames->setEnabled(true);

	QString osm_max_frames = QString::number(osm_frames.size());
	txt_start_frame->setText(QString("1"));
	txt_end_frame->setText(osm_max_frames);

	QString max_frame_text("Max Frames: ");
	max_frame_text.append(osm_max_frames);
	lbl_max_frames->setText(max_frame_text);

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

		tab_menu->setTabEnabled(1, false);
		tab_menu->setTabEnabled(2, false);
		cmb_processing_states->setEnabled(false);
	}

	eng_data = new EngineeringData(osm_frames);
	track_info = new TrackInformation(osm_frames);
	data_plots = new EngineeringPlots(osm_frames);

	size_t num_tracks = track_info->get_count_of_tracks();
	if (num_tracks == 0)
	{
		QtHelpers::LaunchMessageBox(QString("No Tracking Data"), "No tracking data was found within the file. No data will be plotted.");
	}

	data_plots->past_midnight = eng_data->get_seconds_from_midnight();
	data_plots->past_epoch = eng_data->get_seconds_from_epoch();

	data_plots->set_plotting_track_frames(track_info->get_plotting_tracks(), track_info->get_count_of_tracks());

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
	chk_show_tracks->setChecked(false);
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
	QProgressDialog progress_dialog("Loading frames", "", 0, 4);
	progress_dialog.setWindowModality(Qt::ApplicationModal);
	progress_dialog.setWindowTitle("Overall ABIR Load Progress");
	progress_dialog.setCancelButton(nullptr);
	progress_dialog.setMinimumDuration(0);
	progress_dialog.setValue(1);

	// Load the ABIR data
	playback_controller->StopTimer();

	ABIRDataResult abir_data_result = file_processor.LoadImageFile(abp_file_metadata.image_path, min_frame, max_frame, config_values.version);

	progress_dialog.setLabelText("Configuring application");
	progress_dialog.setValue(2);

	if (abir_data_result.had_error) {
		QtHelpers::LaunchMessageBox(QString("Error Reading ABIR Frames"), "Error reading .abpimage file. See log for more details.");
		btn_get_frames->setEnabled(true);
		return;
	}

	std::vector<std::vector<uint16_t>> video_frames = abir_data_result.video_frames_16bit;
	unsigned int number_frames = static_cast<unsigned int>(video_frames.size());

	int x_pixels = abir_data_result.x_pixels;
	int y_pixels = abir_data_result.y_pixels;

	VideoDetails vid_details = {x_pixels, y_pixels, video_frames};

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

	progress_dialog.setLabelText("Finalizing application state");
	progress_dialog.setValue(3);

    video_display->InitializeTrackData(track_info->get_osm_frames(index0, index1), track_info->get_manual_frames(index0, index1));
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

	playback_controller->set_initial_speed_index(10);
    UpdateFps();

	tab_plots->setCurrentIndex(1);

	btn_get_frames->setEnabled(true);
	btn_calibration_dialog->setEnabled(true);

	tab_menu->setTabEnabled(1, true);
	tab_menu->setTabEnabled(2, true);

	lbl_bad_pixel_count->setText("");

	cmb_processing_states->setEnabled(true);
	btn_workspace_save->setEnabled(true);

	btn_create_track->setEnabled(true);
	btn_import_tracks->setEnabled(true);

    ToggleVideoPlaybackOptions(true);

	progress_dialog.setValue(4);
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
	video_display->label->disable();
	video_display->label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	popout_video->acquire(video_display->label);
    connect(popout_video, &QDialog::finished, this, &SirveApp::HandlePopoutVideoClosed);
	popout_video->open();

}

void SirveApp::HandlePopoutVideoClosed()
{
	video_display->label->enable();
	video_display->label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	btn_popout_video->setChecked(false);
    video_display->ReclaimLabel();
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
	lbl_lift_value->setText(QString::number(lift));

	slider_gain->setValue(gain * 1000);
	lbl_gain_value->setText(QString::number(gain));
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
    lbl_lift_value->setText(QString::number(lift_value / 1000.));

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
	lbl_gain_value->setText(QString::number(gain_value / 1000.));

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

void SirveApp::HandlePlotDisplayAutoChange(int index)
{
	// When color tab is selected, the histogram is automatically displayed
	if (index == 1) {
		tab_plots->setCurrentIndex(0);
	}

	// When processing tab is selected, the engineering plots are automically displayed
	if (index == 2) {
		tab_plots->setCurrentIndex(1);
	}
}

void SirveApp::ShowCalibrationDialog()
{
	CalibrationDialog calibrate_dialog(calibration_model);

	auto response = calibrate_dialog.exec();

	if (response == 0) {

		return;
	}

	calibration_model = calibrate_dialog.model;
    video_display->SetCalibrationModel(calibrate_dialog.model);
	btn_calculate_radiance->setEnabled(true);
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
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QDir::homePath());

    if (!directory.isEmpty())
    {
        configReaderWriter::SaveWorkspaceFolder(directory);
        config_values = configReaderWriter::ExtractWorkspaceConfigValues();
        workspace = new Workspace(config_values.workspace_folder);

        lbl_current_workspace_folder->setText("Current Workspace Folder: " + config_values.workspace_folder);
        lbl_current_workspace_folder->setWordWrap(true);

        cmb_workspace_name->clear();
        cmb_workspace_name->addItems(workspace->get_workspace_names(config_values.workspace_folder));
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
	menu_file->addAction(action_close);

	menu_settings = menuBar()->addMenu(tr("&Settings"));
	menu_settings->addAction(action_set_timing_offset);
    menu_settings->addAction(action_change_workspace_directory);


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
        DataExport::WriteTrackDataToCsv(save_path, eng_data->get_plotting_frame_data(), track_info->get_plotting_tracks(), track_info->get_manual_plotting_tracks());
	}
	else {
		min_frame = data_plots->index_sub_plot_xmin;
		max_frame = data_plots->index_sub_plot_xmax;

        DataExport::WriteTrackDataToCsv(save_path, eng_data->get_plotting_frame_data(), track_info->get_plotting_tracks(), track_info->get_manual_plotting_tracks(), min_frame, max_frame);
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
	QString color = cmb_color_maps->currentText();

	// find number of color maps
	int number_maps = video_colors.maps.size();

	// cycle through all color maps
	for (int i = 0; i < number_maps; i++)
	{
		// checks to find where input_map matches provided maps
		if (color == video_colors.maps[i].name)
		{
            video_display->HandleColorMapUpdate(video_colors.maps[i].colors);
			color_map_display->set_color_map(video_colors.maps[i].colors);
			return;
		}
	}
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
	video_display->highlight_bad_pixels_colors(bad_pixel_color);
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

	AnnotationListDialog annotate_gui(video_display->annotation_list, standard_info);
    connect(&annotate_gui, &AnnotationListDialog::annotationListUpdated, video_display, &VideoDisplay::HandleAnnotationChanges);
	annotate_gui.exec();
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

void SirveApp::CopyOsmDirectory()
{
	clipboard->setText(abp_file_metadata.osm_path);
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

		// int start_frame = QInputDialog::getInt(this, "Bad Pixel Replacement", "Start frame", 1,  min_frame, max_frame, 1, &ok);
		// 	if (!ok)
		// 		return;
		// int end_frame = QInputDialog::getInt(this, "Bad Pixel Replacement", "End frame (maximum 500 frames from start)", max_frame, start_frame + 5, max_frame, 1, &ok);
		// if (!ok)
		// 	return;
		int start_frame = txt_bad_pixel_start_frame->text().toInt();
		int end_frame = txt_bad_pixel_end_frame->text().toInt();
		ABIRDataResult test_frames = file_processor.LoadImageFile(abp_file_metadata.image_path, start_frame, end_frame, config_values.version);
		
		if(type_choice == 0){ 
			QProgressDialog progress_dialog0("Finding Dead/Out of Scale Pixels", "Cancel", 0, 4);
			progress_dialog0.setWindowTitle("Bad Pixels");
			progress_dialog0.setWindowModality(Qt::ApplicationModal);
			progress_dialog0.setMinimumDuration(0);
			arma::uvec index_dead0 = BadPixels::FindDeadBadscalePixels(test_frames.video_frames_16bit);
			progress_dialog0.setValue(4);

			QProgressDialog progress_dialog0a("Finding Outlier Pixels", "Cancel", 0, test_frames.video_frames_16bit.size());
			progress_dialog0a.setWindowTitle("Bad Pixels");
			progress_dialog0a.setWindowModality(Qt::ApplicationModal);
			progress_dialog0a.setMinimumDuration(0);

			if (outlier_method == 0){		
				arma::uvec index_outlier0 = BadPixels::IdentifyBadPixelsMedian(N,test_frames.video_frames_16bit, progress_dialog0a);	
				index_outlier0 = arma::unique(arma::join_vert(index_outlier0,index_dead0));
				std::vector<unsigned int> dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier0);
				ReplaceBadPixels(dead_pixels);		
			}
			else{	
				u_int window_length = txt_moving_median_N->text().toUInt();
				arma::uvec index_outlier0 = BadPixels::IdentifyBadPixelsMovingMedian(window_length,N,test_frames.video_frames_16bit, progress_dialog0a);
				index_outlier0 = arma::unique(arma::join_vert(index_outlier0,index_dead0));
				std::vector<unsigned int> dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier0);	
				ReplaceBadPixels(dead_pixels);
			}
		} else if (type_choice == 1){
			QProgressDialog progress_dialog1("Finding Dead/Out of Scale Pixels", "Cancel", 0,4);
			progress_dialog1.setWindowTitle("Bad Pixels");
			progress_dialog1.setWindowModality(Qt::ApplicationModal);
			progress_dialog1.setMinimumDuration(0);
			arma::uvec index_dead1 = BadPixels::FindDeadBadscalePixels(test_frames.video_frames_16bit);
			progress_dialog1.setValue(2);
			std::vector<unsigned int> dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_dead1);
			progress_dialog1.setValue(3);
			ReplaceBadPixels(dead_pixels);
			progress_dialog1.setValue(4);
		} else {
			QProgressDialog progress_dialog2("Finding Outlier Pixels", "Cancel", 0, test_frames.video_frames_16bit.size());
			progress_dialog2.setWindowTitle("Bad Pixels");
			progress_dialog2.setWindowModality(Qt::ApplicationModal);
			progress_dialog2.setMinimumDuration(0);
			if (outlier_method == 0){		
				arma::uvec index_outlier2 = BadPixels::IdentifyBadPixelsMedian(N,test_frames.video_frames_16bit, progress_dialog2);	
				std::vector<unsigned int> dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier2);
				ReplaceBadPixels(dead_pixels);		
			}
			else{	
				u_int window_length = txt_moving_median_N->text().toUInt();
				arma::uvec index_outlier2 = BadPixels::IdentifyBadPixelsMovingMedian(window_length,N,test_frames.video_frames_16bit, progress_dialog2);
				std::vector<unsigned int> dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier2);	
				ReplaceBadPixels(dead_pixels);
			}
		}		

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
	base_state.replaced_pixels = pixels_to_replace;
	QProgressDialog progress_dialog("Replacing Bad Pixels", "Cancel", 0, base_state.details.frames_16bit.size());
	progress_dialog.setWindowTitle("Adjusting Bad Pixels");
	progress_dialog.setWindowModality(Qt::ApplicationModal);
	progress_dialog.setMinimumDuration(0);
	progress_dialog.setValue(1);
    BadPixels::ReplacePixelsWithNeighbors(base_state.details.frames_16bit, pixels_to_replace, base_state.details.x_pixels, progress_dialog);

    video_display->container.ClearProcessingStates();
    video_display->container.AddProcessingState(base_state);

	lbl_bad_pixel_count->setText("Bad pixels currently replaced: " + QString::number(pixels_to_replace.size()));
	chk_highlight_bad_pixels->setEnabled(true);
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
        ApplyFixedNoiseSuppression(abp_file_metadata.image_path, image_path, start_frame, end_frame);
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

void SirveApp::ExecuteNonUniformityCorrectionSelectionOption()
{
	// QStringList options;
	// options << tr("From Current File") << tr("From External File");

	// QStringList shadow_options;
	// shadow_options << tr("Hide Shadow") << tr("Show Shadow");

	// QString hide_shadow_choice = "Hide Shadow";

	// bool ok;
	// QString item = QInputDialog::getItem(this, "Fixed Mean Noise Suppression", "Options", options, 0, false, &ok);
	//Pause the video if it's running
	playback_controller->StopTimer();

    processingState original = video_display->container.CopyCurrentState();

	int number_video_frames = static_cast<int>(original.details.frames_16bit.size());
	// if (!ok)
	// 	return;

	if (!chk_FNS_external_file->isChecked())
	{
		int delta_frames = data_plots->index_sub_plot_xmax - data_plots->index_sub_plot_xmin;
		// int start_frame = QInputDialog::getInt(this, "Fixed Noise Suppression", "Start frame", 1, -delta_frames, delta_frames, 1, &ok);
		// if (!ok)
		// 	return;

		int start_frame = txt_FNS_start_frame->text().toInt();


		// int number_of_frames_for_avg = QInputDialog::getInt(this, "Fixed Noise Suppresssion", "Number of frames to use for suppression", 10, 1,  number_video_frames, 1, &ok);
		// if (!ok)
		// 	return;

		int end_frame = txt_FNS_end_frame->text().toInt();
		// int end_frame = start_frame + number_of_frames_for_avg - 1;
        ApplyFixedNoiseSuppression(abp_file_metadata.image_path, abp_file_metadata.image_path, start_frame, end_frame);

	}
	else
	{
        ApplyFixedNoiseSuppressionFromExternalFile();
	}

}

void SirveApp::ApplyFixedNoiseSuppression(QString image_path, QString file_path, unsigned int start_frame, unsigned int end_frame)
{
	int compare = QString::compare(file_path, image_path, Qt::CaseInsensitive);
	if (compare!=0){
        if (!VerifyFrameSelection(start_frame, end_frame))
		{
			QtHelpers::LaunchMessageBox(QString("Invalid Frame Selection"), "Fixed noise suppression not completed, invalid frame selection");
			return;
		}
	}

    processingState original = video_display->container.CopyCurrentState();

	processingState noise_suppresions_state = original;
	noise_suppresions_state.details.frames_16bit.clear();

	int number_frames = static_cast<int>(original.details.frames_16bit.size());

	QProgressDialog progress_dialog("Memory safe fixed noise suppression", "Cancel", 0, number_frames);
	progress_dialog.setWindowTitle("Fixed Noise Suppression");
	progress_dialog.setWindowModality(Qt::ApplicationModal);
	progress_dialog.setMinimumDuration(0);
	progress_dialog.setValue(1);

	FixedNoiseSuppression FNS;
	noise_suppresions_state.details.frames_16bit = FNS.ProcessFrames(abp_file_metadata.image_path, file_path, start_frame, end_frame, config_values.version, original.details, progress_dialog);

	// QProgressDialog progress_dialog2("Median Filter", "Cancel", 0, number_frames);
	// progress_dialog2.setWindowTitle("Filter");
	// progress_dialog2.setWindowModality(Qt::ApplicationModal);
	// progress_dialog2.setMinimumDuration(0);
	// progress_dialog2.setValue(1);
	// // noise_suppresions_state.details.frames_16bit = Deinterlacing::cross_correlation(original.details, progress_dialog);
	// noise_suppresions_state.details.frames_16bit = MedianFilter::median_filter_standard(original.details, 5, progress_dialog2);


	noise_suppresions_state.method = ProcessingMethod::fixed_noise_suppression;
	noise_suppresions_state.FNS_file_path = file_path;
	noise_suppresions_state.FNS_start_frame = start_frame;
	noise_suppresions_state.FNS_stop_frame = end_frame;
    video_display->container.AddProcessingState(noise_suppresions_state);

	QFileInfo fi(file_path);
	QString fileName = fi.fileName().toLower();
	QString current_filename = abp_file_metadata.file_name.toLower() + ".abpimage";

	if (fileName == current_filename)
		fileName = "Current File";

	QString description = "File: " + fileName + "\n";
	description += "From frame " + QString::number(start_frame) + " to " + QString::number(end_frame);

	lbl_fixed_suppression->setText(description);
}

void SirveApp::ExecuteDeinterlace()
{
	DeinterlaceType deinterlace_method_type = static_cast<DeinterlaceType>(cmb_deinterlace_options->currentIndex());

    ApplyDeinterlacing(deinterlace_method_type);
}

void SirveApp::ApplyDeinterlacing(DeinterlaceType deinterlace_method_type)
{
    processingState original = video_display->container.CopyCurrentState();

	//Deinterlace deinterlace_method(deinterlace_method_type, original.details.x_pixels, original.details.y_pixels);

	processingState deinterlace_state = original;
	deinterlace_state.details.frames_16bit.clear();

	// Apply de-interlace to the frames

	int number_frames = static_cast<int>(original.details.frames_16bit.size());

	QProgressDialog progress("", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.setWindowTitle(QString("Deinterlacing Frames"));
	progress.setMaximum(number_frames - 1);
	progress.setLabelText(QString("Cross correlation..."));
	progress.setMinimumWidth(300);

	deinterlace_state.details.frames_16bit = Deinterlacing::CrossCorrelation(original.details, progress);
	// for (int i = 0; i < number_frames; i++)
	// {
	// 	progress.setValue(i);

	// 	deinterlace_state.details.frames_16bit.push_back(deinterlace_method.deinterlace_frame(original.details.frames_16bit[i]));
	// 	if (progress.wasCanceled())
	// 		break;
	// }

	if (progress.wasCanceled())
	{
		return;
	}

	deinterlace_state.method = ProcessingMethod::deinterlace;
	deinterlace_state.deint_type = deinterlace_method_type;
    video_display->container.AddProcessingState(deinterlace_state);
}

void SirveApp::HandleOsmTracksToggle()
{
	bool current_status = video_display->plot_tracks;
    video_display->ToggleOsmTracks(!current_status);

	if (!current_status) {
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

void SirveApp::HandleProcessingStatesCleared()
{
	cmb_processing_states->clear();
	lbl_adaptive_noise_suppression_status->setText("No Frames Setup");
	lbl_fixed_suppression->setText("No Frames Selected");
}

void SirveApp::ExecuteNoiseSuppression()
{
	//-----------------------------------------------------------------------------------------------
	// get user selected frames for suppression

	int delta_frames = data_plots->index_sub_plot_xmax - data_plots->index_sub_plot_xmin;

	// QStringList shadow_options;
	// shadow_options << tr("Hide Shadow") << tr("Show Shadow");

	// bool ok;

	// int relative_start_frame = QInputDialog::getInt(this, "Adaptive Noise Suppression", "Relative start frame", -30, -delta_frames, delta_frames, 1, &ok);
	// if (!ok)
	// 	return;

	// int number_of_frames = QInputDialog::getInt(this, "Adaptive Noise Suppresssion", "Number of frames to use for suppression", 5, 1, std::abs(relative_start_frame), 1, &ok);
	// if (!ok)
	// 	return;

	// QString hide_shadow_choice = QInputDialog::getItem(this, "Adaptive Noise Suppression", "Options", shadow_options, 0, false, &ok);
	// if (!ok)
	// 	return;

	int relative_start_frame = txt_ANS_offset_frames->text().toInt();
	int number_of_frames = txt_ANS_number_frames->text().toInt();
	QString hide_shadow_choice = "Hide Shadow";
	if(chk_hide_shadow->isChecked()){
		hide_shadow_choice = "Hide Shadow";
	}
	else
	{
		hide_shadow_choice = "Show Shadow";
	}

	ApplyAdaptiveNoiseCorrection(relative_start_frame, number_of_frames, hide_shadow_choice);
}

void SirveApp::ApplyAdaptiveNoiseCorrection(int relative_start_frame, int number_of_frames, QString hide_shadow_choice)
{
	//Pause the video if it's running
	playback_controller->StopTimer();

	// int NThresh = cmb_shadow_threshold->currentIndex() + 1;
	int NThresh = 1;
	processingState original = video_display->container.CopyCurrentState();
	int number_video_frames = static_cast<int>(original.details.frames_16bit.size());

	processingState noise_suppresions_state = original;
	noise_suppresions_state.details.frames_16bit.clear();

	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	// DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
	DWORDLONG availPhysMem = memInfo.ullAvailPhys;
	double R = double(availPhysMem)/(double(number_video_frames)*16*640*480);

	// if ( R >= 1.5 ){
	// 	QProgressDialog progress_dialog("Fast adaptive noise suppression", "Cancel", 0, 3*number_video_frames);
	// 	progress_dialog.setWindowTitle("Adaptive Noise Suppression");
	// 	progress_dialog.setWindowModality(Qt::ApplicationModal);
	// 	progress_dialog.setMinimumDuration(0);
	// 	progress_dialog.setValue(1);
	// 	noise_suppresions_state.details.frames_16bit = AdaptiveNoiseSuppression::process_frames_fast(relative_start_frame, number_of_frames, original.details, hide_shadow_choice, progress_dialog);
	// }
	// else{
		QProgressDialog progress_dialog("Memory safe adaptive noise suppression", "Cancel", 0, number_video_frames);
		progress_dialog.setWindowTitle("Adaptive Noise Suppression");
		progress_dialog.setWindowModality(Qt::ApplicationModal);
		progress_dialog.setMinimumDuration(0);
		progress_dialog.setValue(1);
		noise_suppresions_state.details.frames_16bit = AdaptiveNoiseSuppression::ProcessFramesConserveMemory(relative_start_frame, number_of_frames, NThresh, original.details, hide_shadow_choice, progress_dialog);
	// }

	// QProgressDialog progress_dialog2("Median Filter", "Cancel", 0, number_video_frames);
	// progress_dialog2.setWindowTitle("Filter");
	// progress_dialog2.setWindowModality(Qt::ApplicationModal);
	// progress_dialog2.setMinimumDuration(0);
	// progress_dialog2.setValue(1);
	// noise_suppresions_state.details.frames_16bit = MedianFilter::median_filter_standard(noise_suppresions_state.details, 5, progress_dialog2);

	QString description = "Filter starts at ";
	if (relative_start_frame > 0)
		description += "+";

	lbl_adaptive_noise_suppression_status->setWordWrap(true);
	description += QString::number(relative_start_frame) + " frames and averages " + QString::number(number_of_frames) + " frames";

	lbl_adaptive_noise_suppression_status->setText(description);

	bool hide_shadow_bool = hide_shadow_choice == "Hide Shadow";

	noise_suppresions_state.method = ProcessingMethod::adaptive_noise_suppression;
	noise_suppresions_state.ANS_relative_start_frame = relative_start_frame;
	noise_suppresions_state.ANS_num_frames = number_of_frames;
	noise_suppresions_state.ANS_hide_shadow = hide_shadow_bool;
    video_display->container.AddProcessingState(noise_suppresions_state);
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

	image_vector = (image_vector - arma::mean(image_vector))/(12*arma::stddev(image_vector)) + .5;
	//  image_vector = (image_vector - arma::mean(image_vector))/(image_vector.max()) + .5;

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

	double lift = slider_lift->value() / 1000.;
	double gain = slider_gain->value() / 1000.;

	histogram_plot->UpdateHistogramAbsPlot(image_vector, lift, gain);

	// Correct image based on min/max value inputs
    ColorCorrection::UpdateColor(image_vector, lift, gain);

	histogram_plot->UpdateHistogramRelPlot(image_vector);

	image_vector = image_vector/image_vector.max() * 255;

	std::vector<double>out_vector = arma::conv_to<std::vector<double>>::from(image_vector);
	std::vector<uint8_t> display_ready_converted_values = {out_vector.begin(), out_vector.end()};

    video_display->UpdateFrameVector(original_frame_vector, display_ready_converted_values);
}
