#include "SirveApp.h"

SirveApp::SirveApp(QWidget *parent)
	: QMainWindow(parent)
{
	config_values = configreader::load();

    // use this to change the current working directory
    DirectoryPicker *directoryPicker = new DirectoryPicker(this);

    // Connect to the directorySelected signal if you want to perform some action when a directory is selected
    connect(directoryPicker, &DirectoryPicker::directorySelected, [=](const QString &directory) {
        qDebug() << "Selected directory:" << directory;
        // Do something with the selected directory
    });

	// establish object that will hold video and connect it to the playback thread
	color_map_display = new ColorMapDisplay(video_colors.maps[0].colors);
	video_display = new VideoDisplay(video_colors.maps[0].colors);
	video_display->moveToThread(&thread_video);

	histogram_plot = new HistogramLine_Plot();

	setup_ui();
	popout_video = new PopoutDialog();
	popout_histogram = new PopoutDialog();
	popout_engineering = new PopoutDialog();

	//---------------------------------------------------------------------------

	// establish object to control playback timer and move to a new thread
	playback_controller = new Playback(1);
	playback_controller->moveToThread(&thread_timer);
	
	// establish copy copy
	clipboard = QApplication::clipboard();
	connect(btn_copy_directory, &QPushButton::clicked, this, &SirveApp::copy_osm_directory);

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
	setup_connections();

	toggle_relative_histogram(false);
	toggle_video_playback_options(false);
	enable_engineering_plot_options();

	create_menu_actions();

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

void SirveApp::setup_ui() {

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
	tab_menu->addTab(setup_file_import_tab(), "Import");
	tab_menu->addTab(setup_color_correction_tab(), "Color/Overlays");
	tab_menu->addTab(setup_filter_tab(), "Processing");
	tab_menu->addTab(setup_workspace_tab(), "Workspace");

	QSizePolicy fixed_width;
	fixed_width.setHorizontalPolicy(QSizePolicy::Minimum);
	fixed_width.setVerticalPolicy(QSizePolicy::Preferred);
	tab_menu->setSizePolicy(fixed_width);

	setup_video_frame();
	setup_plot_frame();

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
	// ------------------------------------------------------------------------

	this->setCentralWidget(frame_main);
	this->show();
    this->directoryPicker.show();

}

QWidget* SirveApp::setup_file_import_tab() {


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
	btn_get_frames = new QPushButton(" Load Frames");

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

QWidget* SirveApp::setup_color_correction_tab()
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
	btn_add_annotations = new QPushButton("Add/Edit Annotations");

	cmb_color_maps = new QComboBox();
	int number_maps = video_colors.maps.size();
	for (int i = 0; i < number_maps; i++)
		cmb_color_maps->addItem(video_colors.maps[i].name);

	QStringList colors = ColorScheme::GetTrackColors();

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

QWidget* SirveApp::setup_filter_tab() {

	QWidget* widget_tab_processing = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_processing = new QVBoxLayout(widget_tab_processing);


	// ------------------------------------------------------------------------
	QGridLayout* grid_bad_pixels = new QGridLayout();
	grid_bad_pixels->addWidget(QtHelpers::HorizontalLine(), 0, 0, 1, 2);

	QLabel* label_bad_pixel = new QLabel("Replacing Bad Pixels With Local Average");
	grid_bad_pixels->addWidget(label_bad_pixel, 1, 0, 1, 2);

	lbl_bad_pixel_count = new QLabel("");
	grid_bad_pixels->addWidget(lbl_bad_pixel_count, 2, 0, 1, 2);

	chk_highlight_bad_pixels = new QCheckBox("Highlight Bad Pixels");
	grid_bad_pixels->addWidget(chk_highlight_bad_pixels, 3, 0, 1, 1);

	btn_bad_pixel_identification = new QPushButton("Replace Dead Pixels");
	connect(btn_bad_pixel_identification, &QPushButton::clicked, this, &SirveApp::ui_replace_bad_pixels);
	grid_bad_pixels->addWidget(btn_bad_pixel_identification, 3, 1, 1, 1);

	grid_bad_pixels->addWidget(QtHelpers::HorizontalLine(), 4, 0, 1, 2);

	vlayout_tab_processing->addLayout(grid_bad_pixels);

	// ------------------------------------------------------------------------

	QLabel* label_nuc = new QLabel("Fixed Noise Suppression");
	lbl_fixed_suppression = new QLabel("No Frames Selected");

	btn_create_nuc = new QPushButton("Create Filter");

	//QWidget* widget_tab_processing_nuc = new QWidget();
	QGridLayout* grid_tab_processing_nuc = new QGridLayout();

	grid_tab_processing_nuc->addWidget(label_nuc, 0, 0, 1, 2);
	grid_tab_processing_nuc->addWidget(lbl_fixed_suppression, 1, 0, 1, 2);
	grid_tab_processing_nuc->addWidget(btn_create_nuc, 2, 1);
	grid_tab_processing_nuc->addWidget(QtHelpers::HorizontalLine(), 3, 0, 1, 2);

	vlayout_tab_processing->addLayout(grid_tab_processing_nuc);

	// ------------------------------------------------------------------------

	QLabel* label_adaptive_noise_suppression = new QLabel("Adaptive Noise Suppression");
	label_adaptive_noise_suppression_status = new QLabel("No Frames Setup");
	btn_bgs = new QPushButton("Create Filter");

	//QWidget* widget_tab_processing_bgs = new QWidget();
	QGridLayout* grid_tab_processing_bgs = new QGridLayout();

	grid_tab_processing_bgs->addWidget(label_adaptive_noise_suppression, 0, 0, 1, 2);
	grid_tab_processing_bgs->addWidget(label_adaptive_noise_suppression_status, 1, 0, 1, 2);
	grid_tab_processing_bgs->addWidget(btn_bgs, 2, 1);
	grid_tab_processing_bgs->addWidget(QtHelpers::HorizontalLine(), 3, 0, 1, 2);

	vlayout_tab_processing->addLayout(grid_tab_processing_bgs);

	// ------------------------------------------------------------------------
	QLabel* label_deinterlace = new QLabel("De-Interlace Methods");
	cmb_deinterlace_options = new QComboBox();
	btn_deinterlace = new QPushButton("Create Filter");

	cmb_deinterlace_options->addItem("Max Absolute Value");
	cmb_deinterlace_options->addItem("Centroid");
	cmb_deinterlace_options->addItem("Avg Cross Correlation");

	QGridLayout* grid_tab_processing_deinterlace = new QGridLayout();

	grid_tab_processing_deinterlace->addWidget(label_deinterlace, 0, 0, 1, 2);
	grid_tab_processing_deinterlace->addWidget(cmb_deinterlace_options, 1, 0, 1, 2);
	grid_tab_processing_deinterlace->addWidget(btn_deinterlace, 2, 1);
	grid_tab_processing_deinterlace->addWidget(QtHelpers::HorizontalLine(), 3, 0, 1, 2);

	vlayout_tab_processing->addLayout(grid_tab_processing_deinterlace);

	// ------------------------------------------------------------------------

	vlayout_tab_processing->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout

	return widget_tab_processing;
}

QWidget* SirveApp::setup_workspace_tab(){
	QWidget* widget_tab_workspace = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_workspace = new QVBoxLayout(widget_tab_workspace);

	cmb_workspace_name = new QComboBox();
	cmb_workspace_name->addItems(workspace.get_workspace_names());

	btn_workspace_load = new QPushButton("Load Workspace");
	btn_workspace_save = new QPushButton("Save Workspace");

	cmb_processing_states = new QComboBox();
	btn_undo_step = new QPushButton("Undo One Step");

	QLabel *lbl_track = new QLabel("Manual Track Management");
	lbl_create_track_message = new QLabel("");
	lbl_create_track_message->setStyleSheet("QLabel { color: red }");
	QFont large_font;
	large_font.setPointSize(16);
	lbl_create_track_message->setFont(large_font);
	btn_create_track = new QPushButton("Create Track");
	btn_finish_create_track = new QPushButton("Finish");
	btn_finish_create_track->setHidden(true);
	btn_finish_create_track->setFont(large_font);
	btn_import_tracks = new QPushButton("Import Tracks");

	QGridLayout* grid_workspace = new QGridLayout();
	grid_workspace->addWidget(cmb_workspace_name, 0, 0, 1, -1);
	grid_workspace->addWidget(btn_workspace_load, 1, 0, 1, 1);
	grid_workspace->addWidget(btn_workspace_save, 1, 1, 1, 1);
	grid_workspace->addWidget(QtHelpers::HorizontalLine(), 2, 0, 1, -1);
	grid_workspace->addWidget(cmb_processing_states, 3, 0, 1, 1);
	grid_workspace->addWidget(btn_undo_step, 3, 1, 1, 1);
	grid_workspace->addWidget(QtHelpers::HorizontalLine(), 4, 0, 1, -1);
	grid_workspace->addWidget(lbl_track, 5, 0, 1, -1, Qt::AlignCenter);
	grid_workspace->addWidget(lbl_create_track_message, 6, 0, 1, 1);
	grid_workspace->addWidget(btn_create_track, 6, 1, 1, 1);
	grid_workspace->addWidget(btn_finish_create_track, 6, 1, 1, 1);
	grid_workspace->addWidget(btn_import_tracks, 7, 0, 1, -1);

	tm_widget = new TrackManagementWidget(widget_tab_workspace);
	QScrollArea *track_management_scroll_area = new QScrollArea();
    track_management_scroll_area->setWidgetResizable( true );
	track_management_scroll_area->setWidget(tm_widget);
	track_management_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	grid_workspace->addWidget(track_management_scroll_area, 8, 0, 1, -1);

	vlayout_tab_workspace->addLayout(grid_workspace);
	vlayout_tab_workspace->insertStretch(-1, 0);
	return widget_tab_workspace;
}

void SirveApp::setup_video_frame(){

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

void SirveApp::setup_plot_frame() {

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
}

void SirveApp::setup_connections() {


	//---------------------------------------------------------------------------

	connect(&video_display->container, &Video_Container::update_display_video, this, &SirveApp::handle_frame_change);
	connect(btn_undo_step, &QPushButton::clicked, &video_display->container, &Video_Container::undo);
	connect(playback_controller, &Playback::frame_selected, this, &SirveApp::handle_frame_number_change);

	connect(&video_display->container, &Video_Container::state_added, this, &SirveApp::handle_new_processing_state);
	connect(&video_display->container, &Video_Container::state_removed, this, &SirveApp::handle_processing_state_removal);
	connect(&video_display->container, &Video_Container::states_cleared, this, &SirveApp::handle_cleared_processing_states);

	connect(cmb_processing_states, qOverload<int>(&QComboBox::currentIndexChanged), &video_display->container, &Video_Container::select_state);

	connect(histogram_plot, &HistogramLine_Plot::click_drag_histogram, this, &SirveApp::histogram_clicked);

	connect(video_display, &VideoDisplay::add_new_bad_pixels, this, &SirveApp::receive_new_bad_pixels);
	connect(video_display, &VideoDisplay::remove_bad_pixels, this, &SirveApp::receive_new_good_pixels);

	//---------------------------------------------------------------------------

	connect(tab_menu, &QTabWidget::currentChanged, this, &SirveApp::auto_change_plot_display);
	connect(chk_relative_histogram, &QCheckBox::toggled, this, &SirveApp::toggle_relative_histogram);

	//---------------------------------------------------------------------------
	// Link color correction sliders to changing color correction values
	connect(slider_gain, &QSlider::valueChanged, this, &SirveApp::gain_slider_toggled);
	connect(slider_lift, &QSlider::valueChanged, this, &SirveApp::lift_slider_toggled);

	connect(btn_reset_color_correction, &QPushButton::clicked, this, &SirveApp::reset_color_correction);

	connect(chk_auto_lift_gain, &QCheckBox::stateChanged, this, &SirveApp::handle_chk_auto_lift_gain);
	connect(txt_lift_sigma, &QLineEdit::editingFinished, this, &SirveApp::update_global_frame_vector);
	connect(txt_gain_sigma, &QLineEdit::editingFinished, this, &SirveApp::update_global_frame_vector);

	//---------------------------------------------------------------------------

	connect(chk_show_tracks, &QCheckBox::stateChanged, this, &SirveApp::toggle_osm_tracks);
	connect(cmb_tracker_color, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::edit_tracker_color);

	connect(chk_sensor_track_data, &QCheckBox::stateChanged, video_display, &VideoDisplay::toggle_sensor_boresight_data);
	connect(chk_show_time, &QCheckBox::stateChanged, video_display, &VideoDisplay::toggle_frame_time);
	connect(cmb_color_maps, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::edit_color_map);
	connect(cmb_text_color, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::edit_banner_color);

	connect(btn_add_annotations, &QPushButton::clicked, this, &SirveApp::annotate_video);
	connect(btn_change_banner_text, &QPushButton::clicked, this, &SirveApp::edit_banner_text);

	//---------------------------------------------------------------------------

	// Link horizontal slider to playback controller
	connect(playback_controller, &Playback::frame_selected, slider_video, &QSlider::setValue);
	connect(slider_video, &QSlider::valueChanged, playback_controller, &Playback::set_current_frame_number);

	//---------------------------------------------------------------------------

	// Link playback to play controls
	connect(btn_play, &QPushButton::clicked, playback_controller, &Playback::start_timer);
	connect(btn_pause, &QPushButton::clicked, playback_controller, &Playback::stop_timer);
	connect(btn_reverse, &QPushButton::clicked, playback_controller, &Playback::reverse);


	connect(btn_fast_forward, &QPushButton::clicked, playback_controller, &Playback::speed_timer);
	connect(btn_slow_back, &QPushButton::clicked, playback_controller, &Playback::slow_timer);
	connect(btn_next_frame, &QPushButton::clicked, playback_controller, &Playback::next_frame);
	connect(video_display, &VideoDisplay::advance_frame, playback_controller, &Playback::next_frame);
	connect(btn_prev_frame, &QPushButton::clicked, playback_controller, &Playback::prev_frame);
	connect(btn_frame_record, &QPushButton::clicked, this, &SirveApp::start_stop_video_record);

	connect(btn_fast_forward, &QPushButton::clicked, this, &SirveApp::update_fps);
	connect(btn_slow_back, &QPushButton::clicked, this, &SirveApp::update_fps);

	connect(btn_zoom, &QPushButton::clicked, this, &SirveApp::toggle_zoom_on_video);
	connect(btn_calculate_radiance, &QPushButton::clicked, this, &SirveApp::toggle_calculation_on_video);
	connect(video_display, &VideoDisplay::clear_mouse_buttons, this, &SirveApp::clear_zoom_and_calculation_buttons);

	connect(btn_popout_video, &QPushButton::clicked, this, &SirveApp::handle_popout_video_btn);

	//---------------------------------------------------------------------------

	//Link buttons to functions
	connect(btn_load_osm, &QPushButton::clicked, this, &SirveApp::ui_choose_abp_file);
	connect(btn_calibration_dialog, &QPushButton::clicked, this, &SirveApp::show_calibration_dialog);
	connect(btn_get_frames, &QPushButton::clicked, this, &SirveApp::ui_load_abir_data);
	connect(txt_end_frame, &QLineEdit::returnPressed, this, &SirveApp::ui_load_abir_data);

	connect(chk_highlight_bad_pixels, &QPushButton::clicked, video_display, &VideoDisplay::highlight_bad_pixels);

	connect(btn_create_nuc, &QPushButton::clicked, this, &SirveApp::ui_execute_non_uniformity_correction_selection_option);

	connect(btn_bgs, &QPushButton::clicked, this, &SirveApp::ui_execute_noise_suppression);

	connect(btn_deinterlace, &QPushButton::clicked, this, &SirveApp::ui_execute_deinterlace);

	//---------------------------------------------------------------------------

	connect(btn_workspace_save, &QPushButton::clicked, this, &SirveApp::save_workspace);
	connect(btn_workspace_load, &QPushButton::clicked, this, &SirveApp::load_workspace);
	connect(btn_import_tracks, &QPushButton::clicked, this, &SirveApp::import_tracks);
	connect(btn_create_track, &QPushButton::clicked, this, &SirveApp::handle_btn_create_track);
	connect(btn_finish_create_track, &QPushButton::clicked, this, &SirveApp::handle_btn_finish_create_track);
	connect(video_display, &VideoDisplay::finish_create_track, this, &SirveApp::handle_btn_finish_create_track);

	connect(tm_widget, &TrackManagementWidget::display_track, video_display, &VideoDisplay::show_manual_track_id);
	connect(tm_widget, &TrackManagementWidget::hide_track, video_display, &VideoDisplay::hide_manual_track_id);
	connect(tm_widget, &TrackManagementWidget::delete_track, this, &SirveApp::handle_removal_of_track);
	connect(tm_widget, &TrackManagementWidget::recolor_track, this, &SirveApp::handle_manual_track_recoloring);

	// Connect epoch button click to function
	connect(btn_apply_epoch, &QPushButton::clicked, this, &SirveApp::apply_epoch_time);

	//Enable saving frame
	connect(btn_frame_save, &QPushButton::clicked, this, &SirveApp::save_frame);

	//---------------------------------------------------------------------------
	// Connect x-axis and y-axis changes to functions
	connect(cmb_plot_yaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::plot_change);
	connect(cmb_plot_xaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::plot_change);

	// Connect save button functions
	connect(btn_save_plot, &QPushButton::clicked, this, &SirveApp::save_plot);

	//---------------------------------------------------------------------------
	// connect the plot radial buttons to adjust plot

	connect(rad_log, &QRadioButton::toggled, this, &SirveApp::plot_change);
	connect(rad_decimal, &QRadioButton::toggled, this, &SirveApp::plot_change);
	connect(rad_linear, &QRadioButton::toggled, this, &SirveApp::plot_change);
	connect(rad_scientific, &QRadioButton::toggled, this, &SirveApp::plot_change);

	//---------------------------------------------------------------------------
	connect(btn_popout_histogram, &QPushButton::clicked, this, &SirveApp::handle_popout_histogram_btn);
}

void SirveApp::import_tracks()
{
	QString base_track_folder = "workspace";
	QString file_selection = QFileDialog::getOpenFileName(this, ("Open Track File"), base_track_folder, ("Track File (*.csv)"));

	int compare = QString::compare(file_selection, "", Qt::CaseInsensitive);
	if (compare == 0) {
		QtHelpers::LaunchMessageBox(QString("Issue Finding File"), "No track file was selected.");
		return;
	}

	TrackFileReadResult result = track_info->read_tracks_from_file(file_selection);

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
		video_display->add_manual_track_id_to_show_later(track_id);
		tm_widget->add_track_control(track_id);
	}

	track_info->add_manual_tracks(result.frames);

	int index0 = data_plots->index_sub_plot_xmin;
	int index1 = data_plots->index_sub_plot_xmax + 1;
	video_display->update_manual_track_data(track_info->get_manual_frames(index0, index1));
	data_plots->update_manual_plotting_track_frames(track_info->get_manual_plotting_tracks(), track_info->get_manual_track_ids());
	plot_change();
}

void SirveApp::handle_btn_create_track()
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
			std::vector<std::optional<TrackDetails>> existing_track_details = track_info->copy_manual_track(track_id);
			prepare_for_track_creation(track_id);
			video_display->enter_track_creation_mode(existing_track_details);
		}
	}
	else
	{
		std::vector<std::optional<TrackDetails>> empty_track_details = track_info->get_empty_track();
		prepare_for_track_creation(track_id);
		video_display->enter_track_creation_mode(empty_track_details);
	}
}

