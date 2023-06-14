#include "SirveApp.h"

SirveApp::SirveApp(QWidget *parent)
	: QMainWindow(parent)
{
	
	INFO << "GUI: Initializing GUI";

	config_values = configreader::load();

	// establish object that will hold video and connect it to the playback thread
	video_display = new VideoDisplay(1, 1, config_values.max_used_bits);
	video_display->moveToThread(&thread_video);
	//QObject::connect(&thread_video, &QThread::started, video_display, &Video::update_display_frame);

	setup_ui();

	//---------------------------------------------------------------------------

	// establish object to control playback timer and move to a new thread
	playback_controller = new Playback(1);
	playback_controller->moveToThread(&thread_timer);
	QObject::connect(&thread_video, &QThread::started, playback_controller, &Playback::start_timer);

	// establish copy copy 
	clipboard = QApplication::clipboard();
	QObject::connect(btn_copy_directory, &QPushButton::clicked, this, &SirveApp::copy_osm_directory);

	//---------------------------------------------------------------------------
	// setup container to store all videos
	eng_data = NULL;

	// default recording video to false
	record_video = false;

	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------

	// links label showing the video to the video frame
	video_layout = new QGridLayout();
	video_layout->addWidget(video_display->label);
	frame_video->setLayout(video_layout);

	
	// links chart with frame where it will be contained
	QVBoxLayout *histogram_layout = new QVBoxLayout();
	histogram_layout->addWidget(video_display->histogram_plot->rel_chart_view);
	frame_histogram->setLayout(histogram_layout);

	// links chart with frame where it will be contained
	QVBoxLayout *histogram_abs_layout = new QVBoxLayout();
	histogram_abs_layout->addWidget(video_display->histogram_plot->chart_view);
	frame_histogram_abs->setLayout(histogram_abs_layout);	
	
	// links chart with frame where it will be contained
	QVBoxLayout *histogram_abs_layout_full = new QVBoxLayout();
	histogram_abs_layout_full->addWidget(video_display->histogram_plot->chart_full_view);
	frame_histogram_abs_full->setLayout(histogram_abs_layout_full);

	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------

	// setup color correction class based on slider values
	int max_lift, min_lift, max_gain, min_gain;
	color_correction.get_min_slider_range(min_lift, max_lift);
	color_correction.get_max_slider_range(min_gain, max_gain);
	
	slider_lift->setMinimum(min_lift);
	slider_lift->setMaximum(max_lift);
	slider_gain->setMinimum(min_gain);
	slider_gain->setMaximum(max_gain);

	reset_color_correction();

	// establish connections to all qwidgets	
	setup_connections();
	
	toggle_relative_histogram(false);
	toggle_video_playback_options(false);
	enable_engineering_plot_options();

	create_menu_actions();

	this->resize(0, 0);
		
	INFO << "GUI: GUI Initialized";
}

SirveApp::~SirveApp() {

	
	delete video_display;
	delete playback_controller;
	//delete histogram_plot;

	//delete video_display;
	//delete histogram_plot;
	//delete data_plots;
	
	delete eng_data;

	thread_video.terminate();
	thread_timer.terminate();

	DEBUG << "GUI: GUI destructor called";

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
	
	btn_workspace_save->setEnabled(false);
	btn_workspace_load->setEnabled(true);
	cmb_processing_states->setEnabled(false);
	// ------------------------------------------------------------------------

	this->setCentralWidget(frame_main);
	this->show();

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
	
	QFrame* horizontal_segment1 = new QFrame();
	horizontal_segment1->setFrameShape(QFrame::HLine);

	QGridLayout* grid_import_file = new QGridLayout();
	grid_import_file->addWidget(lbl_file_name, 0, 0, 1, 2);
	grid_import_file->addWidget(btn_load_osm, 1, 0);
	grid_import_file->addWidget(btn_copy_directory, 1, 1);
	grid_import_file->addWidget(btn_calibration_dialog, 2, 0, 1, 2);
	grid_import_file->addWidget(horizontal_segment1, 3, 0, 1, 2);

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

	QFrame* horizontal_segment2 = new QFrame();
	horizontal_segment2->setFrameShape(QFrame::HLine);

	QGridLayout* grid_tab_import_frames = new QGridLayout();
	grid_tab_import_frames->addWidget(label_start_frame, 0, 0);
	grid_tab_import_frames->addWidget(label_stop_frame, 0, 1);
	grid_tab_import_frames->addWidget(txt_start_frame, 1, 0);
	grid_tab_import_frames->addWidget(txt_end_frame, 1, 1);
	grid_tab_import_frames->addWidget(lbl_max_frames, 2, 0, 1, 2);
	grid_tab_import_frames->addWidget(btn_get_frames, 3, 0, 1, 2);
	grid_tab_import_frames->addWidget(horizontal_segment2, 4, 0, 1, 2);

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

	QFrame* horizontal_segment3 = new QFrame();
	horizontal_segment3->setFrameShape(QFrame::HLine);

	QVBoxLayout* vlayout_tab_import_epoch = new QVBoxLayout();
	vlayout_tab_import_epoch->addWidget(label_epoch);
	vlayout_tab_import_epoch->addWidget(dt_epoch);
	vlayout_tab_import_epoch->addWidget(label_date_format);
	vlayout_tab_import_epoch->addWidget(lbl_current_epoch);
	vlayout_tab_import_epoch->addWidget(btn_apply_epoch);
	vlayout_tab_import_epoch->addWidget(horizontal_segment3);

	vlayout_tab_import->addLayout(vlayout_tab_import_epoch);

	// ------------------------------------------------------------------------
	lbl_file_load = new QLabel("File Load Status: ");
	lbl_file_load->setFrameShape(QFrame::Box);
	lbl_file_load->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	lbl_file_load->setMinimumHeight(100);

	vlayout_tab_import->addWidget(lbl_file_load);
	vlayout_tab_import->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout

	return widget_tab_import;
}