void SirveApp::prepare_for_track_creation(int track_id)
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

void SirveApp::handle_btn_finish_create_track()
{
	const std::vector<std::optional<TrackDetails>> & created_track_details = video_display->get_created_track_details();
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
		exit_track_creation_mode();
		return;
	}

	auto response = QtHelpers::LaunchYesNoMessageBox("Finish Track Creation", "This action will finalize track creation. Pressing \"Yes\" will save the track, \"No\" will cancel track editing, and \"Cancel\" will return to track editing mode. Are you finished editing the track?", true);

	if (response == QMessageBox::Cancel)
	{
		return;
	}

	if (response == QMessageBox::Yes)
	{
		QString base_track_folder = "workspace";
		QString new_track_file_name = QFileDialog::getSaveFileName(this, "Select a new file to save the track into", base_track_folder, "CSV (*.csv)");
		if (new_track_file_name.isEmpty())
		{
			QtHelpers::LaunchMessageBox("Returning to Track Creation", "An invalid or empty file was chosen. To prevent data loss, edited tracks must be saved to disk to finish track creation. Returning to track editing mode.");
			return;
		}

		tm_widget->add_track_control(currently_editing_or_creating_track_id);
		video_display->add_manual_track_id_to_show_later(currently_editing_or_creating_track_id);
		track_info->add_created_manual_track(currently_editing_or_creating_track_id, created_track_details, new_track_file_name);

		int index0 = data_plots->index_sub_plot_xmin;
		int index1 = data_plots->index_sub_plot_xmax + 1;
		video_display->update_manual_track_data(track_info->get_manual_frames(index0, index1));
		data_plots->update_manual_plotting_track_frames(track_info->get_manual_plotting_tracks(), track_info->get_manual_track_ids());
		plot_change();
	}

	exit_track_creation_mode();
}