QWidget* SirveApp::setup_color_correction_tab() {

	QWidget* widget_tab_color = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_color = new QVBoxLayout(widget_tab_color);
	
	QWidget* widget_tab_color_sliders = new QWidget(tab_menu);
	QWidget* widget_tab_color_controls = new QWidget(tab_menu);
	QLabel* label_lift = new QLabel("Dark \nSet Point");
	QLabel* label_gain = new QLabel("Light \nSet Point");
	lbl_lift_value = new QLabel("0.0");
	lbl_gain_value = new QLabel("1.0");
	slider_lift = new QSlider();
	slider_gain = new QSlider();
	chk_relative_histogram = new QCheckBox("Relative Histogram");
	btn_reset_color_correction = new QPushButton("Reset Set Points");

	QFrame* horizontal_segment4 = new QFrame();
	horizontal_segment4->setFrameShape(QFrame::HLine);

	// Set attributes for all qwidgets
	label_lift->setToolTip("Dark Set Point pushes the image darker");
	label_gain->setToolTip("Light Set Point pushes the image lighter");
	slider_lift->setOrientation(Qt::Horizontal);
	slider_lift->setMinimum(0);
	slider_lift->setMaximum(1000);
	slider_lift->setSingleStep(1);
	slider_lift->setPageStep(10);
	slider_lift->setValue(0);
	slider_lift->setTickPosition(QSlider::TicksAbove);
	slider_lift->setTickInterval(100);

	slider_gain->setOrientation(Qt::Horizontal);
	slider_gain->setMinimum(0);
	slider_gain->setMaximum(1000);
	slider_gain->setSingleStep(1);
	slider_gain->setPageStep(10);
	slider_gain->setValue(1000);
	slider_gain->setTickPosition(QSlider::TicksAbove);
	slider_gain->setTickInterval(100);

	// End set attributes 

	// define layouts
	QGridLayout* grid_tab_color_sliders = new QGridLayout(tab_menu);
	QHBoxLayout* hlayout_tab_color_controls = new QHBoxLayout(tab_menu);

	// add widgets to layouts
	grid_tab_color_sliders->addWidget(label_lift, 0, 0);
	grid_tab_color_sliders->addWidget(slider_lift, 0, 1);
	grid_tab_color_sliders->addWidget(lbl_lift_value, 0, 2);
	grid_tab_color_sliders->addWidget(label_gain, 1, 0);
	grid_tab_color_sliders->addWidget(slider_gain, 1, 1);
	grid_tab_color_sliders->addWidget(lbl_gain_value, 1, 2);

	hlayout_tab_color_controls->addWidget(chk_relative_histogram);
	hlayout_tab_color_controls->addWidget(btn_reset_color_correction);

	widget_tab_color_sliders->setLayout(grid_tab_color_sliders);
	widget_tab_color_controls->setLayout(hlayout_tab_color_controls);

	vlayout_tab_color->addWidget(widget_tab_color_sliders);
	vlayout_tab_color->addWidget(widget_tab_color_controls);
	vlayout_tab_color->addWidget(horizontal_segment4);


	 // --------------------------------------------------------------------------

	QGridLayout* grid_tracker = new QGridLayout(widget_tab_color);
	QHBoxLayout* hlayout_osm_tracks = new QHBoxLayout(widget_tab_color);
	QHBoxLayout* hlayout_primary_track = new QHBoxLayout(widget_tab_color);
	QHBoxLayout* hlayout_text_color = new QHBoxLayout(widget_tab_color);
	QHBoxLayout* hlayout_color_map = new QHBoxLayout(widget_tab_color);

	QLabel* lbl_colormap = new QLabel("Set Colormap:");
	QLabel* lbl_primary_track = new QLabel("Set Primary Track Color:");
	QLabel* lbl_text_color = new QLabel("Set Text Color:");

	chk_show_tracks = new QCheckBox("Show OSM Tracks");
	chk_primary_track_data = new QCheckBox("Show Primary Track Info");
	chk_sensor_track_data = new QCheckBox("Show Sensor Info");
	chk_show_time = new QCheckBox("Show Zulu Time");

	btn_change_banner_text = new QPushButton("Change Banner Text");
	btn_add_annotations = new QPushButton("Add/Edit Annotations");

	QFrame* horizontal_segment5 = new QFrame();
	QFrame* horizontal_segment6 = new QFrame();
	QFrame* horizontal_segment7 = new QFrame();
	QFrame* horizontal_segment8 = new QFrame();
	horizontal_segment5->setFrameShape(QFrame::HLine);
	horizontal_segment6->setFrameShape(QFrame::HLine);
	horizontal_segment7->setFrameShape(QFrame::HLine);
	horizontal_segment8->setFrameShape(QFrame::HLine);
	
	cmb_color_maps = new QComboBox();
	int number_maps = video_display->video_colors.maps.size();
	for (int i = 0; i < number_maps; i++)
		cmb_color_maps->addItem(video_display->video_colors.maps[i].name);
	
	QList<QString> colors{};
	colors.append("red");
	colors.append("orange");
	colors.append("yellow");
	colors.append("green");
	colors.append("blue");
	colors.append("violet");
	colors.append("black");
	colors.append("white");

	cmb_tracker_color = new QComboBox();
	cmb_text_color = new QComboBox();
	cmb_primary_tracker_color = new QComboBox();
	
	cmb_tracker_color->addItems(colors);
	cmb_text_color->addItems(colors);
	cmb_primary_tracker_color->addItems(colors);

	cmb_tracker_color->setEnabled(false);
	cmb_primary_tracker_color->setEnabled(false);

	// --------------------------------------------------------------------------
	hlayout_osm_tracks->addWidget(chk_show_tracks);
	hlayout_osm_tracks->addStretch();
	hlayout_osm_tracks->addWidget(cmb_tracker_color);	
	
	hlayout_primary_track->addWidget(lbl_primary_track);
	hlayout_primary_track->addStretch(); 
	hlayout_primary_track->addWidget(cmb_primary_tracker_color);
	
	hlayout_text_color->addWidget(lbl_text_color);
	hlayout_text_color->addStretch();
	hlayout_text_color->addWidget(cmb_text_color);
	
	hlayout_color_map->addWidget(lbl_colormap);
	hlayout_color_map->addStretch();
	hlayout_color_map->addWidget(cmb_color_maps);
	
	vlayout_tab_color->addLayout(hlayout_osm_tracks);
	vlayout_tab_color->addLayout(hlayout_primary_track);
	vlayout_tab_color->addWidget(horizontal_segment5);
	vlayout_tab_color->addWidget(chk_primary_track_data);
	vlayout_tab_color->addWidget(chk_sensor_track_data);
	vlayout_tab_color->addWidget(chk_show_time);
	vlayout_tab_color->addLayout(hlayout_text_color);
	vlayout_tab_color->addWidget(btn_change_banner_text);
	vlayout_tab_color->addWidget(horizontal_segment6);
	vlayout_tab_color->addLayout(hlayout_color_map);
	vlayout_tab_color->addWidget(horizontal_segment7);
	vlayout_tab_color->addWidget(btn_add_annotations);

	vlayout_tab_color->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout


	return widget_tab_color;
}

QWidget* SirveApp::setup_filter_tab() {

	QWidget* widget_tab_processing = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_processing = new QVBoxLayout(widget_tab_processing);
	
	QLabel* label_nuc = new QLabel("Fixed Background Suppression");
	lbl_fixed_suppression = new QLabel("No Frames Selected");

	btn_create_nuc = new QPushButton("Create Filter");

	QFrame* horizontal_segment5 = new QFrame();
	horizontal_segment5->setFrameShape(QFrame::HLine);

	//QWidget* widget_tab_processing_nuc = new QWidget();
	QGridLayout* grid_tab_processing_nuc = new QGridLayout();

	grid_tab_processing_nuc->addWidget(label_nuc, 0, 0, 1, 2);
	grid_tab_processing_nuc->addWidget(lbl_fixed_suppression, 1, 0, 1, 2);
	grid_tab_processing_nuc->addWidget(btn_create_nuc, 2, 1);
	grid_tab_processing_nuc->addWidget(horizontal_segment5, 3, 0, 1, 2);

	vlayout_tab_processing->addLayout(grid_tab_processing_nuc);

	// ------------------------------------------------------------------------

	QLabel* label_background_subtraction = new QLabel("Adaptive Background Suppression");
	lbl_adaptive_background_suppression = new QLabel("No Frames Setup");
	btn_bgs = new QPushButton("Create Filter");

	QFrame* horizontal_segment6 = new QFrame();
	horizontal_segment6->setFrameShape(QFrame::HLine);

	//QWidget* widget_tab_processing_bgs = new QWidget();
	QGridLayout* grid_tab_processing_bgs = new QGridLayout();

	grid_tab_processing_bgs->addWidget(label_background_subtraction, 0, 0, 1, 2);
	grid_tab_processing_bgs->addWidget(lbl_adaptive_background_suppression, 1, 0, 1, 2);
	grid_tab_processing_bgs->addWidget(btn_bgs, 2, 1);
	grid_tab_processing_bgs->addWidget(horizontal_segment6, 3, 0, 1, 2);

	vlayout_tab_processing->addLayout(grid_tab_processing_bgs);

	// ------------------------------------------------------------------------
	QLabel* label_deinterlace = new QLabel("De-Interlace Methods");
	cmb_deinterlace_options = new QComboBox();
	btn_deinterlace = new QPushButton("Create Filter");

	QFrame* horizontal_segment7 = new QFrame();
	horizontal_segment7->setFrameShape(QFrame::HLine);

	cmb_deinterlace_options->addItem("Max Absolute Value");
	cmb_deinterlace_options->addItem("Centroid");
	cmb_deinterlace_options->addItem("Avg Cross Correlation");

	QGridLayout* grid_tab_processing_deinterlace = new QGridLayout();

	grid_tab_processing_deinterlace->addWidget(label_deinterlace, 0, 0, 1, 2);
	grid_tab_processing_deinterlace->addWidget(cmb_deinterlace_options, 1, 0, 1, 2);
	grid_tab_processing_deinterlace->addWidget(btn_deinterlace, 2, 1);
	grid_tab_processing_deinterlace->addWidget(horizontal_segment7, 3, 0, 1, 2);

	vlayout_tab_processing->addLayout(grid_tab_processing_deinterlace);

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

	QFrame* horizontal_segment = new QFrame();
	horizontal_segment->setFrameShape(QFrame::HLine);

	cmb_processing_states = new QComboBox();
	btn_undo_step = new QPushButton("Undo One Step");
	
	QGridLayout* grid_workspace = new QGridLayout();
	grid_workspace->addWidget(cmb_workspace_name, 0, 0, 1, -1);
	grid_workspace->addWidget(btn_workspace_load, 1, 0, 1, 1);
	grid_workspace->addWidget(btn_workspace_save, 1, 1, 1, 1);
	grid_workspace->addWidget(horizontal_segment, 2, 0, 1, -1);
	grid_workspace->addWidget(cmb_processing_states, 3, 0, 1, 1);
	grid_workspace->addWidget(btn_undo_step, 3, 1, 1, 1);

	vlayout_tab_workspace->addLayout(grid_workspace);
	return widget_tab_workspace;
}