void SirveApp::exit_track_creation_mode()
{
	btn_finish_create_track->setHidden(true);
	btn_create_track->setHidden(false);
	lbl_create_track_message->setText("");
	currently_editing_or_creating_track_id = -1;
	tab_menu->setTabEnabled(0, true);
	tab_menu->setTabEnabled(2, true);
	btn_workspace_load->setDisabled(false);
	btn_workspace_save->setDisabled(false);

	video_display->exit_track_creation_mode();
}

void SirveApp::handle_removal_of_track(int track_id)
{
	tm_widget->remove_track_control(track_id);
	track_info->remove_manual_track(track_id);
	int index0 = data_plots->index_sub_plot_xmin;
	int index1 = data_plots->index_sub_plot_xmax + 1;
	video_display->update_manual_track_data(track_info->get_manual_frames(index0, index1));
	video_display->delete_manual_track(track_id);
	data_plots->update_manual_plotting_track_frames(track_info->get_manual_plotting_tracks(), track_info->get_manual_track_ids());
	plot_change();
}

void SirveApp::handle_manual_track_recoloring(int track_id, QColor new_color)
{
	video_display->recolor_manual_track(track_id, new_color);
	data_plots->recolor_manual_track(track_id, new_color);
	plot_change(); //Note: Engineering_Plots does not yet control its own graphical updates like VideoDisplay
}

void SirveApp::save_workspace()
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
		workspace.save_state(workspace_name, abp_file_metadata.image_path, data_plots->index_sub_plot_xmin + 1, data_plots->index_sub_plot_xmax + 1, video_display->container.get_processing_states(), video_display->annotation_list);
		cmb_workspace_name->clear();
		cmb_workspace_name->addItems(workspace.get_workspace_names());
		cmb_workspace_name->setCurrentText(workspace_name);
	}
}

void SirveApp::load_workspace()
{
	QString current_workspace_name = cmb_workspace_name->currentText();
	WorkspaceValues workspace_vals = workspace.load_state(current_workspace_name);

	int compare = QString::compare(workspace_vals.image_path, "", Qt::CaseInsensitive);
	if (compare == 0) {
		QtHelpers::LaunchMessageBox(QString("Issue Loading Workspace"), "The workspace is empty.");
		return;
	}

	bool validated = validate_abp_files(workspace_vals.image_path);
	if (validated) {
		load_osm_data();
	}

	if (workspace_vals.start_frame == 0 || workspace_vals.end_frame == 0)
	{
		return;
	}
	else
	{
		load_abir_data(workspace_vals.start_frame, workspace_vals.end_frame);
	}

	processing_state original = workspace_vals.all_states[0];
	if (original.replaced_pixels.size() > 0)
	{
		std::vector<unsigned int> bad_pixels = original.replaced_pixels;
		replace_bad_pixels(bad_pixels);
	}

	for (auto i = 1; i < workspace_vals.all_states.size(); i++)
	{
		processing_state current_state = workspace_vals.all_states[i];
		QString ANS_hide_shadow_str = "Hide Shadow";
		if (!current_state.ANS_hide_shadow){
			ANS_hide_shadow_str = "Show Shadow";
		}

		//QString ANS_hide_shadow_str = current_state.ANS_hide_shadow ? "Hide Shadow" : "Show Shadow";
		switch (current_state.method)
		{
			case Processing_Method::adaptive_noise_suppression:
				create_adaptive_noise_correction(current_state.ANS_relative_start_frame, current_state.ANS_num_frames, ANS_hide_shadow_str);
				break;

			case Processing_Method::deinterlace:
				create_deinterlace(current_state.deint_type);
				break;

			case Processing_Method::fixed_noise_suppression:
				fixed_noise_suppression(current_state.FNS_file_path, current_state.FNS_start_frame, current_state.FNS_stop_frame);
				break;

			default:
				QtHelpers::LaunchMessageBox(QString("Unexpected Workspace Behavior"), "Unexpected processing method in workspace, unable to proceed.");
		}
	}

	for (auto i = 0; i < workspace_vals.annotations.size(); i++)
	{
		annotation_info anno = workspace_vals.annotations[i];
		video_display->annotation_list.push_back(anno);
	}
}

void SirveApp::ui_choose_abp_file()
{
	QString file_selection = QFileDialog::getOpenFileName(this, ("Open File"), "", ("Image File(*.abpimage)"));
	int compare = QString::compare(file_selection, "", Qt::CaseInsensitive);
	if (compare == 0) {
		QtHelpers::LaunchMessageBox(QString("Issue Finding File"), "No file was selected.");
		return;
	}

	bool validated = validate_abp_files(file_selection);
	if (validated) {
		load_osm_data();
	}
};

bool SirveApp::validate_abp_files(QString path_to_image_file)
{
	AbpFileMetadata possible_abp_file_metadata = file_processor.locate_abp_files(path_to_image_file);

	if (!possible_abp_file_metadata.error_msg.isEmpty())
	{
		if (eng_data != NULL) {
			// if eng_data already initialized, allow user to re-select frames
			txt_start_frame->setEnabled(true);
			txt_end_frame->setEnabled(true);
			btn_get_frames->setEnabled(true);
			btn_calibration_dialog->setEnabled(true);
		}

		QtHelpers::LaunchMessageBox(QString("Issue Finding File"), possible_abp_file_metadata.error_msg);

		return false;
	}

	abp_file_metadata = possible_abp_file_metadata;

	return true;
};

void SirveApp::load_osm_data()
{
	osm_frames = osm_reader.read_osm_file(abp_file_metadata.osm_path);
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

	set_lift_and_gain(0, 1);

	if (eng_data != NULL)
	{
		slider_video->setValue(0);
		toggle_video_playback_options(false);

		// Reset video frame
		playback_controller->stop_timer();
		reset_color_correction();

		// delete objects with existing data within them
		delete eng_data;
		delete track_info;
		delete data_plots;
		delete engineering_plot_layout;

		video_display->container.clear_processing_states();
		video_display->remove_frame();
		histogram_plot->remove_histogram_plots();

		tab_menu->setTabEnabled(1, false);
		tab_menu->setTabEnabled(2, false);
		cmb_processing_states->setEnabled(false);
	}

	eng_data = new Engineering_Data(osm_frames);
	track_info = new TrackInformation(osm_frames);
	data_plots = new Engineering_Plots(osm_frames);

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
	update_epoch_string(create_epoch_string(epoch0));
	display_original_epoch(create_epoch_string(epoch0));

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
	connect(btn_popout_engineering, &QPushButton::clicked, this, &SirveApp::handle_popout_engineering_btn);
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
	video_display->initialize_toggles();

	// Reset setting engineering plot defaults
	menu_plot_all_data->setIconVisibleInMenu(true);

	menu_plot_primary->setIconVisibleInMenu(false);

	menu_plot_frame_marker->setIconVisibleInMenu(false);

	enable_engineering_plot_options();
	data_plots->set_plot_title(QString("EDIT CLASSIFICATION"));

	return;
}

void SirveApp::ui_load_abir_data()
{
	btn_get_frames->setEnabled(false);

	int min_frame = get_integer_from_txt_box(txt_start_frame->text());
	int max_frame = get_integer_from_txt_box(txt_end_frame->text());

	if (!verify_frame_selection(min_frame, max_frame)) {
		btn_get_frames->setEnabled(true);

		return;
	}

	load_abir_data(min_frame, max_frame);
}

void SirveApp::load_abir_data(int min_frame, int max_frame)
{
	QProgressDialog progress_dialog("Loading frames", "", 0, 4);
	progress_dialog.setWindowModality(Qt::ApplicationModal);
	progress_dialog.setWindowTitle("Overall ABIR Load Progress");
	progress_dialog.setCancelButton(nullptr);
	progress_dialog.setMinimumDuration(0);
	progress_dialog.setValue(1);

	// Load the ABIR data
	playback_controller->stop_timer();

	ABIR_Data_Result abir_data_result = file_processor.load_image_file(abp_file_metadata.image_path, min_frame, max_frame, config_values.version);

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

	video_details vid_details = {x_pixels, y_pixels, video_frames};

	processing_state primary = { Processing_Method::original, vid_details };
	video_display->container.clear_processing_states();
	video_display->container.add_processing_state(primary);

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
	std::vector<Plotting_Frame_Data> temp = eng_data->get_subset_plotting_frame_data(index0, index1);

	progress_dialog.setLabelText("Finalizing application state");
	progress_dialog.setValue(3);

	video_display->initialize_track_data(track_info->get_osm_frames(index0, index1), track_info->get_manual_frames(index0, index1));
	video_display->initialize_frame_data(min_frame, temp, file_processor.abir_data.ir_data);
	video_display->receive_video_data(x_pixels, y_pixels, number_frames);
	update_global_frame_vector();

	// Reset engineering plots with new sub plot indices
	data_plots->index_sub_plot_xmin = min_frame - 1;
	data_plots->index_sub_plot_xmax = max_frame - 1;
	data_plots->plot_all_data = false;
	menu_plot_all_data->setIconVisibleInMenu(false);
	data_plots->plot_current_marker = true;
	menu_plot_frame_marker->setIconVisibleInMenu(true);
	plot_change();

	//Update frame marker on engineering plot
	connect(playback_controller, &Playback::frame_selected, data_plots, &Engineering_Plots::plot_current_step);

	playback_controller->set_initial_speed_index(10);
	update_fps();

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

	toggle_video_playback_options(true);

	progress_dialog.setValue(4);
}