void SirveApp::setup_video_frame(){

	frame_video_player->setFrameShape(QFrame::Box);
	QVBoxLayout* vlayout_frame_video = new QVBoxLayout(frame_video_player);

	// ------------------------------------------------------------------------

	frame_video = new QFrame();
	frame_video->setMinimumHeight(480);
	frame_video->setMinimumWidth(640);
	frame_video->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	
	vlayout_frame_video->addWidget(frame_video);

	// ------------------------------------------------------------------------

	lbl_video_frame = new QLabel("Frame");
	lbl_video_frame->setAlignment(Qt::AlignLeft);

	lbl_video_time_midnight = new QLabel("Time");
	lbl_video_time_midnight->setAlignment(Qt::AlignHCenter);
	lbl_zulu_time = new QLabel("Zulu");
	lbl_zulu_time->setAlignment(Qt::AlignRight);

	lbl_fps = new QLabel("fps");
	lbl_fps->setAlignment(Qt::AlignRight);

	//QWidget* widget_video_label_descriptions = new QWidget();
	QHBoxLayout* hlayout_video_label_description = new QHBoxLayout();

	hlayout_video_label_description->addWidget(lbl_video_frame);
	hlayout_video_label_description->addWidget(lbl_video_time_midnight);
	hlayout_video_label_description->addWidget(lbl_zulu_time);

	vlayout_frame_video->addLayout(hlayout_video_label_description);

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

	QWidget* widget_video_buttons = new QWidget();
	QHBoxLayout* hlayout_video_buttons = new QHBoxLayout();

	hlayout_video_buttons->addWidget(btn_frame_save);
	hlayout_video_buttons->addWidget(btn_frame_record);
	hlayout_video_buttons->addWidget(btn_zoom);
	hlayout_video_buttons->addWidget(btn_calculate_radiance);
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

	frame_histogram = new QFrame();
	frame_histogram_abs = new QFrame();
	frame_histogram_abs_full = new QFrame();

	QWidget* widget_tab_histogram_split = new QWidget();
	QVBoxLayout* vlayout_tab_histogram = new QVBoxLayout(widget_tab_histogram_split);

	vlayout_tab_histogram->addWidget(frame_histogram);
	vlayout_tab_histogram->addWidget(frame_histogram_abs);
	vlayout_tab_histogram->setStretch(0, 1);
	vlayout_tab_histogram->setStretch(1, 1);

	stacked_layout_histograms = new QStackedLayout();
	stacked_layout_histograms->addWidget(frame_histogram_abs_full);
	stacked_layout_histograms->addWidget(widget_tab_histogram_split);
	
	QWidget* widget_tab_histogram = new QWidget();
	widget_tab_histogram->setLayout(stacked_layout_histograms);

	// ------------------------------------------------------------------------

	frame_plots = new FixedAspectRatioFrame();
	frame_plots->enable_fixed_aspect_ratio(false);
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
	// 
	
	QObject::connect(&thread_video, &QThread::started, video_display, &VideoDisplay::update_display_frame);

	QObject::connect(&video_display->container, &Video_Container::update_display_video, video_display, &VideoDisplay::receive_video_data);
	QObject::connect(btn_undo_step, &QPushButton::clicked, &video_display->container, &Video_Container::undo);

	QObject::connect(&video_display->container, &Video_Container::states_cleared, cmb_processing_states, &QComboBox::clear);
	QObject::connect(&video_display->container, &Video_Container::state_added, this, &SirveApp::handle_new_processing_state);
	QObject::connect(&video_display->container, &Video_Container::state_removed, cmb_processing_states, &QComboBox::removeItem);
	QObject::connect(cmb_processing_states, qOverload<int>(&QComboBox::currentIndexChanged), &video_display->container, &Video_Container::select_state);

	QObject::connect(playback_controller, &Playback::update_frame, video_display, &VideoDisplay::update_specific_frame);
	QObject::connect(&color_correction, &Min_Max_Value::update_min_max, video_display, &VideoDisplay::update_color_correction);
	QObject::connect(video_display->histogram_plot, &HistogramLine_Plot::click_drag_histogram, this, &SirveApp::histogram_clicked);

	//---------------------------------------------------------------------------	
	
	QObject::connect(tab_menu, &QTabWidget::currentChanged, this, &SirveApp::auto_change_plot_display);
	QObject::connect(chk_relative_histogram, &QCheckBox::toggled, this, &SirveApp::toggle_relative_histogram);
	
	//---------------------------------------------------------------------------	
	// Link color correction sliders to changing color correction values
	QObject::connect(slider_gain, &QSlider::valueChanged, this, &SirveApp::gain_slider_toggled);
	QObject::connect(slider_lift, &QSlider::valueChanged, this, &SirveApp::lift_slider_toggled);

	//---------------------------------------------------------------------------	

	QObject::connect(btn_reset_color_correction, &QPushButton::clicked, this, &SirveApp::reset_color_correction);
	
	
	//---------------------------------------------------------------------------
		
	connect(chk_show_tracks, &QCheckBox::stateChanged, this, &SirveApp::toggle_osm_tracks);
	connect(cmb_primary_tracker_color, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::edit_primary_tracker_color);
	connect(cmb_tracker_color, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::edit_tracker_color);
	
	connect(chk_primary_track_data, &QCheckBox::stateChanged, this, &SirveApp::toggle_primary_track_data);
	connect(chk_sensor_track_data, &QCheckBox::stateChanged, this, &SirveApp::toggle_sensor_track_data);
	connect(chk_show_time, &QCheckBox::stateChanged, this, &SirveApp::toggle_frame_time);
	connect(cmb_color_maps, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::edit_color_map);
	connect(cmb_text_color, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SirveApp::edit_banner_color);

	connect(btn_add_annotations, &QPushButton::clicked, this, &SirveApp::annotate_video);
	connect(btn_change_banner_text, &QPushButton::clicked, this, &SirveApp::edit_banner_text);

	//---------------------------------------------------------------------------

	// Link horizontal slider to playback controller
	QObject::connect(playback_controller, &Playback::update_frame, slider_video, &QSlider::setValue);
	QObject::connect(slider_video, &QSlider::valueChanged, playback_controller, &Playback::set_current_frame_number);

	//---------------------------------------------------------------------------

	// Link playback to play controls
	QObject::connect(btn_play, &QPushButton::clicked, playback_controller, &Playback::start_timer);
	QObject::connect(btn_pause, &QPushButton::clicked, playback_controller, &Playback::stop_timer);
	QObject::connect(btn_reverse, &QPushButton::clicked, playback_controller, &Playback::reverse);


	QObject::connect(btn_fast_forward, &QPushButton::clicked, playback_controller, &Playback::speed_timer);
	QObject::connect(btn_slow_back, &QPushButton::clicked, playback_controller, &Playback::slow_timer);
	QObject::connect(btn_next_frame, &QPushButton::clicked, playback_controller, &Playback::next_frame);
	QObject::connect(btn_prev_frame, &QPushButton::clicked, playback_controller, &Playback::prev_frame);
	QObject::connect(btn_frame_record, &QPushButton::clicked, this, &SirveApp::start_stop_video_record);

	QObject::connect(btn_fast_forward, &QPushButton::clicked, this, &SirveApp::update_fps);
	QObject::connect(btn_slow_back, &QPushButton::clicked, this, &SirveApp::update_fps);

	QObject::connect(btn_zoom, &QPushButton::clicked, this, &SirveApp::toggle_zoom_on_video);
	QObject::connect(btn_calculate_radiance, &QPushButton::clicked, this, &SirveApp::toggle_calculation_on_video);

	//---------------------------------------------------------------------------

	//Link buttons to functions
	QObject::connect(btn_load_osm, &QPushButton::clicked, this, &SirveApp::ui_choose_abp_file);
	QObject::connect(btn_calibration_dialog, &QPushButton::clicked, this, &SirveApp::show_calibration_dialog);
	QObject::connect(btn_get_frames, &QPushButton::clicked, this, &SirveApp::ui_load_abir_data);
	QObject::connect(txt_end_frame, &QLineEdit::returnPressed, this, &SirveApp::ui_load_abir_data);

	QObject::connect(btn_create_nuc, &QPushButton::clicked, this, &SirveApp::ui_execute_non_uniformity_correction_selection_option);

	QObject::connect(btn_bgs, &QPushButton::clicked, this, &SirveApp::ui_execute_background_subtraction);
	
	QObject::connect(btn_deinterlace, &QPushButton::clicked, this, &SirveApp::ui_execute_deinterlace);

	//---------------------------------------------------------------------------

	QObject::connect(btn_workspace_save, &QPushButton::clicked, this, &SirveApp::save_workspace);
	QObject::connect(btn_workspace_load, &QPushButton::clicked, this, &SirveApp::load_workspace);

	// Connect epoch button click to function
	QObject::connect(btn_apply_epoch, &QPushButton::clicked, this, &SirveApp::apply_epoch_time);

	//Enable saving frame
	QObject::connect(btn_frame_save, &QPushButton::clicked, this, &SirveApp::save_frame);

	//Update frame number label
	QObject::connect(playback_controller, &Playback::update_frame, this, &SirveApp::set_frame_number_label);


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
	INFO << "WORKSPACE: Start ABP load process";

	QString current_workspace_name = cmb_workspace_name->currentText();
	WorkspaceValues workspace_vals = workspace.load_state(current_workspace_name);

	int compare = QString::compare(workspace_vals.image_path, "", Qt::CaseInsensitive);
	if (compare == 0) {
		QtHelpers::LaunchMessageBox(QString("Issue Loading Workspace"), "The workspace is empty.");
		return;
	}

	INFO << "WORKSPACE: ABP image path in workspace, attempting to load";

	bool validated = validate_abp_files(workspace_vals.image_path);
	if (validated) {
		load_osm_data();
	}

	if (workspace_vals.start_frame == 0 || workspace_vals.end_frame == 0) {
		INFO << "No frames selected in the workspace";
		return;
	}
	else {
		load_abir_data(workspace_vals.start_frame, workspace_vals.end_frame);
	}

	for (auto i = 1; i < workspace_vals.all_states.size(); i++)
	{
		processing_state current_state = workspace_vals.all_states[i];
		switch (current_state.method)
		{
			case Processing_Method::background_subtraction:
				INFO << "Creating background subtraction from workspace.";
				create_background_subtraction_correction(current_state.bgs_relative_start_frame, current_state.bgs_num_frames);
				break;

			case Processing_Method::deinterlace:
				INFO << "Creating deinterlace from workspace.";
				create_deinterlace(current_state.deint_type);
				break;

			case Processing_Method::non_uniformity_correction:
				INFO << "Creating non uniformity correction from workspace.";
				create_non_uniformity_correction(current_state.nuc_file_path, current_state.nuc_start_frame, current_state.nuc_stop_frame);
				break;

			default:
				INFO << "Unexpected processing method in workspace.";
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
	INFO << "GUI: Starting ABP selection and load process";

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
		INFO << "GUI: No valid file selected for load";
		
		lbl_file_load->setText(possible_abp_file_metadata.info_msg);

		if (eng_data != NULL) {
			// if eng_data already initialized, allow user to re-select frames
			txt_start_frame->setEnabled(true);
			txt_end_frame->setEnabled(true);
			btn_get_frames->setEnabled(true);
			btn_calibration_dialog->setEnabled(true);
		}

		INFO << "FILE SELECTION: Unable to load valid files";

		QtHelpers::LaunchMessageBox(QString("Issue Finding File"), possible_abp_file_metadata.error_msg);
		
		return false;
	}

	abp_file_metadata = possible_abp_file_metadata;

	INFO << "FILE SELECTION: Success - both .abpimage and .abposm files exist";
	return true;
};

void SirveApp::load_osm_data()
{
	osm_frames = osm_reader.read_osm_file(abp_file_metadata.osm_path);
	if (osm_frames.size() == 0) {
		WARN << "File Processing: OSM load process quit early. File not loaded correctly";
		
		QtHelpers::LaunchMessageBox(QString("Error loading OSM file"), QString("Error reading OSM file. Close program and open logs for details."));
		return;
	}

	lbl_file_load->setText(abp_file_metadata.info_msg);
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

	if (eng_data != NULL) {
			
		DEBUG << "GUI: Deleting pointers to engineering data, data plots, and layout";

		toggle_video_playback_options(false);

		// Reset video frame
		playback_controller->stop_timer();
		reset_color_correction();

		// delete objects with existing data within them
		delete eng_data;
		delete data_plots;
		delete engineering_plot_layout;			
		
		video_display->container.clear_processing_states();
		video_display->remove_frame();

		cmb_processing_states->setEnabled(false);

		clear_frame_label();
		
		video_layout->addWidget(video_display->label);
		frame_video->setLayout(video_layout);
	}

	DEBUG << "GUI: Creating new objects for engineering data, data plots, and layout";

	eng_data = new Engineering_Data(osm_frames);

	data_plots = new Engineering_Plots(osm_frames);

	data_plots->past_midnight = eng_data->get_seconds_from_midnight();
	data_plots->past_epoch = eng_data->get_seconds_from_epoch();

	data_plots->track_irradiance_data = eng_data->get_track_irradiance_data();

	int num_tracks = data_plots->track_irradiance_data.size();
	if (num_tracks == 0)
	{
		QtHelpers::LaunchMessageBox(QString("No Tracking Data"), "No tracking data was found within the file. No data will be plotted.");
	}
	
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
	engineering_plot_layout->addWidget(data_plots->chart_view);
	frame_plots->setLayout(engineering_plot_layout);

	// Reset settings on video playback to defaults
	video_display->toggle_osm_tracks(false);

	video_display->toggle_primary_track_data(false);

	//menu_sensor_boresight->setIconVisibleInMenu(false);
	video_display->toggle_sensor_boresight_data(false);

	btn_calculate_radiance->setChecked(false);
	btn_calculate_radiance->setEnabled(false);

	CalibrationData temp;
	calibration_model = temp;

	video_display->banner_color = QString("red");
	video_display->banner_text = QString("EDIT CLASSIFICATION");
	video_display->tracker_color = QString("red");
	
	// Reset setting engineering plot defaults
	menu_plot_all_data->setIconVisibleInMenu(true);
	
	menu_plot_primary->setIconVisibleInMenu(false);

	menu_plot_frame_marker->setIconVisibleInMenu(false);
	
	enable_engineering_plot_options();
	data_plots->set_plot_title(QString("EDIT CLASSIFICATION"));

	INFO << "GUI: OSM successfully loaded";

	return;
}

void SirveApp::ui_load_abir_data()
{
	btn_get_frames->setEnabled(false);

	INFO << "GUI: Starting ABIR load process";

	int min_frame = get_integer_from_txt_box(txt_start_frame->text());
	int max_frame = get_integer_from_txt_box(txt_end_frame->text());

	if (!verify_frame_selection(min_frame, max_frame)) {
		btn_get_frames->setEnabled(true);

		INFO << "GUI: No video loaded";
		return;
	}

	DEBUG << "GUI: Frame numbers are valid, loading ABIR data";

	load_abir_data(min_frame, max_frame);
}

void SirveApp::load_abir_data(int min_frame, int max_frame)
{
	// Load the ABIR data
	playback_controller->stop_timer();

	// Create the video properties data
	video_details vid_details;

	INFO << "GUI: Reading in video data";
	vid_details.set_number_of_bits(config_values.max_used_bits);

	//----------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------

	ABIR_Data_Result abir_data_result = file_processor.load_image_file(abp_file_metadata.image_path, min_frame, max_frame, config_values.version);
	
	if (abir_data_result.had_error) {
		QtHelpers::LaunchMessageBox(QString("Error Reading ABIR Frames"), "Error reading .abpimage file. See log for more details.");
		INFO << "GUI: Video import stopped. An error occurred.";
		btn_get_frames->setEnabled(true);
		return;
	}

	int x_pixels = abir_data_result.x_pixels;
	int y_pixels = abir_data_result.y_pixels;
	std::vector<std::vector<uint16_t>> video_frames = abir_data_result.video_frames_16bit;

	DEBUG << "GUI: Frames are of size " << x_pixels << " x " << y_pixels;

	video_display->clear_all_zoom_levels(x_pixels, y_pixels);
	vid_details.set_image_size(x_pixels, y_pixels);
	vid_details.set_video_frames(video_frames);

	frame_video->setMinimumHeight(y_pixels);
	frame_video->setMinimumWidth(x_pixels);

	int number_frames = vid_details.frames_16bit.size();
	QString status_txt = lbl_file_load->text();
	QString update_text("\nFrames ");
	update_text.append(QString::number(min_frame));
	update_text.append(" to ");
	update_text.append(QString::number(min_frame + number_frames - 1));
	update_text.append(" were loaded");
	status_txt.append(update_text);
	lbl_file_load->setText(status_txt);
	txt_start_frame->setText(QString::number(min_frame));
	txt_end_frame->setText(QString::number(max_frame));

	processing_state primary = { Processing_Method::original, vid_details };
	video_display->container.clear_processing_states();
	video_display->container.add_processing_state(primary);
	cmb_processing_states->setEnabled(true);
	btn_workspace_save->setEnabled(true);
	
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

	//std::vector<Plotting_Frame_Data> temp_data = eng_data->get_plotting_frame_data();
	//std::vector<Plotting_Frame_Data>::const_iterator first = temp_data.begin() + min_frame - 1;
	//std::vector<Plotting_Frame_Data>::const_iterator last = temp_data.begin() + (min_frame) + (max_frame - min_frame);
	//std::vector<Plotting_Frame_Data> subset_data(first, last);
	int index0 = min_frame - 1;
	int index1 = min_frame + (max_frame - min_frame);
	std::vector<Plotting_Frame_Data> temp = eng_data->get_subset_plotting_frame_data(index0, index1);
	video_display->set_frame_data(temp, file_processor.abir_data.ir_data);

	// Reset engineering plots with new sub plot indices
	data_plots->index_sub_plot_xmin = min_frame - 1;
	data_plots->index_sub_plot_xmax = max_frame - 1;
	data_plots->plot_all_data = false;
	menu_plot_all_data->setIconVisibleInMenu(false);
	data_plots->plot_current_marker = true;
	menu_plot_frame_marker->setIconVisibleInMenu(true);
	plot_change();

	//Update frame marker on engineering plot
	QObject::connect(playback_controller, &Playback::update_frame, data_plots, &Engineering_Plots::plot_current_step);
	
	playback_controller->set_speed_index(10);
	update_fps();

	tab_plots->setCurrentIndex(1);

	btn_get_frames->setEnabled(true);
	btn_calibration_dialog->setEnabled(true);

	tab_menu->setTabEnabled(1, true);
	tab_menu->setTabEnabled(2, true);

	INFO << "GUI: ABIR file load complete";

	toggle_video_playback_options(true);
	playback_controller->start_timer();

	//---------------------------------------------------------------------------
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

void SirveApp::update_fps()
{
	QString fps = QString::number(playback_controller->speeds[playback_controller->index_speed], 'g', 2);
	fps.append(" fps");

	lbl_fps->setText(fps);
}


void SirveApp::histogram_clicked(double x0, double x1) {
	// connects the clickable histogram to the main program 

	// get current lift/gain values
	double lift_value = color_correction.min_convert_slider_to_value(slider_lift->value());
	double gain_value = color_correction.max_convert_slider_to_value(slider_gain->value());

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

		slider_gain->setValue(color_correction.get_ui_slider_value(x1));
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

		slider_lift->setValue(color_correction.get_ui_slider_value(x1));
	}

}


void SirveApp::lift_slider_toggled(int value) {

	double lift_value = color_correction.min_convert_slider_to_value(slider_lift->value());
	double gain_value = color_correction.max_convert_slider_to_value(slider_gain->value());

	//Prevent lift from being higher than gain value
	if (lift_value >= gain_value) {

		int new_value = color_correction.get_ui_slider_value(gain_value);
		slider_lift->setValue(new_value - 1);
		lift_value = color_correction.min_convert_slider_to_value(slider_lift->value());;
	}

	color_correction_toggled(lift_value, gain_value);
}

void SirveApp::gain_slider_toggled(int value) {

	double lift_value = color_correction.min_convert_slider_to_value(slider_lift->value());
	double gain_value = color_correction.max_convert_slider_to_value(slider_gain->value());

	// Prevent gain going below lift value
	if (gain_value <= lift_value) {

		int new_value = color_correction.get_ui_slider_value(lift_value);
		slider_gain->setValue(new_value + 1);
		gain_value = color_correction.max_convert_slider_to_value(slider_gain->value());;
	}

	color_correction_toggled(lift_value, gain_value);
}

void SirveApp::color_correction_toggled(double lift_value, double gain_value) {
		
	emit color_correction.update_min_max(lift_value, gain_value);
	
	set_color_correction_slider_labels();

	if (!playback_controller->timer->isActive() && false)
	{
		unsigned int current_frame_number = playback_controller->get_current_frame_number();
		video_display->update_specific_frame(current_frame_number);
	}		

	DEBUG << "GUI: New values set for Lift/Gamma/Gain correction: " << std::to_string(lift_value) << "/" << "/" << std::to_string(gain_value);
}

void SirveApp::reset_color_correction()
{
		
	slider_lift->setValue(0);
	slider_gain->setValue(1000);
	chk_relative_histogram->setChecked(false);

	DEBUG << "GUI: Color correction reset";
}


void SirveApp::toggle_plot_full_data()
{
	data_plots->plot_all_data = !data_plots->plot_all_data;
	menu_plot_all_data->setIconVisibleInMenu(data_plots->plot_all_data);

	plot_change();
	DEBUG << "GUI: Toggled plot full data";
	
}

void SirveApp::toggle_plot_primary_only()
{
	data_plots->plot_primary_only = !data_plots->plot_primary_only;
	menu_plot_primary->setIconVisibleInMenu(data_plots->plot_primary_only);

	plot_change();
	DEBUG << "GUI: Toggled plot primary data only";
}

void SirveApp::toggle_plot_current_frame_marker() {
	data_plots->plot_current_marker = !data_plots->plot_current_marker;
	menu_plot_frame_marker->setIconVisibleInMenu(data_plots->plot_current_marker);

	plot_change();
	DEBUG << "GUI: Toggled plot current frame marker";
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
		data_plots->track_irradiance_data = eng_data->get_track_irradiance_data();

		int index0 = data_plots->index_sub_plot_xmin + 1;
		int index1 = data_plots->index_sub_plot_xmax + 1;

		std::vector<Plotting_Frame_Data> temp = eng_data->get_subset_plotting_frame_data(index0, index1);
		video_display->update_frame_data(temp);
		video_display->update_display_frame();

		set_zulu_label();

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
	INFO << "GUI: Plot saved";
}

void SirveApp::save_frame()
{
	if(playback_controller->is_running())
		playback_controller->stop_timer();
	
	video_display->save_frame();
	INFO << "GUI: Video frame saved  ";

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
	
	if (ok) {

		DEBUG << "GUI: Banner text changed";
		video_display->update_banner_text(input_text);

		// checks if banners are the same and asks user if they want them to be the same
		QString plot_banner_text = data_plots->title;
		int check = QString::compare(input_text, plot_banner_text, Qt::CaseSensitive);
		if (check != 0)
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle("Update All Banners");
			msgBox.setText("Video and plot banners do not match. Would you like to set both to the same banner?");
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::No);
			int ret = msgBox.exec();

			if (ret == QMessageBox::Yes)
			{
				data_plots->set_plot_title(input_text);
				
			}
		}
	}
	else {
		DEBUG << "GUI: Banner change cancelled";
	}
}