void SirveApp::handle_popout_engineering_btn(bool checked)
{
	if (checked)
	{
		open_popout_engineering_plot();
	}
	else
	{
		popout_engineering->close();
	}
}

void SirveApp::open_popout_engineering_plot()
{
	popout_engineering->acquire(data_plots->chart_view);
	connect(popout_engineering, &QDialog::finished, this, &SirveApp::popout_engineering_closed);
	popout_engineering->open();
}

void SirveApp::popout_engineering_closed()
{
	btn_popout_engineering->setChecked(false);
	engineering_plot_layout->addWidget(data_plots->chart_view);
	frame_plots->setLayout(engineering_plot_layout);
}

void SirveApp::handle_popout_histogram_btn(bool checked)
{
	if (checked) {
		open_popout_histogram_plot();
	}
	else
	{
		popout_histogram->close();
	}
}

void SirveApp::open_popout_histogram_plot()
{
	popout_histogram->acquire(histogram_plot->abs_chart_view);
	connect(popout_histogram, &QDialog::finished, this, &SirveApp::popout_histogram_closed);
	popout_histogram->open();
}

void SirveApp::popout_histogram_closed()
{
	btn_popout_histogram->setChecked(false);
	histogram_abs_layout->addWidget(histogram_plot->abs_chart_view);
}

void SirveApp::handle_popout_video_btn(bool checked)
{
	if (checked) {
		open_popout_video_display();
	}
	else {
		popout_video->close();
	}
}

void SirveApp::open_popout_video_display()
{
	video_display->label->disable();
	video_display->label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	popout_video->acquire(video_display->label);
	connect(popout_video, &QDialog::finished, this, &SirveApp::popout_video_closed);
	popout_video->open();
}

void SirveApp::popout_video_closed()
{
	video_display->label->enable();
	video_display->label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	btn_popout_video->setChecked(false);
	video_display->reclaim_label();
}

void SirveApp::start_stop_video_record()
{

	if (record_video)
	{
		//Stopping record video

		video_display->stop_recording();

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
		bool file_opened = video_display->start_recording(playback_controller->get_fps());

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


void SirveApp::toggle_zoom_on_video() {

	bool status_zoom_btn = btn_zoom->isChecked();

	if (status_zoom_btn)
	{
		video_display->toggle_action_zoom(true);
		btn_calculate_radiance->setChecked(false);
	}
	else {
		video_display->toggle_action_zoom(false);
	}

}


void SirveApp::toggle_calculation_on_video()
{

	bool status_calculation_btn = btn_calculate_radiance->isChecked();

	if (status_calculation_btn) {

		video_display->toggle_action_calculate_radiance(true);
		btn_zoom->setChecked(false);
	}
	else {
		video_display->toggle_action_calculate_radiance(false);
	}
}

void SirveApp::clear_zoom_and_calculation_buttons()
{
	btn_zoom->setChecked(false);
	btn_calculate_radiance->setChecked(false);
}

void SirveApp::update_fps()
{
	QString fps = QString::number(playback_controller->speeds[playback_controller->index_speed], 'g', 2);
	fps.append(" fps");

	lbl_fps->setText(fps);
}


void SirveApp::histogram_clicked(double x0, double x1) {
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

void SirveApp::handle_chk_auto_lift_gain(int state)
{
	if (state == Qt::Checked)
	{
		slider_lift->setEnabled(false);
		slider_gain->setEnabled(false);

		update_global_frame_vector();

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

void SirveApp::set_lift_and_gain(double lift, double gain)
{
	slider_lift->setValue(lift * 1000);
	lbl_lift_value->setText(QString::number(lift));

	slider_gain->setValue(gain * 1000);
	lbl_gain_value->setText(QString::number(gain));
}

void SirveApp::lift_slider_toggled() {
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

	update_global_frame_vector();
}

void SirveApp::gain_slider_toggled() {
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

	update_global_frame_vector();
}

void SirveApp::reset_color_correction()
{
	slider_lift->setValue(0);
	slider_gain->setValue(1000);
	chk_relative_histogram->setChecked(false);
}


void SirveApp::toggle_plot_full_data()
{
	data_plots->plot_all_data = !data_plots->plot_all_data;
	menu_plot_all_data->setIconVisibleInMenu(data_plots->plot_all_data);

	plot_change();
}

void SirveApp::toggle_plot_primary_only()
{
	data_plots->plot_primary_only = !data_plots->plot_primary_only;
	menu_plot_primary->setIconVisibleInMenu(data_plots->plot_primary_only);

	plot_change();
}

void SirveApp::toggle_plot_current_frame_marker()
{
	data_plots->plot_current_marker = !data_plots->plot_current_marker;
	menu_plot_frame_marker->setIconVisibleInMenu(data_plots->plot_current_marker);

	plot_change();
}

void SirveApp::auto_change_plot_display(int index)
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

void SirveApp::show_calibration_dialog()
{
	CalibrationDialog calibrate_dialog(calibration_model);

	auto response = calibrate_dialog.exec();

	if (response == 0) {

		return;
	}

	calibration_model = calibrate_dialog.model;
	video_display->set_calibration_model(calibrate_dialog.model);
	btn_calculate_radiance->setEnabled(true);
}

void SirveApp::set_data_timing_offset()
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

		std::vector<Plotting_Frame_Data> temp = eng_data->get_subset_plotting_frame_data(index0, index1);
		video_display->update_frame_data(temp);

		plot_change();
	}
}

void SirveApp::close_window()
{
	close();
}

void SirveApp::save_plot()
{
	data_plots->save_plot();
}

void SirveApp::save_frame()
{
	if(playback_controller->is_running())
		playback_controller->stop_timer();

	video_display->save_frame();

	if(playback_controller->is_running())
		playback_controller->start_timer();
}

void SirveApp::create_menu_actions()
{
	QIcon on("icons/check.png");

	action_close = new QAction("Close");
	action_close->setStatusTip("Close main window");
	connect(action_close, &QAction::triggered, this, &SirveApp::close_window);

	action_set_timing_offset = new QAction("Set Timing Offset");
	action_set_timing_offset->setStatusTip("Set a time offset to apply to collected data");
	connect(action_set_timing_offset, &QAction::triggered, this, &SirveApp::set_data_timing_offset);

	menu_file = menuBar()->addMenu(tr("&File"));
	menu_file->addAction(action_close);

	menu_settings = menuBar()->addMenu(tr("&Settings"));
	menu_settings->addAction(action_set_timing_offset);


	// ------------------------- PLOT MENU ACTIONS -------------------------

	menu_plot_all_data = new QAction(tr("&Plot all frame data"), this);
	menu_plot_all_data->setIcon(on);
	menu_plot_all_data->setStatusTip(tr("Plot all data from OSM file"));
	menu_plot_all_data->setIconVisibleInMenu(true);
	connect(menu_plot_all_data, &QAction::triggered, this, &SirveApp::toggle_plot_full_data);

	menu_plot_primary = new QAction(tr("&Plot Primary Data Only"), this);
	menu_plot_primary->setIcon(on);
	menu_plot_primary->setStatusTip(tr("Plot only the primary object"));
	menu_plot_primary->setIconVisibleInMenu(false);
	connect(menu_plot_primary, &QAction::triggered, this, &SirveApp::toggle_plot_primary_only);

	menu_plot_frame_marker = new QAction(tr("&Plot Marker for Current Frame"), this);
	menu_plot_frame_marker->setIcon(on);
	menu_plot_frame_marker->setStatusTip(tr("Plot marker to show current video frame"));
	menu_plot_frame_marker->setIconVisibleInMenu(false);
	connect(menu_plot_frame_marker, &QAction::triggered, this, &SirveApp::toggle_plot_current_frame_marker);

	menu_plot_edit_banner = new QAction(tr("&Edit Banner Text"), this);
	menu_plot_edit_banner->setStatusTip(tr("Edit the banner text for the plot"));
	connect(menu_plot_edit_banner, &QAction::triggered, this, &SirveApp::edit_plot_text);

	menu_plot_edit_banner = new QAction(tr("&Export Tracking Data"), this);
	menu_plot_edit_banner->setStatusTip(tr("Export the plotted data to file"));
	connect(menu_plot_edit_banner, &QAction::triggered, this, &SirveApp::export_plot_data);

	// ---------------------- Set Acctions to Menu --------------------

	plot_menu = new QMenu(this);
	plot_menu->addAction(menu_plot_all_data);
	plot_menu->addAction(menu_plot_primary);
	plot_menu->addAction(menu_plot_frame_marker);
	plot_menu->addAction(menu_plot_edit_banner);

	btn_plot_menu->setMenu(plot_menu);

}

void SirveApp::edit_banner_text()
{
	bool ok;
	QString input_text = QInputDialog::getText(0, "Banner Text", "Input Banner Text", QLineEdit::Normal, video_display->banner_text, &ok);

	if (!ok)
	{
		return;
	}

	video_display->update_banner_text(input_text);

	// checks if banners are the same and asks user if they want them to be the same
	QString plot_banner_text = data_plots->title;
	int check = QString::compare(input_text, plot_banner_text, Qt::CaseSensitive);
	if (check != 0)
	{
		auto response = QtHelpers::LaunchYesNoMessageBox("Update All Banners", "Video and plot banners do not match. Would you like to set both to the same banner?");
		if (response == QMessageBox::Yes)
		{
			data_plots->set_plot_title(input_text);
		}
	}
}

void SirveApp::edit_plot_text()
{
	bool ok;
	QString input_text = QInputDialog::getText(0, "Plot Header Text", "Input Plot Header Text", QLineEdit::Normal, data_plots->title, &ok);

	if (ok)
	{
		data_plots->set_plot_title(input_text);
	}
}

void SirveApp::export_plot_data()
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
		DataExport::write_track_data_to_csv(save_path, eng_data->get_plotting_frame_data(), track_info->get_plotting_tracks(), track_info->get_manual_plotting_tracks());
	}
	else {
		min_frame = data_plots->index_sub_plot_xmin;
		max_frame = data_plots->index_sub_plot_xmax;

		DataExport::write_track_data_to_csv(save_path, eng_data->get_plotting_frame_data(), track_info->get_plotting_tracks(), track_info->get_manual_plotting_tracks(), min_frame, max_frame);
	}

	QMessageBox msgBox;
	msgBox.setWindowTitle("Export Data");
	msgBox.setText("Successfully exported track data to file");
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.exec();
}

int SirveApp::get_color_index(QVector<QString> colors, QColor input_color) {

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


void SirveApp::edit_color_map()
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
			video_display->update_color_map(video_colors.maps[i].colors);
			color_map_display->set_color_map(video_colors.maps[i].colors);
			return;
		}
	}
}