void SirveApp::edit_plot_text()
{
	bool ok;
	QString input_text = QInputDialog::getText(0, "Plot Header Text", "Input Plot Header Text", QLineEdit::Normal, data_plots->title, &ok);

	if (ok) {
		data_plots->set_plot_title(input_text);
		DEBUG << "GUI: Plot header text changed";

	}
	else {
		DEBUG << "GUI: Plot header text change cancelled";
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

	int min_frame, max_frame;
	if (item == "Export All Data") 
	{
		min_frame = 0;
		max_frame = eng_data->frame_data.size() - 1;

		eng_data->write_track_date_to_csv(save_path, min_frame, max_frame);
	}
	else {
		min_frame = data_plots->index_sub_plot_xmin + 1;
		max_frame = data_plots->index_sub_plot_xmax + 1;

		eng_data->write_track_date_to_csv(save_path, min_frame, max_frame);
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

	int index = cmb_color_maps->currentIndex();
	QString color = cmb_color_maps->currentText();
	video_display->update_color_map(color);

}

void SirveApp::edit_banner_color()
{
	
	int index = cmb_text_color->currentIndex();
	QString color = cmb_text_color->currentText();

	video_display->update_banner_color(color);

	return;
	//}
}

void SirveApp::edit_tracker_color()
{
	QString tracker_color = cmb_tracker_color->currentText();
	video_display->update_tracker_color(tracker_color);
	video_display->update_display_frame();
}

void SirveApp::edit_primary_tracker_color()
{
	QString tracker_color = cmb_primary_tracker_color->currentText();
	video_display->update_tracker_primary_color(tracker_color);
	video_display->update_display_frame();
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

		DEBUG << "GUI: Engineering plot units changed to case " << x_index;
		DEBUG << "GUI: Engineering plot changed to case " << y_index;

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
	QObject::connect(&annotate_gui, &AnnotationListDialog::annotation_list_updated, video_display, &VideoDisplay::update_display_frame);
	annotate_gui.exec();
}

int SirveApp::get_integer_from_txt_box(QString input)
{
	bool convert_value_numeric;
	int value = input.toInt(&convert_value_numeric);
		
	if (convert_value_numeric) {
		DEBUG << "GUI: Integer from text box is " << value;
		return value;
	}
	else {
		DEBUG << "GUI: Value is text box is non-numeric: " << input.toLocal8Bit().constData();
		return -1;
	}
}

void SirveApp::set_frame_number_label(unsigned int current_frame_number)
{
	// check that engineering is non-null before accessing
	if (eng_data) {
		int index = data_plots->index_sub_plot_xmin;

		int frame_number = index + current_frame_number + 1;
		QString frame_text("Frame # ");
		frame_text.append(QString::number(frame_number));
		lbl_video_frame->setText(frame_text);

		double seconds_midnight = eng_data->get_epoch_time_from_index(index + current_frame_number);
		QString seconds_text("From Midnight ");
		seconds_text.append(QString::number(seconds_midnight, 'g', 8));
		lbl_video_time_midnight->setText(seconds_text);

		// ------------------------------------------------------------------------------------
		set_zulu_label();

	}

}

void SirveApp::set_zulu_label()
{

	int index = data_plots->index_sub_plot_xmin;
	unsigned int current_frame_number = playback_controller->get_current_frame_number();

	double seconds_midnight = eng_data->get_epoch_time_from_index(index + current_frame_number);

	// ------------------------------------------------------------------------------------
	int hour = seconds_midnight / 3600;
	int minutes = (seconds_midnight - hour * 3600) / 60;
	double seconds = seconds_midnight - hour * 3600 - minutes * 60;

	QString zulu_time("");
	if (hour < 10)
		zulu_time.append("0");
	zulu_time.append(QString::number(hour));
	zulu_time.append(":");

	if (minutes < 10)
		zulu_time.append("0");
	zulu_time.append(QString::number(minutes));
	zulu_time.append(":");

	if (seconds < 10)
		zulu_time.append("0");
	zulu_time.append(QString::number(seconds, 'f', 3));

	zulu_time.append("Z");
	lbl_zulu_time->setText(zulu_time);

}

void SirveApp::clear_frame_label()
{

	QString frame_text("");
	lbl_video_frame->setText(frame_text);

	QString seconds_text("");
	lbl_video_time_midnight->setText(seconds_text);

	QString zulu_text("");
	lbl_zulu_time->setText(zulu_text);
}


void SirveApp::copy_osm_directory()
{
	clipboard->setText(abp_file_metadata.osm_path);
}

void SirveApp::update_enhanced_range(bool input)
{

	emit enhanced_dynamic_range(input);

}

void SirveApp::toggle_relative_histogram(bool input)
{
	if (input) {

		video_display->show_relative_histogram = true;

		stacked_layout_histograms->setCurrentIndex(1);
		video_display->update_display_frame();

	}
	else
	{
		video_display->show_relative_histogram = false;
		stacked_layout_histograms->setCurrentIndex(0);

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
	data_plots->track_irradiance_data = eng_data->get_track_irradiance_data();
	plot_change();
}

void SirveApp::create_non_uniformity_correction_from_external_file()
{
	ExternalNUCInformationWidget external_nuc_dialog;

	auto response = external_nuc_dialog.exec();

	if (response == 0) {

		return;
	}

	QString image_path = external_nuc_dialog.abp_metadata.image_path;
	unsigned int min_frame = external_nuc_dialog.start_frame;
	unsigned int max_frame = external_nuc_dialog.stop_frame;

	try {

		// assumes file version is same as base file opened
		create_non_uniformity_correction(image_path, min_frame, max_frame);
	}
	catch (const std::exception& e)
	{
		// catch any errors when loading frames. try-catch not needed when loading frames from same file since no errors originally occurred
		INFO << "NUC: Exception occurred when loading video data for the NUC: " << e.what();
	
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

	bool ok;
	QString item = QInputDialog::getItem(this, "Fixed Mean Background Suppression", "Options", options, 0, false, &ok);
	
	if (!ok)
		return;

	if (item == "From Current File") {


		// get total number of frames
		int num_messages = osm_frames.size();

		QString prompt1 = "Start Frame (";
		prompt1.append(QString::number(num_messages));
		prompt1.append(" frames total)");

		QString prompt2 = "Stop Frame (";
		prompt2.append(QString::number(num_messages));
		prompt2.append(" frames total)");

		bool ok;

		// get min frame for nuc while limiting input between 1 and total messages
		int start_frame = QInputDialog::getInt(this, "Exernal File NUC Correction", prompt1, 1, 1, num_messages, 1, &ok);
		if (!ok)
			return;

		// get max frame for nuc while limiting input between min and total messages
		int stop_frame = QInputDialog::getInt(this, "Exernal File NUC Correction", prompt2, start_frame, start_frame, num_messages, 1, &ok);
		if (!ok)
			return;

		INFO << "GUI: Creating NUC from current file.";
		create_non_uniformity_correction(abp_file_metadata.image_path, start_frame, stop_frame);

	}
	else {
		INFO << "GUI: Creating NUC from external file.";
		create_non_uniformity_correction_from_external_file();
	}
	
}

void SirveApp::create_non_uniformity_correction(QString file_path, unsigned int min_frame, unsigned int max_frame)
{
	INFO << "Creating non-uniformity correction.";

	if (!verify_frame_selection(min_frame, max_frame)) {
		INFO << "GUI: NUC correction not completed, invalid frame selection";
		
		QtHelpers::LaunchMessageBox(QString("Invalid Frame Selection"), "NUC correction not completed, invalid frame selection");
		return;
	}

	NUC nuc;
	std::vector<double> nuc_correction = nuc.get_nuc_correction(abp_file_metadata.image_path, min_frame, max_frame, config_values.version);

	if (nuc_correction.size() == 0)
	{
		QtHelpers::LaunchMessageBox(QString("File Version Not Within Range"), "File version was not within valid range. See log for more details");
		
		INFO << "GUI: NUC correction not completed";
		return;
	}

	INFO << "Calculated NUC correction";

	processing_state original = video_display->container.copy_current_state();

	processing_state nuc_state = original;
	nuc_state.details.frames_16bit.clear();
	nuc_state.details.histogram_data.clear();

	// Apply NUC to the frames		
	int number_frames = original.details.frames_16bit.size();

	QProgressDialog progress("", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.setWindowTitle(QString("Fixed Background Suppression"));
	progress.setMinimum(0);
	progress.setMaximum(number_frames - 1);
	progress.setLabelText(QString("Applying correction..."));
	progress.setMinimumWidth(300);

	for (int i = 0; i < number_frames; i++) {
		progress.setValue(i);
		DEBUG << "GUI: Applying NUC correction to " << i + 1 << " of " << number_frames << " frames";
		nuc_state.details.frames_16bit.push_back(nuc.apply_nuc_correction(original.details.frames_16bit[i]));
		if (progress.wasCanceled())
			break;
	}

	if (progress.wasCanceled()) {
		INFO << "GUI: NUC process was canceled";
		return;
	}

	progress.setLabelText(QString("Down-converting video and creating histogram data..."));

	nuc_state.method = Processing_Method::non_uniformity_correction;
	nuc_state.nuc_file_path = file_path;
	nuc_state.nuc_start_frame = min_frame;
	nuc_state.nuc_stop_frame = max_frame;
	video_display->container.add_processing_state(nuc_state);

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
	
	DEBUG << "GUI: Found de-interlacing method type";
	create_deinterlace(deinterlace_method_type);
}

void SirveApp::create_deinterlace(deinterlace_type deinterlace_method_type)
{
	INFO << "Creating deinterlace";

	processing_state original = video_display->container.copy_current_state();
	
	Deinterlace deinterlace_method(deinterlace_method_type, original.details.x_pixels, original.details.y_pixels);
	
	processing_state deinterlace_state = original;
	deinterlace_state.details.frames_16bit.clear();
	deinterlace_state.details.histogram_data.clear();
	
	// Apply de-interlace to the frames		
	
	int number_frames = original.details.frames_16bit.size();

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
		DEBUG << "GUI: Applying de-interlace to " << i + 1 << " of " << number_frames << " frames";
		deinterlace_state.details.frames_16bit.push_back(deinterlace_method.deinterlace_frame(original.details.frames_16bit[i]));
		if (progress.wasCanceled())
			break;
	}

	if (progress.wasCanceled()) {
		INFO << "De-interlace process was canceled";
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
		cmb_primary_tracker_color->setEnabled(true);
	}
	else
	{
		cmb_tracker_color->setEnabled(false);
		cmb_primary_tracker_color->setEnabled(false);
	}
	
	video_display->update_display_frame();
}

void SirveApp::toggle_primary_track_data()
{

	bool current_status = video_display->display_tgt_pos_txt;

	video_display->toggle_primary_track_data(!current_status);
	video_display->update_display_frame();

}

void SirveApp::toggle_sensor_track_data()
{

	bool current_status = video_display->display_boresight_txt;

	video_display->toggle_sensor_boresight_data(!current_status);
	video_display->update_display_frame();

}

void SirveApp::toggle_frame_time()
{
	bool current_status = chk_show_time->isChecked();
	video_display->display_time = current_status;
	video_display->update_display_frame();
}

void SirveApp::handle_new_processing_state(QString state_name, int index)
{
	cmb_processing_states->addItem(state_name);
	cmb_processing_states->setCurrentIndex(index);
}

void SirveApp::ui_execute_background_subtraction() {

	INFO << "GUI: Background subtraction video being created";
	
	//-----------------------------------------------------------------------------------------------
	// get user selected frames for suppression

	int delta_frames = data_plots->index_sub_plot_xmax - data_plots->index_sub_plot_xmin;

	bool ok;
	int relative_start_frame = QInputDialog::getInt(this, "Adaptive Background Suppression", "Relative start frame", -5, -delta_frames, delta_frames, 1, &ok);
	if (!ok)
		return;

	int number_of_frames = QInputDialog::getInt(this, "Adaptive Background Suppresssion", "Number of frames to use for suppression", 5, 1, std::abs(relative_start_frame), 1, &ok);
	if (!ok)
		return;

	DEBUG << "GUI: Input value for background subtraction validated";
	create_background_subtraction_correction(relative_start_frame, number_of_frames);
}

void SirveApp::create_background_subtraction_correction(int relative_start_frame, int num_frames)
{
	INFO << "GUI: Background subtraction video being created";
	processing_state original = video_display->container.copy_current_state();

	INFO << "GUI: Creating adjustment for video";
	std::vector<std::vector<double>> background_correction = AdaptiveNoiseSuppression::get_correction(relative_start_frame, num_frames, original.details);

	if (background_correction.size() == 0) {
		INFO << "GUI: Background subtraction adjustment process was canceled or ended unexpectedly";
		return;
	}

	INFO << "GUI: Background subtraction adjustment for video is completed";

	QProgressDialog progress("Copying data for processing", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.setWindowTitle(QString("Adaptive Background Suppression"));
	progress.setMinimumWidth(300);

	processing_state background_subtraction_state = original;
	background_subtraction_state.details.frames_16bit.clear();
	background_subtraction_state.details.histogram_data.clear();

	// Apply background subtraction to the frames
	int number_frames = original.details.frames_16bit.size();

	progress.setMaximum(number_frames - 1);
	progress.setLabelText(QString("Adjusting frames..."));

	for (int i = 0; i < number_frames; i++) {
		DEBUG << "GUI: Applying background subtraction to " << i + 1 << " of " << number_frames << "frames";
		progress.setValue(i);
		background_subtraction_state.details.frames_16bit.push_back(AdaptiveNoiseSuppression::apply_correction(original.details.frames_16bit[i], background_correction[i]));
		if (progress.wasCanceled())
		{
			INFO << "GUI: Background subtraction process was canceled";
			return;
		}
	}

	progress.setLabelText(QString("Down-converting video and creating histogram data..."));

	// -------------------------------------------------------------------------------------
	// write description of filter
	
	QString description = "Filter starts at "; 
	if (relative_start_frame > 0)
		description += "+";

	lbl_adaptive_background_suppression->setWordWrap(true);
	description += QString::number(relative_start_frame) + " frames and averages " + QString::number(num_frames) + " frames";
	
	lbl_adaptive_background_suppression->setText(description);

	background_subtraction_state.method = Processing_Method::background_subtraction;
	background_subtraction_state.bgs_relative_start_frame = relative_start_frame;
	background_subtraction_state.bgs_num_frames = num_frames;
	video_display->container.add_processing_state(background_subtraction_state);

	slider_gain->setValue(1);
}

void SirveApp::set_color_correction_slider_labels()
{
	double lift_value = color_correction.min_convert_slider_to_value(slider_lift->value());
	double gain_value = color_correction.max_convert_slider_to_value(slider_gain->value());
		
	QString lift_string;
	lift_string.setNum(lift_value);
	lbl_lift_value->setText(lift_string);

	QString gain_string;
	gain_string.setNum(gain_value);
	lbl_gain_value->setText(gain_string);
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

	if (input)
	{
		//playback_controller->start_timer();
	}
	else
	{
		playback_controller->stop_timer();
		slider_video->setValue(1);
		lbl_video_time_midnight->setText("");
		lbl_zulu_time->setText("");
		lbl_video_frame->setText("");
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
	int length = new_epoch.size();
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
	if (min_frame < 1) {
		DEBUG << "Invalid choice for start frame. Entered: " << min_frame;
		QtHelpers::LaunchMessageBox(QString("Non-Numeric Data"), "Invalid data entered for the start frame");
		return false;
	}

	if (max_frame < 1) {
		DEBUG << "Invalid choice for end frame. Entered: " << max_frame;
		QtHelpers::LaunchMessageBox(QString("Non-Numeric Data"), "Invalid data entered for the end frame");
		return false;
	}

	if (min_frame > max_frame) {
		DEBUG << "User entered minimum frame (" << min_frame << ") is greater than maximum frame (" << max_frame << ")";
		QtHelpers::LaunchMessageBox(QString("Bad Data Entered"), "Start frame is greater than the end frame");
		return false;
	}

	int frame_stop = data_plots->full_plot_xmax + 1;

	if (max_frame > frame_stop) {
		DEBUG << "Stop frame after maximum frame. Entered: " << max_frame << "Maximum: " << frame_stop;
		QtHelpers::LaunchMessageBox(QString("Outside of Data Range"), "Data must be within valid range (1-" + QString::number(frame_stop) + ")");
		return false;
	}

	return true;
}