void SirveApp::edit_banner_color()
{
	QString color = cmb_text_color->currentText();
	video_display->update_banner_color(color);
}

void SirveApp::edit_tracker_color()
{
	QString tracker_color = cmb_tracker_color->currentText();
	video_display->update_tracker_color(tracker_color);
}

void SirveApp::plot_change()
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
		data_plots->toggle_yaxis_scientific(scientific_is_checked);

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

		data_plots->set_yaxis_chart_id(y_index);
		data_plots->plot();

		data_plots->plot_current_step(playback_controller->get_current_frame_number());
	}

}

void SirveApp::annotate_video()
{
	video_info standard_info;
	standard_info.x_pixels = video_display->image_x;
	standard_info.y_pixels = video_display->image_y;

	standard_info.min_frame = data_plots->index_sub_plot_xmin + 1;
	standard_info.max_frame = data_plots->index_sub_plot_xmax + 1;

	AnnotationListDialog annotate_gui(video_display->annotation_list, standard_info);
	connect(&annotate_gui, &AnnotationListDialog::annotation_list_updated, video_display, &VideoDisplay::handle_annotation_changes);
	annotate_gui.exec();
}

int SirveApp::get_integer_from_txt_box(QString input)
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

void SirveApp::copy_osm_directory()
{
	clipboard->setText(abp_file_metadata.osm_path);
}

void SirveApp::toggle_relative_histogram(bool input)
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

void SirveApp::apply_epoch_time()
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
	update_epoch_string(create_epoch_string(epoch));
	// --------------------------------------------------------------------

	epoch_jdate = jtime::JulianDate(year, month, day, hr, min, sec);
	eng_data->update_epoch_time(epoch_jdate);

	data_plots->past_epoch = eng_data->get_seconds_from_epoch();
	plot_change();
}

void SirveApp::ui_replace_bad_pixels()
{
	auto response = QtHelpers::LaunchYesNoMessageBox("Bad Pixel Confirmation", "Replacing bad pixels will reset all filters and modify the original frame. Are you sure you want to continue?");

	if (response == QMessageBox::Yes) {
		ABIR_Data_Result abir_first_50_frames = file_processor.load_image_file(abp_file_metadata.image_path, 1, 50, config_values.version);

		if (abir_first_50_frames.had_error) {
			QtHelpers::LaunchMessageBox(QString("Error Reading ABIR Frames"), "Error reading first 50 frames from .abpimage file, cannot identify bad pixels.");
			return;
		}

		std::vector<unsigned int> dead_pixels = BadPixels::identify_dead_pixels(abir_first_50_frames.video_frames_16bit);
		replace_bad_pixels(dead_pixels);
	}
}

void SirveApp::receive_new_bad_pixels(std::vector<unsigned int> new_pixels)
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
			replace_bad_pixels(bad_pixels);
		}
	}
}

void SirveApp::replace_bad_pixels(std::vector<unsigned int> & pixels_to_replace)
{
	processing_state base_state = video_display->container.processing_states[0];
	base_state.replaced_pixels = pixels_to_replace;
	BadPixels::replace_pixels_with_neighbors(base_state.details.frames_16bit, pixels_to_replace, base_state.details.x_pixels);

	video_display->container.clear_processing_states();
	video_display->container.add_processing_state(base_state);

	lbl_bad_pixel_count->setText("Bad pixels currently replaced: " + QString::number(pixels_to_replace.size()));
	chk_highlight_bad_pixels->setEnabled(true);
}

void SirveApp::receive_new_good_pixels(std::vector<unsigned int> pixels)
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
			load_abir_data(min_frame, max_frame);

			replace_bad_pixels(bad_pixels);
		}
	}
}

void SirveApp::fixed_noise_suppression_from_external_file()
{
	ExternalNUCInformationWidget external_nuc_dialog;

	auto response = external_nuc_dialog.exec();

	if (response == 0) {

		return;
	}

	QString image_path = external_nuc_dialog.abp_metadata.image_path;
	unsigned int min_frame = external_nuc_dialog.start_frame;
	unsigned int max_frame = external_nuc_dialog.stop_frame;

	try
	{
		// assumes file version is same as base file opened
		fixed_noise_suppression(image_path, min_frame, max_frame);
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

void SirveApp::ui_execute_non_uniformity_correction_selection_option()
{
	QStringList options;
	options << tr("From Current File") << tr("From External File");

	QStringList shadow_options;
	shadow_options << tr("Hide Shadow") << tr("Show Shadow");

	QString hide_shadow_choice = "Hide Shadow";

	bool ok;
	QString item = QInputDialog::getItem(this, "Fixed Mean Noise Suppression", "Options", options, 0, false, &ok);
	//Pause the video if it's running
	playback_controller->stop_timer();

	processing_state original = video_display->container.copy_current_state();
	int number_frames = static_cast<int>(original.details.frames_16bit.size());
	if (!ok)
		return;

	if (item == "From Current File")
	{
		int delta_frames = data_plots->index_sub_plot_xmax - data_plots->index_sub_plot_xmin;
		int start_frame = QInputDialog::getInt(this, "Fixed Noise Suppression", "Start frame", 1, -delta_frames, delta_frames, 1, &ok);
		if (!ok)
			return;

		int number_of_frames = QInputDialog::getInt(this, "Fixed Noise Suppresssion", "Number of frames to use for suppression", 1, 1, number_frames, 1, &ok);
		if (!ok)
			return;

		create_fixed_noise_correction(start_frame, number_of_frames, hide_shadow_choice);
	}
	else
	{
		fixed_noise_suppression_from_external_file();
	}

}

void SirveApp::fixed_noise_suppression(QString file_path, unsigned int min_frame, unsigned int max_frame)
{
	if (!verify_frame_selection(min_frame, max_frame))
	{
		QtHelpers::LaunchMessageBox(QString("Invalid Frame Selection"), "NUC correction not completed, invalid frame selection");
		return;
	}

	processing_state original = video_display->container.copy_current_state();

	processing_state noise_suppression_state = original;
	noise_suppression_state.details.frames_16bit.clear();
	int number_frames = static_cast<int>(original.details.frames_16bit.size());

	FixedNoiseSuppressionExternal FNS;
	std::vector<std::vector<double>> fixed_correction = FNS.get_correction(abp_file_metadata.image_path, min_frame, max_frame, number_frames, config_values.version);

	if (fixed_correction.size() == 0)
	{
		QtHelpers::LaunchMessageBox(QString("File Version Not Within Range"), "File version was not within valid range. See log for more details");

		return;
	}

	QProgressDialog progress("", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.setWindowTitle(QString("Fixed Noise Suppression"));
	progress.setMinimum(0);
	progress.setMaximum(number_frames - 1);
	progress.setLabelText(QString("Applying correction..."));
	progress.setMinimumWidth(300);

	QString hide_shadow = "Hide Shadow";
	for (auto i = 0; i < number_frames; i++) {
		progress.setValue(i);

		noise_suppression_state.details.frames_16bit.push_back(ApplyCorrection::apply_correction(original.details.frames_16bit[i], fixed_correction[i], hide_shadow));;
		if (progress.wasCanceled())
			break;
	}


	progress.setLabelText(QString("Down-converting video and creating histogram data..."));

	noise_suppression_state.method = Processing_Method::fixed_noise_suppression;
	noise_suppression_state.FNS_file_path = file_path;
	noise_suppression_state.FNS_start_frame = min_frame;
	noise_suppression_state.FNS_stop_frame = max_frame;
	video_display->container.add_processing_state(noise_suppression_state);

	QFileInfo fi(file_path);
	QString fileName = fi.fileName().toLower();
	QString current_filename = abp_file_metadata.file_name.toLower() + ".abpimage";

	if (fileName == current_filename)
		fileName = "Current File";

	QString description = "File: " + fileName + "\n";
	description += "From frame " + QString::number(min_frame) + " to " + QString::number(max_frame);

	lbl_fixed_suppression->setText(description);
}

void SirveApp::ui_execute_deinterlace()
{
	deinterlace_type deinterlace_method_type = static_cast<deinterlace_type>(cmb_deinterlace_options->currentIndex());

	create_deinterlace(deinterlace_method_type);
}

void SirveApp::create_deinterlace(deinterlace_type deinterlace_method_type)
{
	processing_state original = video_display->container.copy_current_state();

	Deinterlace deinterlace_method(deinterlace_method_type, original.details.x_pixels, original.details.y_pixels);

	processing_state deinterlace_state = original;
	deinterlace_state.details.frames_16bit.clear();

	// Apply de-interlace to the frames

	int number_frames = static_cast<int>(original.details.frames_16bit.size());

	QProgressDialog progress("", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.setWindowTitle(QString("De-interlace Frames"));
	progress.setMaximum(number_frames - 1);
	progress.setLabelText(QString("Creating de-interlaced frames..."));
	progress.setMinimumWidth(300);

	for (int i = 0; i < number_frames; i++)
	{
		progress.setValue(i);

		deinterlace_state.details.frames_16bit.push_back(deinterlace_method.deinterlace_frame(original.details.frames_16bit[i]));
		if (progress.wasCanceled())
			break;
	}

	if (progress.wasCanceled())
	{
		return;
	}

	deinterlace_state.method = Processing_Method::deinterlace;
	deinterlace_state.deint_type = deinterlace_method_type;
	video_display->container.add_processing_state(deinterlace_state);
}

void SirveApp::toggle_osm_tracks()
{
	bool current_status = video_display->plot_tracks;
	video_display->toggle_osm_tracks(!current_status);

	if (!current_status) {
		cmb_tracker_color->setEnabled(true);
	}
	else
	{
		cmb_tracker_color->setEnabled(false);
	}
}

void SirveApp::handle_new_processing_state(QString state_name, int index)
{
	cmb_processing_states->addItem(state_name);
	cmb_processing_states->setCurrentIndex(index);
}

void SirveApp::handle_processing_state_removal(Processing_Method method, int index)
{
	cmb_processing_states->removeItem(index);

	if (method == Processing_Method::adaptive_noise_suppression)
	{
		label_adaptive_noise_suppression_status->setText("No Frames Setup");
	}
	else if (method == Processing_Method::fixed_noise_suppression)
	{
		lbl_fixed_suppression->setText("No Frames Selected");
	}
}

void SirveApp::handle_cleared_processing_states()
{
	cmb_processing_states->clear();
	label_adaptive_noise_suppression_status->setText("No Frames Setup");
	lbl_fixed_suppression->setText("No Frames Selected");
}

void SirveApp::ui_execute_noise_suppression()
{
	//-----------------------------------------------------------------------------------------------
	// get user selected frames for suppression

	int delta_frames = data_plots->index_sub_plot_xmax - data_plots->index_sub_plot_xmin;

	QStringList shadow_options;
	shadow_options << tr("Hide Shadow") << tr("Show Shadow");

	bool ok;
	int relative_start_frame = QInputDialog::getInt(this, "Adaptive Noise Suppression", "Relative start frame", -5, -delta_frames, delta_frames, 1, &ok);
	if (!ok)
		return;

	int number_of_frames = QInputDialog::getInt(this, "Adaptive Noise Suppresssion", "Number of frames to use for suppression", 5, 1, std::abs(relative_start_frame), 1, &ok);
	if (!ok)
		return;

	QString hide_shadow_choice = QInputDialog::getItem(this, "Adaptive Noise Suppression", "Options", shadow_options, 0, false, &ok);
	if (!ok)
		return;

	create_adaptive_noise_correction(relative_start_frame, number_of_frames, hide_shadow_choice);
}


void SirveApp::create_fixed_noise_correction(int start_frame, int num_frames, QString hide_shadow_choice)
{
	//bool hide_shadow_bool = hide_shadow_choice == "Hide Shadow";

	//Pause the video if it's running
	playback_controller->stop_timer();

	processing_state original = video_display->container.copy_current_state();
	int number_frames = static_cast<int>(original.details.frames_16bit.size());

	QProgressDialog progress_dialog("Creating adjustment", "Cancel", 0, number_frames * 2 + 2);
	progress_dialog.setWindowTitle("Fixed Noise Correction");
	progress_dialog.setWindowModality(Qt::ApplicationModal);
	progress_dialog.setMinimumDuration(0);
	progress_dialog.setValue(1);

	std::vector<std::vector<double>> noise_suppression = FixedNoiseSuppression::get_correction(start_frame, num_frames, original.details, progress_dialog);

	if (noise_suppression.size() == 0) {
		return;
	}
	progress_dialog.setValue(number_frames);
	progress_dialog.setLabelText("Adjusting frames and copying data");

	processing_state noise_suppression_state = original;
	noise_suppression_state.details.frames_16bit.clear();

	for (auto i = 0; i < number_frames; i++) {
		progress_dialog.setValue(number_frames + 1 + i);
		noise_suppression_state.details.frames_16bit.push_back(ApplyCorrection::apply_correction(original.details.frames_16bit[i], noise_suppression[i], hide_shadow_choice));
		if (progress_dialog.wasCanceled())
		{
			return;
		}
	}
	progress_dialog.setLabelText("Finalizing corrections");
	progress_dialog.setValue(number_frames * 2 + 1);

	QString description = "Filter starts at ";
	if (start_frame > 0)
		description += "+";

	noise_suppression_state.method = Processing_Method::fixed_noise_suppression;
	noise_suppression_state.FNS_start_frame = data_plots->index_sub_plot_xmin + start_frame;
	noise_suppression_state.FNS_stop_frame = data_plots->index_sub_plot_xmin + start_frame + num_frames - 1;
	noise_suppression_state.FNS_file_path = abp_file_metadata.directory_path;
	video_display->container.add_processing_state(noise_suppression_state);

	chk_auto_lift_gain->setChecked(true);
	progress_dialog.setValue(number_frames * 2 + 2);
}



void SirveApp::create_adaptive_noise_correction(int relative_start_frame, int num_frames, QString hide_shadow_choice)
{
	//Pause the video if it's running
	playback_controller->stop_timer();

	processing_state original = video_display->container.copy_current_state();
	int number_frames = static_cast<int>(original.details.frames_16bit.size());

	QProgressDialog progress_dialog("Creating frame-by-frame adjustment", "Cancel", 0, number_frames * 2 + 2);
	progress_dialog.setWindowTitle("Adaptive Noise Suppression");
	progress_dialog.setWindowModality(Qt::ApplicationModal);
	progress_dialog.setMinimumDuration(0);
	progress_dialog.setValue(1);

	std::vector<std::vector<double>> noise_suppression = AdaptiveNoiseSuppression::get_correction(relative_start_frame, num_frames, original.details, progress_dialog);

	if (noise_suppression.size() == 0) {
		return;
	}
	progress_dialog.setValue(number_frames);
	progress_dialog.setLabelText("Adjusting frames and copying data");

	processing_state noise_suppression_state = original;
	noise_suppression_state.details.frames_16bit.clear();

	for (auto i = 0; i < number_frames; i++) {
		progress_dialog.setValue(number_frames + 1 + i);
		noise_suppression_state.details.frames_16bit.push_back(ApplyCorrection::apply_correction(original.details.frames_16bit[i], noise_suppression[i], hide_shadow_choice));
		if (progress_dialog.wasCanceled())
		{
			return;
		}
	}
	progress_dialog.setLabelText("Finalizing adaptive noise suppression");
	progress_dialog.setValue(number_frames * 2 + 1);

	QString description = "Filter starts at ";
	if (relative_start_frame > 0)
		description += "+";

	label_adaptive_noise_suppression_status->setWordWrap(true);
	description += QString::number(relative_start_frame) + " frames and averages " + QString::number(num_frames) + " frames";

	label_adaptive_noise_suppression_status->setText(description);

	bool hide_shadow_bool = hide_shadow_choice == "Hide Shadow";

	noise_suppression_state.method = Processing_Method::adaptive_noise_suppression;
	noise_suppression_state.ANS_relative_start_frame = relative_start_frame;
	noise_suppression_state.ANS_num_frames = num_frames;
	noise_suppression_state.ANS_hide_shadow = hide_shadow_bool;
	video_display->container.add_processing_state(noise_suppression_state);

	chk_auto_lift_gain->setChecked(true);
	progress_dialog.setValue(number_frames * 2 + 2);
}

void SirveApp::toggle_video_playback_options(bool input)
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
		playback_controller->stop_timer();
		lbl_fps->setText("");
	}
}

void SirveApp::enable_engineering_plot_options()
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

void SirveApp::update_epoch_string(QString new_epoch_string)
{

	QString out = "Applied Epoch: ";
	out = out +new_epoch_string;
	lbl_current_epoch->setText(out);

}

void SirveApp::display_original_epoch(QString new_epoch_string)
{
	QString out = "Original Epoch: ";
	out = out + new_epoch_string;
	lbl_current_epoch->setToolTip(out);
}

QString SirveApp::create_epoch_string(std::vector<double> new_epoch) {

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

bool SirveApp::verify_frame_selection(int min_frame, int max_frame)
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

void SirveApp::handle_frame_change()
{
	update_global_frame_vector();
}

void SirveApp::handle_frame_number_change(unsigned int new_frame_number)
{
	video_display->view_frame(new_frame_number);
	update_global_frame_vector();
}

void SirveApp::update_global_frame_vector()
{
	std::vector<double> original_frame_vector = {video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[video_display->counter].begin(),
		video_display->container.processing_states[video_display->container.current_idx].details.frames_16bit[video_display->counter].end()};

	//Convert current frame to armadillo matrix
	arma::vec image_vector(original_frame_vector);

	//Normalize the image to values between 0 - 1
	int max_value = std::pow(2, config_values.max_used_bits);
	image_vector = image_vector / max_value;

	if (image_vector.max() != 1) {
		double sigma = arma::stddev(image_vector);
		double meanVal = arma::mean(image_vector);
		image_vector = image_vector / (meanVal + 3. * sigma) - .5;
	}

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
		set_lift_and_gain(lift, gain);
	}

	double lift = slider_lift->value() / 1000.;
	double gain = slider_gain->value() / 1000.;

	histogram_plot->update_histogram_abs_plot(image_vector, lift, gain);

	// Correct image based on min/max value inputs
	ColorCorrection::update_color(image_vector, lift, gain);

	histogram_plot->update_histogram_rel_plot(image_vector);

	image_vector = image_vector * 255;
	//arma::vec out_frame_flat = arma::vectorise(image_vector);
	std::vector<double>out_vector = arma::conv_to<std::vector<double>>::from(image_vector);
	std::vector<uint8_t> display_ready_converted_values = {out_vector.begin(), out_vector.end()};

	video_display->update_frame_vector(original_frame_vector, display_ready_converted_values);
}
