#include "QtGuiApplication1.h"

QtGuiApplication1::QtGuiApplication1(QWidget *parent)
	: QMainWindow(parent)
{
	
	INFO << "GUI: Initializing GUI";

	//---------------------------------------------------------------------------
	// read in configuration file 
	QString path = "config/config.json";
	QFile file(path);

	if (!file.open(QFile::ReadOnly)) {
		INFO << "GUI: Cannot open configuration file " + path.toStdString();
		INFO << "GUI: Setting maximum bit level to 14 bits";
		max_used_bits = 14;
	}
	else {
		QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
		QJsonObject jsonObj = jsonDoc.object();

		if (jsonObj.contains("max number of bits")) {
			max_used_bits = jsonObj.value("max number of bits").toInt();
			INFO << "GUI: Setting maximum bit level of image file to " << max_used_bits << " bits";
		}
		else {
			INFO << "GUI: Cannot find key 'max number of bits' in configuration file " + path.toStdString();
			INFO << "GUI: Setting maximum bit level to 14 bits";
			max_used_bits = 14;
		}
	}

	// establish object that will hold video and connect it to the playback thread
	ir_video = new Video(1, 1, max_used_bits);
	ir_video->moveToThread(&thread_video);
	QObject::connect(&thread_video, &QThread::started, ir_video, &Video::update_display_frame);

	setup_ui();

	//---------------------------------------------------------------------------

	// establish object to control playback timer and move to a new thread
	playback_controller = new Playback(1);
	playback_controller->moveToThread(&thread_timer);
	QObject::connect(&thread_video, &QThread::started, playback_controller, &Playback::start_timer);

	// establish copy copy 
	clipboard = QApplication::clipboard();
	QObject::connect(btn_copy_directory, &QPushButton::clicked, this, &QtGuiApplication1::copy_osm_directory);

	//---------------------------------------------------------------------------
	// setup container to store all videos
	eng_data = NULL;

	// default recording video to false
	record_video = false;

	int number_bits = max_used_bits;

	//---------------------------------------------------------------------------
	//---------------------------------------------------------------------------

	// links label showing the video to the video frame
	video_layout = new QGridLayout();
	video_layout->addWidget(ir_video->label);
	frame_video->setLayout(video_layout);

	
	// links chart with frame where it will be contained
	QVBoxLayout *histogram_layout = new QVBoxLayout();
	histogram_layout->addWidget(ir_video->histogram_plot->rel_chart_view);
	frame_histogram->setLayout(histogram_layout);

	// links chart with frame where it will be contained
	QVBoxLayout *histogram_abs_layout = new QVBoxLayout();
	histogram_abs_layout->addWidget(ir_video->histogram_plot->chart_view);
	frame_histogram_abs->setLayout(histogram_abs_layout);	
	
	// links chart with frame where it will be contained
	QVBoxLayout *histogram_abs_layout_full = new QVBoxLayout();
	histogram_abs_layout_full->addWidget(ir_video->histogram_plot->chart_full_view);
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
	enable_engineering_plot_options(false);

	create_menu_actions();

	this->resize(0, 0);
		
	INFO << "GUI: GUI Initialized";
}

QtGuiApplication1::~QtGuiApplication1() {

	
	delete ir_video;
	delete playback_controller;
	//delete histogram_plot;

	//delete ir_video;
	//delete histogram_plot;
	//delete data_plots;
	
	delete eng_data;

	thread_video.terminate();
	thread_timer.terminate();

	DEBUG << "GUI: GUI destructor called";

}

void QtGuiApplication1::setup_ui() {

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
	tab_menu->addTab(setup_color_correction_tab(), "Color");
	tab_menu->addTab(setup_filter_tab(), "Processing");

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
	
	// ------------------------------------------------------------------------

	this->setCentralWidget(frame_main);
	this->show();

}

QWidget* QtGuiApplication1::setup_file_import_tab() {

	
	QWidget* widget_tab_import = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_import = new QVBoxLayout(widget_tab_import);

	// ------------------------------------------------------------------------
	
	lbl_file_name = new QLabel("File Name:");
	btn_load_osm = new QPushButton("Load OSM File");
	btn_copy_directory = new QPushButton("Copy File Path");
	btn_calibration_dialog = new QPushButton("Setup Calibration");

	//btn_load_osm->setMinimumWidth(30);
	//btn_copy_directory->setMinimumWidth(30);
	
	QFrame* horizontal_segment1 = new QFrame();
	horizontal_segment1->setFrameShape(QFrame::HLine);

	QGridLayout* grid_import_file = new QGridLayout();
	grid_import_file->addWidget(lbl_file_name, 0, 0);
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

QWidget* QtGuiApplication1::setup_color_correction_tab() {

	QWidget* widget_tab_color = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_color = new QVBoxLayout(widget_tab_color);
	
	QWidget* widget_tab_color_sliders = new QWidget(tab_menu);
	QWidget* widget_tab_color_controls = new QWidget(tab_menu);
	QLabel* label_lift = new QLabel("Dark \nSet Point");
	QLabel* label_gain = new QLabel("Ligt \nSet Point");
	lbl_lift_value = new QLabel("0.0");
	lbl_gain_value = new QLabel("1.0");
	slider_lift = new QSlider();
	slider_gain = new QSlider();
	chk_relative_histogram = new QCheckBox("Relative Histogram");
	btn_reset_color_correction = new QPushButton("Reset");

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

	QLabel* lbl_colormap = new QLabel("Set Colormap:");
	cmb_color_maps = new QComboBox();

	int number_maps = ir_video->video_colors.maps.size();
	for (int i = 0; i < number_maps; i++)
		cmb_color_maps->addItem(ir_video->video_colors.maps[i].name);
	

	QHBoxLayout* hlayout_colormap = new QHBoxLayout(widget_tab_color);
	hlayout_colormap->addWidget(lbl_colormap);
	hlayout_colormap->addWidget(cmb_color_maps);
	hlayout_colormap->insertStretch(-1, 0);

	vlayout_tab_color->addLayout(hlayout_colormap);

	vlayout_tab_color->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout


	return widget_tab_color;
}

QWidget* QtGuiApplication1::setup_filter_tab() {

	QWidget* widget_tab_processing = new QWidget(tab_menu);
	QVBoxLayout* vlayout_tab_processing = new QVBoxLayout(widget_tab_processing);
	
	QLabel* label_nuc = new QLabel("Non-Uniformity Correction (NUC)");
	QLabel* label_nuc_start = new QLabel("Start Frame");
	QLabel* label_nuc_stop = new QLabel("Stop Frame");
	txt_nuc_start = new QLineEdit();
	txt_nuc_stop = new QLineEdit();
	btn_create_nuc = new QPushButton("Create NUC");
	chk_apply_nuc = new QCheckBox("Apply NUC");

	QFrame* horizontal_segment5 = new QFrame();
	horizontal_segment5->setFrameShape(QFrame::HLine);

	//QWidget* widget_tab_processing_nuc = new QWidget();
	QGridLayout* grid_tab_processing_nuc = new QGridLayout();

	grid_tab_processing_nuc->addWidget(label_nuc, 0, 0, 1, 2);
	grid_tab_processing_nuc->addWidget(label_nuc_start, 1, 0);
	grid_tab_processing_nuc->addWidget(label_nuc_stop, 1, 1);
	grid_tab_processing_nuc->addWidget(txt_nuc_start, 2, 0);
	grid_tab_processing_nuc->addWidget(txt_nuc_stop, 2, 1);
	grid_tab_processing_nuc->addWidget(btn_create_nuc, 3, 0, 1, 2);
	grid_tab_processing_nuc->addWidget(chk_apply_nuc, 4, 0, 1, 2);
	grid_tab_processing_nuc->addWidget(horizontal_segment5, 5, 0, 1, 2);

	vlayout_tab_processing->addLayout(grid_tab_processing_nuc);

	// ------------------------------------------------------------------------

	QLabel* label_background_subtraction = new QLabel("Background Subtraction");
	QLabel* label_bgs_frames = new QLabel("Number of Frames");
	txt_bgs_num_frames = new QLineEdit();
	btn_bgs = new QPushButton("Create Filter");
	chk_bgs = new QCheckBox("Apply Background Subtraction");

	QFrame* horizontal_segment6 = new QFrame();
	horizontal_segment6->setFrameShape(QFrame::HLine);

	//QWidget* widget_tab_processing_bgs = new QWidget();
	QGridLayout* grid_tab_processing_bgs = new QGridLayout();

	grid_tab_processing_bgs->addWidget(label_background_subtraction, 0, 0, 1, 2);
	grid_tab_processing_bgs->addWidget(label_bgs_frames, 1, 0);
	grid_tab_processing_bgs->addWidget(txt_bgs_num_frames, 2, 0);
	grid_tab_processing_bgs->addWidget(btn_bgs, 2, 1);
	grid_tab_processing_bgs->addWidget(chk_bgs, 3, 0, 1, 2);
	grid_tab_processing_bgs->addWidget(horizontal_segment6, 4, 0, 1, 2);

	vlayout_tab_processing->addLayout(grid_tab_processing_bgs);

	// ------------------------------------------------------------------------
	QLabel* label_deinterlace = new QLabel("De-Interlace Methods");
	cmb_deinterlace_options = new QComboBox();
	btn_deinterlace = new QPushButton("Create Filter");
	chk_deinterlace = new QCheckBox("Apply De-Interlace Method");

	QFrame* horizontal_segment7 = new QFrame();
	horizontal_segment7->setFrameShape(QFrame::HLine);

	cmb_deinterlace_options->addItem("Max Absolute Value");
	cmb_deinterlace_options->addItem("Centroid");
	cmb_deinterlace_options->addItem("Avg Cross Correlation");

	//QWidget* widget_tab_processing_deinterlace = new QWidget();
	QGridLayout* grid_tab_processing_deinterlace = new QGridLayout();

	grid_tab_processing_deinterlace->addWidget(label_deinterlace, 0, 0, 1, 2);
	grid_tab_processing_deinterlace->addWidget(cmb_deinterlace_options, 1, 0);
	grid_tab_processing_deinterlace->addWidget(btn_deinterlace, 1, 1);
	grid_tab_processing_deinterlace->addWidget(chk_deinterlace, 2, 0, 1, 2);
	grid_tab_processing_deinterlace->addWidget(horizontal_segment7, 3, 0, 1, 2);

	vlayout_tab_processing->addLayout(grid_tab_processing_deinterlace);

	// ------------------------------------------------------------------------
	btn_clear_filters = new QPushButton("Clear All Image Processing Filters");

	vlayout_tab_processing->insertStretch(-1, 0);  // inserts spacer and stretch at end of layout
	vlayout_tab_processing->addWidget(btn_clear_filters);


	return widget_tab_processing;
}

void QtGuiApplication1::setup_video_frame(){

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

	lbl_fps = new QLabel("fps");
	lbl_fps->setAlignment(Qt::AlignRight);

	//QWidget* widget_video_label_descriptions = new QWidget();
	QHBoxLayout* hlayout_video_label_description = new QHBoxLayout();

	hlayout_video_label_description->addWidget(lbl_video_frame);
	hlayout_video_label_description->addWidget(lbl_video_time_midnight);
	hlayout_video_label_description->addWidget(lbl_fps);

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
	btn_fast_forward->setToolTip("Speed Up");

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
	btn_slow_back->setToolTip("Slow Down");

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

	QPixmap menu_image("icons/menu.png");
	QIcon menu_icon(menu_image);
	btn_video_menu = new QPushButton();
	btn_video_menu->resize(button_video_width, button_video_height);
	btn_video_menu->setIcon(menu_icon);

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
	hlayout_video_buttons->addWidget(btn_fast_forward);
	hlayout_video_buttons->addWidget(btn_slow_back);
	hlayout_video_buttons->addWidget(btn_video_menu);

	vlayout_frame_video->addLayout(hlayout_video_buttons);

}

void QtGuiApplication1::setup_plot_frame() {

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
	frame_plots->enable_fixed_aspect_ratio(true);
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
	cmb_plot_xaxis->addItem(QString("Frames"));
	cmb_plot_xaxis->addItem(QString("Seconds from Midnight"));
	cmb_plot_xaxis->addItem(QString("Seconds from Epoch"));

	cmb_plot_yaxis = new QComboBox();
	cmb_plot_yaxis->addItem(QString("Irradiance"));
	cmb_plot_yaxis->addItem(QString("Azimuth"));
	cmb_plot_yaxis->addItem(QString("Elevation"));

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

void QtGuiApplication1::setup_connections() {


	//---------------------------------------------------------------------------	
	// 
	
	QObject::connect(&thread_video, &QThread::started, ir_video, &Video::update_display_frame);

	QObject::connect(&ir_video->container, &Video_Container::update_display_video, ir_video, &Video::receive_video_data);
	QObject::connect(playback_controller, &Playback::update_frame, ir_video, &Video::update_specific_frame);
	QObject::connect(&color_correction, &Min_Max_Value::update_min_max, ir_video, &Video::update_color_correction);
	QObject::connect(ir_video->histogram_plot, &HistogramLine_Plot::click_drag_histogram, this, &QtGuiApplication1::histogram_clicked);

	//---------------------------------------------------------------------------	
	
	QObject::connect(tab_menu, &QTabWidget::currentChanged, this, &QtGuiApplication1::auto_change_plot_display);
	QObject::connect(chk_relative_histogram, &QCheckBox::toggled, this, &QtGuiApplication1::toggle_relative_histogram);
	
	//---------------------------------------------------------------------------	
	// Link color correction sliders to changing color correction values
	QObject::connect(slider_gain, &QSlider::valueChanged, this, &QtGuiApplication1::gain_slider_toggled);
	QObject::connect(slider_lift, &QSlider::valueChanged, this, &QtGuiApplication1::lift_slider_toggled);

	//---------------------------------------------------------------------------	

	QObject::connect(btn_reset_color_correction, &QPushButton::clicked, this, &QtGuiApplication1::reset_color_correction);
	QObject::connect(cmb_color_maps, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtGuiApplication1::edit_color_map);
	
	//---------------------------------------------------------------------------

	QObject::connect(cmb_deinterlace_options, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtGuiApplication1::toggle_video_filters);

	//---------------------------------------------------------------------------

	// Link horizontal slider to playback controller
	QObject::connect(playback_controller, &Playback::update_frame, slider_video, &QSlider::setValue);
	QObject::connect(slider_video, &QSlider::valueChanged, playback_controller, &Playback::set_counter);

	//---------------------------------------------------------------------------

	// Link playback to play controls
	QObject::connect(btn_play, &QPushButton::clicked, playback_controller, &Playback::start_timer);
	QObject::connect(btn_pause, &QPushButton::clicked, playback_controller, &Playback::stop_timer);
	QObject::connect(btn_reverse, &QPushButton::clicked, playback_controller, &Playback::reverse);


	QObject::connect(btn_fast_forward, &QPushButton::clicked, playback_controller, &Playback::speed_timer);
	QObject::connect(btn_slow_back, &QPushButton::clicked, playback_controller, &Playback::slow_timer);
	QObject::connect(btn_next_frame, &QPushButton::clicked, playback_controller, &Playback::next_frame);
	QObject::connect(btn_prev_frame, &QPushButton::clicked, playback_controller, &Playback::prev_frame);
	QObject::connect(btn_frame_record, &QPushButton::clicked, this, &QtGuiApplication1::start_stop_video_record);

	QObject::connect(btn_fast_forward, &QPushButton::clicked, this, &QtGuiApplication1::update_fps);
	QObject::connect(btn_slow_back, &QPushButton::clicked, this, &QtGuiApplication1::update_fps);

	QObject::connect(btn_zoom, &QPushButton::clicked, this, &QtGuiApplication1::toggle_zoom_on_video);
	QObject::connect(btn_calculate_radiance, &QPushButton::clicked, this, &QtGuiApplication1::toggle_calculation_on_video);

	//---------------------------------------------------------------------------

	//Link buttons to functions
	QObject::connect(btn_load_osm, &QPushButton::clicked, this, &QtGuiApplication1::load_osm_data);
	QObject::connect(btn_calibration_dialog, &QPushButton::clicked, this, &QtGuiApplication1::show_calibration_dialog);
	QObject::connect(btn_get_frames, &QPushButton::clicked, this, &QtGuiApplication1::load_abir_data);
	QObject::connect(txt_end_frame, &QLineEdit::returnPressed, this, &QtGuiApplication1::load_abir_data);

	QObject::connect(btn_create_nuc, &QPushButton::clicked, this, &QtGuiApplication1::create_non_uniformity_correction);
	QObject::connect(txt_nuc_stop, &QLineEdit::returnPressed, this, &QtGuiApplication1::create_non_uniformity_correction);

	QObject::connect(btn_bgs, &QPushButton::clicked, this, &QtGuiApplication1::create_background_subtraction_correction);
	QObject::connect(txt_bgs_num_frames, &QLineEdit::returnPressed, this, &QtGuiApplication1::create_background_subtraction_correction);

	QObject::connect(btn_deinterlace, &QPushButton::clicked, this, &QtGuiApplication1::create_deinterlace);

	QObject::connect(chk_apply_nuc, &QCheckBox::stateChanged, this, &QtGuiApplication1::toggle_video_filters);
	QObject::connect(chk_bgs, &QCheckBox::stateChanged, this, &QtGuiApplication1::toggle_video_filters);
	QObject::connect(chk_deinterlace, &QCheckBox::stateChanged, this, &QtGuiApplication1::toggle_video_filters);

	//---------------------------------------------------------------------------

	// Connect epoch button click to function
	QObject::connect(btn_apply_epoch, &QPushButton::clicked, this, &QtGuiApplication1::apply_epoch_time);

	//---------------------------------------------------------------------------

	//Enable clearing image processing filters
	QObject::connect(btn_clear_filters, &QPushButton::clicked, this, &QtGuiApplication1::clear_image_processing);

	//Enable saving frame
	QObject::connect(btn_frame_save, &QPushButton::clicked, this, &QtGuiApplication1::save_frame);

	//Update frame number label
	QObject::connect(playback_controller, &Playback::update_frame, this, &QtGuiApplication1::set_frame_number_label);


	//---------------------------------------------------------------------------	
	// Connect x-axis and y-axis changes to functions
	connect(cmb_plot_yaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtGuiApplication1::plot_change);
	connect(cmb_plot_xaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtGuiApplication1::plot_change);

	// Connect save button functions
	connect(btn_save_plot, &QPushButton::clicked, this, &QtGuiApplication1::save_plot);

	//---------------------------------------------------------------------------
	// connect the plot radial buttons to adjust plot

	connect(rad_log, &QRadioButton::toggled, this, &QtGuiApplication1::yaxis_log_toggled);
	connect(rad_decimal, &QRadioButton::toggled, this, &QtGuiApplication1::yaxis_decimal_toggled);
	connect(rad_linear, &QRadioButton::toggled, this, &QtGuiApplication1::yaxis_linear_toggled);
	connect(rad_scientific, &QRadioButton::toggled, this, &QtGuiApplication1::yaxis_scientific_toggled);

	//---------------------------------------------------------------------------

}

void QtGuiApplication1::load_osm_data()
{

	INFO << "GUI: Start OSM load process";

	txt_start_frame->setEnabled(false);
	txt_end_frame->setEnabled(false);
	btn_get_frames->setEnabled(false);
	btn_load_osm->setEnabled(false);
	btn_copy_directory->setEnabled(false);
	btn_calibration_dialog->setEnabled(false);

	INFO << "GUI: Loading OSM data";
	int valid_files = file_data.load_osm_file();
	
	if (valid_files != 1)
	{		
		INFO << "GUI: No file selected for load";

		btn_load_osm->setEnabled(true);
		btn_copy_directory->setEnabled(true);

		if (eng_data != NULL) {
			// if eng_data already initialized, allow user to re-select frames
			txt_start_frame->setEnabled(true);
			txt_end_frame->setEnabled(true);
			btn_get_frames->setEnabled(true);
			btn_calibration_dialog->setEnabled(true);
		}

		QString error_status;
		switch (valid_files)
		{
		case 2:
			error_status = "No image file was selcted";
			break;
		case 3:
			error_status = "Image file issue";
			break;
		case 4: 
			error_status = "No OSM file found that matches the image file name";
			break;
		case 5:
			error_status = "Error while reading OSM file. Close program and open logs for details";
			break;
		default:
			return;
			break;
		}
		
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Issue Loading File"));
		QString box_text = error_status;
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();

		return;
	}
			
	INFO << "GUI: OSM file has valid path";

	lbl_file_load->setText(file_data.info_msg);
	lbl_file_name->setText(file_data.file_name);
	lbl_file_name->setToolTip(file_data.directory_path);

	if (valid_files) {
		txt_start_frame->setEnabled(true);
		txt_end_frame->setEnabled(true);
		btn_get_frames->setEnabled(true);

		QString osm_max_frames = QString::number(file_data.osm_data.num_messages);
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
			clear_image_processing();

			// delete objects with existing data within them
			delete eng_data;
			delete data_plots;
			delete engineering_plot_layout;			

			ir_video->container.something.clear();
			ir_video->remove_frame();

			clear_frame_label();

			video_layout->addWidget(ir_video->label);
			frame_video->setLayout(video_layout);

		}

		DEBUG << "GUI: Creating new objects for engineering data, data plots, and layout";

		eng_data = new Engineering_Data(file_data.osm_data.data);
		data_plots = new Engineering_Plots();

		data_plots->frame_numbers = eng_data->frame_numbers;
		data_plots->past_midnight = eng_data->seconds_from_midnight;
		data_plots->past_epoch = eng_data->seconds_from_epoch;
		data_plots->index_sub_plot_xmin = 0;
		data_plots->index_sub_plot_xmax = data_plots->frame_numbers.size() - 1;

		data_plots->track_irradiance_data = eng_data->track_irradiance_data;
				
		//--------------------------------------------------------------------------------
		int num_tracks = data_plots->track_irradiance_data.size();
		if (num_tracks == 0)
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle(QString("No Tracking Data"));
			QString box_text = "No tracking data was found within the file. No data will be plotted.";
			msgBox.setText(box_text);

			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setDefaultButton(QMessageBox::Ok);
			msgBox.exec();
		}

		//--------------------------------------------------------------------------------
		// Enable setting of epoch
		dt_epoch->setEnabled(true);
		btn_apply_epoch->setEnabled(true);
				
		std::vector<double> epoch0 = eng_data->get_epoch();
		std::vector<double> epoch_min = eng_data->get_adj_epoch(-2);
		std::vector<double> epoch_max = eng_data->get_adj_epoch(2);
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

		//--------------------------------------------------------------------------------
		
		// Reset settings on video playback to defaults
		menu_osm->setIconVisibleInMenu(false);
		ir_video->toggle_osm_tracks(false);

		menu_add_primary_data->setIconVisibleInMenu(false);
		ir_video->toggle_primary_track_data(false);

		menu_sensor_boresight->setIconVisibleInMenu(false);
		ir_video->toggle_sensor_boresight_data(false);

		btn_calculate_radiance->setChecked(false);
		btn_calculate_radiance->setEnabled(false);

		CalibrationData temp;
		calibration_model = temp;

		ir_video->banner_color = QString("red");
		ir_video->banner_text = QString("EDIT CLASSIFICATION");
		ir_video->tracker_color = QString("red");
		
		// Reset setting engineering plot defaults
		data_plots->plot_all_data = true;
		menu_plot_all_data->setIconVisibleInMenu(true);
		
		data_plots->plot_primary_only = false;
		menu_plot_primary->setIconVisibleInMenu(false);

		data_plots->plot_current_marker = false;
		menu_plot_frame_marker->setIconVisibleInMenu(false);
		
		enable_engineering_plot_options(true);
		data_plots->set_plot_title(QString("EDIT CLASSIFICATION"));

		INFO << "GUI: OSM successfully loaded";
	}

	btn_load_osm->setEnabled(true);
	btn_copy_directory->setEnabled(true);

	return;
}

void QtGuiApplication1::load_abir_data()
{

	btn_get_frames->setEnabled(false);

	INFO << "GUI: Starting ABIR load process";

	// -----------------------------------------------------------------------------------------

	// Get frame numbers from text boxes
	int min_frame = get_integer_from_txt_box(txt_start_frame->text());
	int max_frame = get_integer_from_txt_box(txt_end_frame->text());

	bool check_data = check_min_max_frame_input(min_frame, max_frame);

	if (!check_data) {
		btn_get_frames->setEnabled(true);

		INFO << "GUI: No video loaded";
		return;
	}

	//-----------------------------------------------------------------------------------

	DEBUG << "GUI: Frame numbers are valid";

	//---------------------------------------------------------------------------

	// Load the ABIR data
	playback_controller->stop_timer();
	ir_video->container.something.clear();

	// Create the video properties data
	video_details primary;
	primary.properties[Video_Parameters::original] = true;

	INFO << "GUI: Reading in video data";
	primary.set_number_of_bits(max_used_bits);

	//----------------------------------------------------------------------------
	QString path = "config/config.json";
	QFile file(path);
	double version = 0;

	if (!file.open(QFile::ReadOnly)) {
		INFO << "GUI: Cannot open configuration file " + path.toStdString();
		INFO << "GUI: Version file being set on loading of image data";
	}
	else {
		QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
		QJsonObject jsonObj = jsonDoc.object();

		if (jsonObj.contains("version")) {
			version = jsonObj.value("version").toDouble();
			INFO << "GUI: Overriding version of image file to " << version;
		}
		else {
			INFO << "GUI: Cannot find key 'version' in configuration file " + path.toStdString();
			INFO << "GUI: Version file being set on loading of image data";
		}
	}
	//----------------------------------------------------------------------------

	std::vector<std::vector<uint16_t>> video_frames = file_data.load_image_file(min_frame, max_frame, version);

	if (video_frames.size() == 0) {

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("File Version Not Within Range"));
		QString box_text = "File version was not within valid range. See log for more details";
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();

		INFO << "GUI: Video import stopped. File version not within range";
		btn_get_frames->setEnabled(true);
		return;
	}

	int x_pixels = file_data.abir_data.ir_data[0].header.image_x_size;
	int y_pixels = file_data.abir_data.ir_data[0].header.image_y_size;

	DEBUG << "GUI: Frames are of size " << x_pixels << " x " << y_pixels;

	ir_video->clear_all_zoom_levels(x_pixels, y_pixels);
	primary.set_image_size(x_pixels, y_pixels);
	primary.set_video_frames(video_frames);
	ir_video->container.something.push_back(primary);

	frame_video->setMinimumHeight(y_pixels);
	frame_video->setMinimumWidth(x_pixels);

	int number_frames = primary.frames_16bit.size();
	QString status_txt = lbl_file_load->text();
	QString update_text("\nFrames ");
	update_text.append(QString::number(min_frame));
	update_text.append(" to ");
	update_text.append(QString::number(min_frame + number_frames - 1));
	update_text.append(" were loaded");

	status_txt.append(update_text);
	lbl_file_load->setText(status_txt);

	//---------------------------------------------------------------------------
	// Set frame number for playback controller and valid values for slider
	playback_controller->set_number_of_frames(number_frames);
	slider_video->setRange(0, number_frames);

	//---------------------------------------------------------------------------
	// Set the video and histogram plots to this data
	ir_video->container.display_original_data();

	
	// Start threads...
	if (!thread_timer.isRunning())
	{
		thread_video.start();
		thread_timer.start();
	}

	std::vector<Plotting_Frame_Data>::const_iterator first = eng_data->frame_data.begin() + min_frame - 1;
	std::vector<Plotting_Frame_Data>::const_iterator last = eng_data->frame_data.begin() + (min_frame) + (max_frame - min_frame);
	std::vector<Plotting_Frame_Data> subset_data(first, last);
	ir_video->set_frame_data(subset_data, file_data.abir_data.ir_data);

	// Reset engineering plots with new sub plot indices
	data_plots->index_sub_plot_xmin = min_frame - 1;
	data_plots->index_sub_plot_xmax = max_frame - 1;
	plot_change(1);

	//Update frame marker on engineering plot
	QObject::connect(playback_controller, &Playback::update_frame, data_plots, &Engineering_Plots::plot_current_step);
	
	playback_controller->set_speed_index(10);
	update_fps();

	tab_plots->setCurrentIndex(1);
	plot_full_data();
	plot_current_frame_marker();

	btn_get_frames->setEnabled(true);
	btn_calibration_dialog->setEnabled(true);

	tab_menu->setTabEnabled(1, true);
	tab_menu->setTabEnabled(2, true);

	INFO << "GUI: ABIR file load complete";

	clear_image_processing();

	toggle_video_playback_options(true);
	playback_controller->start_timer();

	//---------------------------------------------------------------------------
}

void QtGuiApplication1::start_stop_video_record()
{

	if (record_video)
	{
		//Stopping record video

		ir_video->stop_recording();

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
		bool file_opened = ir_video->start_recording(playback_controller->get_fps());

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
			QMessageBox msgBox;
			msgBox.setWindowTitle(QString("Video Record Failed "));
			msgBox.setText("Video file could not be saved to this location");

			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setDefaultButton(QMessageBox::Ok);
			msgBox.exec();
		}

	}

}


void QtGuiApplication1::toggle_zoom_on_video() {

	bool status_zoom_btn = btn_zoom->isChecked();
	
	if (status_zoom_btn)
	{
		ir_video->toggle_action_zoom(true);
		btn_calculate_radiance->setChecked(false);
	}
	else {
		ir_video->toggle_action_zoom(false);
	}

}


void QtGuiApplication1::toggle_calculation_on_video()
{

	bool status_calculation_btn = btn_calculate_radiance->isChecked();

	if (status_calculation_btn) {

		ir_video->toggle_action_calculate_radiance(true);
		btn_zoom->setChecked(false);
	}
	else {
		ir_video->toggle_action_calculate_radiance(false);
	}
}

void QtGuiApplication1::update_fps()
{
	QString fps = QString::number(playback_controller->speeds[playback_controller->index_speed], 'g', 2);
	fps.append(" fps");

	lbl_fps->setText(fps);
}


void QtGuiApplication1::histogram_clicked(double x0, double x1) {
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


void QtGuiApplication1::lift_slider_toggled(int value) {

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

void QtGuiApplication1::gain_slider_toggled(int value) {

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

void QtGuiApplication1::color_correction_toggled(double lift_value, double gain_value) {
		
	emit color_correction.update_min_max(lift_value, gain_value);
	
	set_color_correction_slider_labels();

	if (!playback_controller->timer->isActive() && file_data.valid_image)
	{
		int counter_value = playback_controller->get_counter();
		ir_video->update_specific_frame(counter_value);
		//histogram_plot->update_specific_histogram(counter_value);
	}		

	DEBUG << "GUI: New values set for Lift/Gamma/Gain correction: " << std::to_string(lift_value) << "/" << "/" << std::to_string(gain_value);
}

void QtGuiApplication1::reset_color_correction()
{
		
	slider_lift->setValue(0);
	slider_gain->setValue(1000);
	chk_relative_histogram->setChecked(false);

	DEBUG << "GUI: Color correction reset";
}


void QtGuiApplication1::plot_full_data()
{
	bool current_state = data_plots->plot_all_data;
	bool new_state = !current_state;

	data_plots->plot_all_data = new_state;
	data_plots->toggle_subplot();
	menu_plot_all_data->setIconVisibleInMenu(new_state);

	DEBUG << "GUI: Plot all data changed from " << current_state << " to " << new_state;
	
}

void QtGuiApplication1::plot_primary_only()
{
	bool current_state = data_plots->plot_primary_only;
	bool new_state = !current_state;

	data_plots->plot_primary_only = new_state;
	menu_plot_primary->setIconVisibleInMenu(new_state);

	plot_change(1);

	DEBUG << "GUI: Plot primary data changed from " << current_state << " to " << new_state;
}

void QtGuiApplication1::plot_current_frame_marker() {

	bool current_state = data_plots->plot_current_marker;
	bool new_state = !current_state;

	data_plots->plot_current_marker = new_state;
	menu_plot_frame_marker->setIconVisibleInMenu(new_state);

	if (ir_video->number_of_frames > 0)
	{
		plot_change(1);
		emit data_plots->plot_current_step(playback_controller->get_counter());
	}
	
	DEBUG << "GUI: Show video marker changed from " << current_state << " to " << new_state;
}

void QtGuiApplication1::auto_change_plot_display(int index)
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

void QtGuiApplication1::show_calibration_dialog()
{
	CalibrationDialog calibrate_dialog(calibration_model);
	
	auto response = calibrate_dialog.exec();

	if (response == 0) {

		return;
	}

	calibration_model = calibrate_dialog.model;
	ir_video->set_calibration_model(calibrate_dialog.model);
	btn_calculate_radiance->setEnabled(true);
}

void QtGuiApplication1::save_plot()
{
	data_plots->save_plot();
	INFO << "GUI: Plot saved";
}

void QtGuiApplication1::save_frame()
{
	if(playback_controller->is_running())
		playback_controller->stop_timer();
	
	ir_video->save_frame();
	INFO << "GUI: Video frame saved  ";

	if(playback_controller->is_running())
		playback_controller->start_timer();
}

void QtGuiApplication1::create_menu_actions()
{
	QIcon on("icons/check.png");

	// ------------------------- VIDEO MENU ACTIONS -------------------------

	menu_osm = new QAction(tr("&Toggle OSM Tracks"), this);
	menu_osm->setIcon(on);
	menu_osm->setStatusTip(tr("Shows pixels OSM is tracking"));
	menu_osm->setIconVisibleInMenu(false);
	connect(menu_osm, &QAction::triggered, this, &QtGuiApplication1::toggle_osm_tracks);

	menu_add_primary_data = new QAction(tr("&Toggle Primary Tgt Metrics"), this);
	menu_add_primary_data->setIcon(on);
	menu_add_primary_data->setStatusTip(tr("Shows Az/El of Primary Tgt"));
	menu_add_primary_data->setIconVisibleInMenu(false);
	connect(menu_add_primary_data, &QAction::triggered, this, &QtGuiApplication1::toggle_primary_track_data);

	menu_sensor_boresight = new QAction(tr("&Toggle Sensor Metrics"), this);
	menu_sensor_boresight->setIcon(on);
	menu_sensor_boresight->setStatusTip(tr("Shows Az/El of Sensor Pointing"));
	menu_sensor_boresight->setIconVisibleInMenu(false);
	connect(menu_sensor_boresight, &QAction::triggered, this, &QtGuiApplication1::toggle_sensor_track_data);

	menu_add_banner = new QAction(tr("&Edit Banner Text"), this);
	menu_add_banner->setStatusTip(tr("Edit banner text to video"));
	menu_add_banner->setIconVisibleInMenu(false);
	connect(menu_add_banner, &QAction::triggered, this, &QtGuiApplication1::edit_banner_text);
	connect(this, &QtGuiApplication1::change_banner, ir_video, &Video::update_banner_text);
	
	menu_change_color_banner = new QAction(tr("&Change Text Color"), this);
	menu_change_color_banner->setStatusTip(tr("Change frame text color"));
	connect(menu_change_color_banner, &QAction::triggered, this, &QtGuiApplication1::edit_banner_color);
	connect(this, &QtGuiApplication1::change_banner_color, ir_video, &Video::update_banner_color);

	menu_change_color_tracker = new QAction(tr("&Change Tracker Color"), this);
	menu_change_color_tracker->setStatusTip(tr("Change color of tracking boxes "));
	connect(menu_change_color_tracker, &QAction::triggered, this, &QtGuiApplication1::edit_tracker_color);
	connect(this, &QtGuiApplication1::change_tracker_color, ir_video, &Video::update_tracker_color);

	menu_change_color_map = new QAction(tr("&Change Color Map"), this);
	menu_change_color_map->setStatusTip(tr("Change color map applied to frames "));
	connect(menu_change_color_map, &QAction::triggered, this, &QtGuiApplication1::edit_color_map);
	
	menu_annotate = new QAction(tr("&Annotate Video"), this);
	menu_annotate->setStatusTip(tr("Add text to video display "));
	connect(menu_annotate, &QAction::triggered, this, &QtGuiApplication1::annotate_video);
	
	// ------------------------- PLOT MENU ACTIONS -------------------------

	menu_plot_all_data = new QAction(tr("&Plot all frame data"), this);
	menu_plot_all_data->setIcon(on);
	menu_plot_all_data->setStatusTip(tr("Plot all data from OSM file"));
	menu_plot_all_data->setIconVisibleInMenu(true);
	connect(menu_plot_all_data, &QAction::triggered, this, &QtGuiApplication1::plot_full_data);

	menu_plot_primary = new QAction(tr("&Plot Primary Data Only"), this);
	menu_plot_primary->setIcon(on);
	menu_plot_primary->setStatusTip(tr("Plot only the primary object"));
	menu_plot_primary->setIconVisibleInMenu(false);
	connect(menu_plot_primary, &QAction::triggered, this, &QtGuiApplication1::plot_primary_only);
	
	menu_plot_frame_marker = new QAction(tr("&Plot Marker for Current Frame"), this);
	menu_plot_frame_marker->setIcon(on);
	menu_plot_frame_marker->setStatusTip(tr("Plot marker to show current video frame"));
	menu_plot_frame_marker->setIconVisibleInMenu(false);
	connect(menu_plot_frame_marker, &QAction::triggered, this, &QtGuiApplication1::plot_current_frame_marker);

	menu_plot_edit_banner = new QAction(tr("&Edit Banner Text"), this);
	menu_plot_edit_banner->setStatusTip(tr("Edit the banner text for the plot"));
	connect(menu_plot_edit_banner, &QAction::triggered, this, &QtGuiApplication1::edit_plot_text);

	menu_plot_edit_banner = new QAction(tr("&Export Tracking Data"), this);
	menu_plot_edit_banner->setStatusTip(tr("Export the plotted data to file"));
	connect(menu_plot_edit_banner, &QAction::triggered, this, &QtGuiApplication1::export_plot_data);

	// ---------------------- Set Acctions to Menu --------------------

	menu = new QMenu(this);
	menu->addAction(menu_osm);
	menu->addAction(menu_add_primary_data);
	menu->addAction(menu_sensor_boresight);
	menu->addAction(menu_add_banner);
	menu->addAction(menu_change_color_banner);
	menu->addAction(menu_change_color_tracker);
	//menu->addAction(menu_change_color_map);
	menu->addAction(menu_annotate);

	plot_menu = new QMenu(this);
	plot_menu->addAction(menu_plot_all_data);
	plot_menu->addAction(menu_plot_primary);
	plot_menu->addAction(menu_plot_frame_marker);
	plot_menu->addAction(menu_plot_edit_banner);

	btn_video_menu->setMenu(menu);
	btn_plot_menu->setMenu(plot_menu);


}

void QtGuiApplication1::edit_banner_text()
{	
	bool ok;
	QString input_text = QInputDialog::getText(0, "Banner Text", "Input Banner Text", QLineEdit::Normal, ir_video->banner_text, &ok);
	
	if (ok) {

		emit change_banner(input_text);
		DEBUG << "GUI: Banner text changed";

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

void QtGuiApplication1::edit_plot_text()
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

void QtGuiApplication1::export_plot_data()
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
		min_frame = data_plots->index_sub_plot_xmin;
		max_frame = data_plots->index_sub_plot_xmax;

		eng_data->write_track_date_to_csv(save_path, min_frame, max_frame);
	}

	QMessageBox msgBox;
	msgBox.setWindowTitle("Export Data");
	msgBox.setText("Successfully exported track data to file");
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.exec();
}

int QtGuiApplication1::get_color_index(QVector<QString> colors, QColor input_color) {

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


void QtGuiApplication1::edit_color_map()
{
	/*
	QVector<QString>color_maps{};
	int number_maps = ir_video->video_colors.maps.size();

	for (int i = 0; i < number_maps; i++)
		color_maps.append(ir_video->video_colors.maps[i].name);

	int initial_index = ir_video->index_video_color;
	
	CustomInputDialog color_map_dialog(color_maps, QString("Update Colormap"), QString("Available Colormaps:"), initial_index);
	connect(color_map_dialog.cmb_options, &QComboBox::currentTextChanged, ir_video, &Video::update_color_map);

	auto response = color_map_dialog.exec();

	if (response == 0) {

		ir_video->update_color_map(color_maps[initial_index]);

		return;
	}
	*/

	int index = cmb_color_maps->currentIndex();
	QString color = cmb_color_maps->currentText();
	ir_video->update_color_map(color);

}

void QtGuiApplication1::edit_banner_color()
{
	
	QVector<QString>colors{};

	colors.append("red");
	colors.append("orange");
	colors.append("yellow");
	colors.append("green");
	colors.append("blue");
	colors.append("violet");
	colors.append("black");
	colors.append("white");

	int initial_index = get_color_index(colors, ir_video->banner_color);

	CustomInputDialog banner_color_dialog(colors, QString("Update Banner Colors"), QString("Available Colors:"), initial_index);
	connect(banner_color_dialog.cmb_options, &QComboBox::currentTextChanged, ir_video, &Video::update_banner_color);

	auto response = banner_color_dialog.exec();

	if (response == 0) {

		ir_video->update_banner_color(colors[initial_index]);

		return;
	}
}

void QtGuiApplication1::edit_tracker_color()
{

	QVector<QString> colors{};
	colors.append("red");
	colors.append("orange");
	colors.append("yellow");
	colors.append("green");
	colors.append("blue");
	colors.append("violet");
	colors.append("black");
	colors.append("white");

	int initial_index = get_color_index(colors, ir_video->tracker_color);

	CustomInputDialog tracker_color_dialog(colors, QString("Update Tracker Colors"), QString("Available Colors:"), initial_index);
	connect(tracker_color_dialog.cmb_options, &QComboBox::currentTextChanged, ir_video, &Video::update_tracker_color);

	auto response = tracker_color_dialog.exec();

	if (response == 0) {

		ir_video->update_tracker_color(colors[initial_index]);

		return;
	}

}

void QtGuiApplication1::plot_change(int index)
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
			data_plots->x_axis_units = frames;
			break;

		case 1:
			data_plots->x_axis_units = seconds_past_midnight;
			break;

		case 2:
			data_plots->x_axis_units = seconds_from_epoch;
			break;

		default:
			break;
		}


		switch (y_index)
		{
		case 0:
			data_plots->plot_irradiance(eng_data->max_number_tracks);
			break;

		case 1:
			data_plots->plot_azimuth();
			break;

		case 2:
			data_plots->plot_elevation();
			break;

		default:
			break;
		}

		data_plots->plot_current_step(playback_controller->get_counter());
	}

}

void QtGuiApplication1::annotate_video()
{
	video_info standard_info;
	standard_info.x_pixels = ir_video->image_x;
	standard_info.y_pixels = ir_video->image_y;

	int index = data_plots->index_sub_plot_xmin;
	int min_frame = eng_data->frame_numbers[index];
	int num_frames = playback_controller->get_max_counter();

	standard_info.min_frame = min_frame;
	standard_info.max_frame = min_frame + num_frames;

	Annotations annotate_gui(ir_video->annotation_list, standard_info, ir_video);
	annotate_gui.exec();
}

void QtGuiApplication1::yaxis_log_toggled(bool input)
{
	if (input)
	{
		plot_change(1);
	}
	
}

void QtGuiApplication1::yaxis_linear_toggled(bool input)
{
	if (input)
	{
		plot_change(1);
	}
}

void QtGuiApplication1::yaxis_decimal_toggled(bool input)
{
	if (input)
	{
		plot_change(1);
	}
}

void QtGuiApplication1::yaxis_scientific_toggled(bool input)
{
	if (input)
	{
		plot_change(1);
	}
}

int QtGuiApplication1::get_integer_from_txt_box(QString input)
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

bool QtGuiApplication1::check_value_within_range(int input_value, int min_value, int max_value)
{
	if (input_value < min_value) {
		DEBUG << "GUI: " << input_value << "is less than min value of " << min_value;
		return false;
	}
	if (input_value > max_value) {
		DEBUG << "GUI: " << input_value << "is more than max value of " << max_value;
		return false;
	}
		
	DEBUG << "GUI: " << input_value << " is between " << min_value << " and " << max_value;

	return true;
}

void QtGuiApplication1::set_frame_number_label(int counter)
{
	// check that engineering is non-null before accessing
	if (eng_data) {
		int index = data_plots->index_sub_plot_xmin;

		int frame_number = eng_data->frame_numbers[index + counter];
		QString frame_text("Frame # ");
		frame_text.append(QString::number(frame_number));
		lbl_video_frame->setText(frame_text);

		double seconds_midnight = eng_data->seconds_from_midnight[index + counter];
		QString seconds_text("From Midnight ");
		seconds_text.append(QString::number(seconds_midnight, 'g', 8));
		lbl_video_time_midnight->setText(seconds_text);
	}

}

void QtGuiApplication1::clear_frame_label()
{

	QString frame_text("");
	lbl_video_frame->setText(frame_text);

	QString seconds_text("");
	lbl_video_time_midnight->setText(seconds_text);

}


void QtGuiApplication1::copy_osm_directory()
{
	clipboard->setText(file_data.osm_path);
}

void QtGuiApplication1::update_enhanced_range(bool input)
{

	emit enhanced_dynamic_range(input);

}

void QtGuiApplication1::toggle_relative_histogram(bool input)
{
	if (input) {

		ir_video->show_relative_histogram = true;

		stacked_layout_histograms->setCurrentIndex(1);
		ir_video->update_display_frame();

	}
	else
	{
		ir_video->show_relative_histogram = false;
		stacked_layout_histograms->setCurrentIndex(0);

	}
}

void QtGuiApplication1::apply_epoch_time()
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
	
	data_plots->past_epoch = eng_data->seconds_from_epoch;
	data_plots->track_irradiance_data = eng_data->track_irradiance_data;
	plot_change(0);
	
}

void QtGuiApplication1::create_non_uniformity_correction()
{
		
	INFO << "GUI: Creating non-uniformity correction file from original data";
		
	//----------------------------------------------------------------------------------------------------
	// Get frame numbers from text boxes
	int min_frame = get_integer_from_txt_box(txt_nuc_start->text());
	int max_frame = get_integer_from_txt_box(txt_nuc_stop->text());

	bool check_data = check_min_max_frame_input(min_frame, max_frame);

	if (!check_data) {
		
		INFO << "GUI: NUC correction not completed";
		return;
	}

	//----------------------------------------------------------------------------------------------------

	NUC nuc(file_data.image_path, min_frame, max_frame, file_data.file_version);
	std::vector<double> nuc_correction = nuc.get_nuc_correction();

	if (nuc_correction.size() == 0)
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("File Version Not Within Range"));
		QString box_text = "File version was not within valid range. See log for more details";
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();

		INFO << "GUI: NUC correction not completed";
		return;
	}

	INFO << "Calculated NUC correction";

	video_details nuc_video;
	video_details current_state = get_current_filter_state();
	int index_current_state = ir_video->container.find_data_index(current_state);

	video_details original = ir_video->container.something[index_current_state];

	nuc_video = original;
	nuc_video.clear_16bit_vector();
	nuc_video.clear_8bit_vector();
	nuc_video.histogram_data.clear();
	
	nuc_video.properties[Video_Parameters::original] = false;
	nuc_video.properties[Video_Parameters::non_uniformity_correction] = true;

	// Apply NUC to the frames		
	int number_frames = original.frames_16bit.size();

	QProgressDialog progress("", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.setWindowTitle(QString("Non-Uniformity Correction"));
		
	progress.setMinimum(0);
	progress.setMaximum(number_frames - 1);
	progress.setLabelText(QString("Applying non-uniformity correction..."));

	progress.setMinimumWidth(300);

	for (int i = 0; i < number_frames; i++) {
		progress.setValue(i);
		DEBUG << "GUI: Applying NUC correction to " << i + 1 << " of " << number_frames << " frames";
		nuc_video.frames_16bit.push_back(nuc.apply_nuc_correction(original.frames_16bit[i]));
		if (progress.wasCanceled())
			break;
	}

	if (progress.wasCanceled()) {
		INFO << "GUI: NUC process was canceled";
		return;
	}

	progress.setLabelText(QString("Down-converting video and creating histogram data..."));

	//nuc_video.convert_16bit_to_8bit();
	//nuc_video.create_histogram_data();		

	bool nuc_exists = false;
	int index_nuc = ir_video->container.find_data_index(nuc_video);
	if (index_nuc > 0) {
		nuc_exists = true;
		ir_video->container.something[index_nuc] = nuc_video;

		INFO << "GUI: Previous NUC video was replaced";
	}
	else
	{
		ir_video->container.something.push_back(nuc_video);
		INFO << "GUI: NUC video added";
	}
	

	show_available_filter_options();
}

void QtGuiApplication1::create_deinterlace()
{
	INFO << "GUI: Creating de-interlace file from original data";
		
	deinterlace_type deinterlace_method_type = find_deinterlace_type(cmb_deinterlace_options->currentIndex());
	Video_Parameters deinterlace_video_type = find_deinterlace_video_type(cmb_deinterlace_options->currentIndex());
	
	video_details deinterlace_video;
	video_details current_state = get_current_filter_state();
	int index_current_state = ir_video->container.find_data_index(current_state);

	video_details original = ir_video->container.something[index_current_state];

	DEBUG << "GUI: Found de-interlacing method type and video type";
	Deinterlace deinterlace_method(deinterlace_method_type, original.x_pixels, original.y_pixels);

	deinterlace_video = original;
	deinterlace_video.clear_16bit_vector();
	deinterlace_video.clear_8bit_vector();
	deinterlace_video.histogram_data.clear();

	deinterlace_video.properties[Video_Parameters::original] = false;
	deinterlace_video.properties[deinterlace_video_type] = true;
	
	// Apply de-interlace to the frames		
	
	int number_frames = original.frames_16bit.size();

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
		deinterlace_video.frames_16bit.push_back(deinterlace_method.deinterlace_frame(original.frames_16bit[i]));
		if (progress.wasCanceled())
			break;
	}

	if (progress.wasCanceled()) {
		INFO << "De-interlace process was canceled";
		return;
	}

	
	//deinterlace_video.convert_16bit_to_8bit();
	//deinterlace_video.create_histogram_data();

	bool deinterlace_exists = false;
	int index_deinterlace = ir_video->container.find_data_index(deinterlace_video);
	if (index_deinterlace > 0) {
		deinterlace_exists = true;
		ir_video->container.something[index_deinterlace] = deinterlace_video;
		INFO << "GUI: Previous de-interlace video was replaced";
	}
	else
	{
		ir_video->container.something.push_back(deinterlace_video);
		INFO << "GUI: De-interlace video was added";
	}

	show_available_filter_options();
	
}

void QtGuiApplication1::toggle_osm_tracks()
{
	bool current_status = menu_osm->isIconVisibleInMenu();
	
	if (current_status) {
		menu_osm->setIconVisibleInMenu(false);
	}
	else {
		menu_osm->setIconVisibleInMenu(true);
	}
	
	ir_video->toggle_osm_tracks(!current_status);
	ir_video->update_display_frame();

}

void QtGuiApplication1::toggle_primary_track_data()
{

	bool current_status = menu_add_primary_data->isIconVisibleInMenu();

	if (current_status) {
		menu_add_primary_data->setIconVisibleInMenu(false);
	}
	else {
		menu_add_primary_data->setIconVisibleInMenu(true);
	}

	ir_video->toggle_primary_track_data(!current_status);
	ir_video->update_display_frame();

}

void QtGuiApplication1::toggle_sensor_track_data()
{


	bool current_status = menu_sensor_boresight->isIconVisibleInMenu();

	if (current_status) {
		menu_sensor_boresight->setIconVisibleInMenu(false);
	}
	else {
		menu_sensor_boresight->setIconVisibleInMenu(true);
	}

	ir_video->toggle_sensor_boresight_data(!current_status);
	ir_video->update_display_frame();

}

deinterlace_type QtGuiApplication1::find_deinterlace_type(int index) {

	switch (index)
	{
	case 0:
		DEBUG << "GUI: De-interlace type max absolute value";
		return deinterlace_type::max_absolute_value;
	case 1:
		DEBUG << "GUI: De-interlace type centroid";
		return deinterlace_type::centroid;
	case 2:
		DEBUG << "GUI: De-interlace type avg cross correlation";
		return deinterlace_type::avg_cross_correlation;
		
	default:
		break;
	}

	INFO << "GUI: De-interlace type not found. Default to max absolute value";
	return deinterlace_type::max_absolute_value;
}

Video_Parameters QtGuiApplication1::find_deinterlace_video_type(int index)
{
	switch (index)
	{
	case 0:
		return Video_Parameters::deinterlace_max_absolute_value;
	case 1:
		return Video_Parameters::deinterlace_centroid;
	case 2:
		return Video_Parameters::deinterlace_avg_cross_correlation;

	default:
		break;
	}

	return Video_Parameters::deinterlace_max_absolute_value;
}

void QtGuiApplication1::clear_image_processing()
{
	
	int n = ir_video->container.something.size();

	txt_nuc_start->setText("");
	txt_nuc_stop->setText("");
	txt_bgs_num_frames->setText("");

	int num_videos = ir_video->container.something.size();
	if (num_videos > 0)
	{
		ir_video->container.something.erase(ir_video->container.something.begin() + 1, ir_video->container.something.begin() + 1 + (n - 1));

		chk_apply_nuc->setChecked(false);
		chk_bgs->setChecked(false);
		chk_deinterlace->setChecked(false);

		show_available_filter_options();
	}

}

video_details QtGuiApplication1::get_current_filter_state()
{
	video_details current_status;
	bool no_filters_applied = true;

	if (chk_apply_nuc->isChecked()) {
		current_status.properties[Video_Parameters::non_uniformity_correction] = true;
		no_filters_applied = false;
	}
	if (chk_bgs->isChecked()) {
		current_status.properties[Video_Parameters::background_subtraction] = true;
		no_filters_applied = false;
	}
	if (chk_deinterlace->isChecked()) {
		Video_Parameters deinterlace_video_type = find_deinterlace_video_type(cmb_deinterlace_options->currentIndex());
		current_status.properties[deinterlace_video_type] = true;

		no_filters_applied = false;
	}

	if (no_filters_applied)
		current_status.properties[Video_Parameters::original] = true;

	return current_status;
}

bool QtGuiApplication1::check_filter_selection(video_details filter_state)
{
		
	int index_exists = ir_video->container.find_data_index(filter_state);
	
	if (index_exists >= 0)
		return true;
	
	return false;
}

void QtGuiApplication1::show_available_filter_options()
{
	video_details current_state = get_current_filter_state();
	DEBUG << "GUI: Finding all available filters that can now be applied";

	//Check NUC
	if (!current_state.properties[Video_Parameters::non_uniformity_correction])
	{
		current_state.properties[Video_Parameters::original] = false;
		current_state.properties[Video_Parameters::non_uniformity_correction] = true;
		if (check_filter_selection(current_state))
			chk_apply_nuc->setEnabled(true);
		else {
			chk_apply_nuc->setChecked(false);
			chk_apply_nuc->setEnabled(false);
		}

		//reset filter state
		current_state.properties[Video_Parameters::non_uniformity_correction] = false;
	}

	//Check BGS
	if (!current_state.properties[Video_Parameters::background_subtraction])
	{
		current_state.properties[Video_Parameters::original] = false;
		current_state.properties[Video_Parameters::background_subtraction] = true;
		if (check_filter_selection(current_state))
			chk_bgs->setEnabled(true);
		else {
			chk_bgs->setChecked(false);
			chk_bgs->setEnabled(false);
		}

		//reset filter state
		current_state.properties[Video_Parameters::background_subtraction] = false;
	}

	//Check De-interlace
	Video_Parameters deinterlace_video_type = find_deinterlace_video_type(cmb_deinterlace_options->currentIndex());
	if (!current_state.properties[deinterlace_video_type])
	{
		current_state.properties[Video_Parameters::original] = false;
		current_state.properties[deinterlace_video_type] = true;
		if (check_filter_selection(current_state))
			chk_deinterlace->setEnabled(true);
		else {
			chk_deinterlace->setChecked(false);
			chk_deinterlace->setEnabled(false);
		}

		//reset filter state
		current_state.properties[deinterlace_video_type] = false;
	}

}

void QtGuiApplication1::create_background_subtraction_correction() {

	INFO << "GUI: Background subtraction video being created";
	
	//-----------------------------------------------------------------------------------------------
	// Get frame numbers from text boxes
	int num_frames_subtract = get_integer_from_txt_box(txt_bgs_num_frames->text());

	// if non-numeric data is entered...
	if (num_frames_subtract < 0)
	{
		DEBUG << "GUI: User entered non-numeric data in number of frames for background subtraction. Entered: " << txt_bgs_num_frames->text().toLocal8Bit().constData();
			
		INFO << "GUI: Background subtraction not completed";

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Non-Numeric Data"));
		msgBox.setText("Non-numeric data entered for background subtraction");

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
		return;
	}

	//find frame start / stop 
	int frame_start = 1;
	int frame_stop = eng_data->frame_numbers.back();

	bool num_within_range = check_value_within_range(num_frames_subtract, frame_start, frame_stop);

	// if values outside range...
	if (!num_within_range)
	{
		DEBUG << "GUI: Number of frames provided is not within the range 0 - " << frame_stop << ". Value entered: " << num_frames_subtract;
		INFO << "GUI: Background subtraction not completed";

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Outside of Data Range"));
		QString box_text = "Data must be within valid range (" + QString::number(frame_start) + "-" + QString::number(frame_stop) + ")";
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
		return;
	}

	//-----------------------------------------------------------------------------------------------------

	video_details background_subraction_video;
	video_details current_state = get_current_filter_state();
	int index_current_state = ir_video->container.find_data_index(current_state);

	video_details original = ir_video->container.something[index_current_state];

	DEBUG << "GUI: Input value for background subtraction validated";
	INFO << "GUI: Creating adjustment for video";
	BackgroundSubtraction background(num_frames_subtract);
	std::vector<std::vector<double>> background_correction = background.get_correction(original);

	INFO << "GUI: Background subtraction adjustment for video is completed";

	QProgressDialog progress("", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.setLabelText(QString("Copying data for processing..."));
	progress.setWindowTitle(QString("Background Subtraction"));

	progress.setMinimumWidth(300);

	background_subraction_video = original;
	background_subraction_video.clear_16bit_vector();
	background_subraction_video.clear_8bit_vector();
	background_subraction_video.histogram_data.clear();

	background_subraction_video.properties[Video_Parameters::original] = false;
	background_subraction_video.properties[Video_Parameters::background_subtraction] = true;

	// Apply background subtraction to the frames		
	int number_frames = original.frames_16bit.size();

	progress.setMaximum(number_frames - 1);
	progress.setLabelText(QString("Subtracting frames..."));

	for (int i = 0; i < number_frames; i++) {
		DEBUG << "GUI: Applying background subtraction to " << i + 1 << " of " << number_frames << "frames";
		progress.setValue(i);
		background_subraction_video.frames_16bit.push_back(background.apply_correction(original.frames_16bit[i], background_correction[i]));
		if (progress.wasCanceled())
			break;
	}

	if (progress.wasCanceled()) {
		INFO << "GUI: Background subtraction process was canceled";
		return;
	}

	progress.setLabelText(QString("Down-converting video and creating histogram data..."));

	//background_subraction_video.convert_16bit_to_8bit();
	//background_subraction_video.create_histogram_data();

	bool background_subtraction_exists = false;
	int index_background_subtraction = ir_video->container.find_data_index(background_subraction_video);
	if (index_background_subtraction > 0) {
		background_subtraction_exists = true;
		ir_video->container.something[index_background_subtraction] = background_subraction_video;
		INFO << "GUI: Background subtraction video was replaced";
	}
	else
	{
		ir_video->container.something.push_back(background_subraction_video);
		INFO << "GUI: Background subtraction video was added";
	}

	show_available_filter_options();
}

void QtGuiApplication1::toggle_video_filters()
{
	INFO << "GUI: User toggled new video for display";

	video_details user_requested = get_current_filter_state();
	bool request_exists = check_filter_selection(user_requested);

	if (!request_exists) {
		chk_apply_nuc->setChecked(false);
		chk_bgs->setChecked(false);
		chk_deinterlace->setChecked(false);

		INFO << "GUI: No matching video. Resetting filters";
	}

	show_available_filter_options();

	if (request_exists)
		ir_video->container.display_data(user_requested);
	else {
		video_details updated_user_request = get_current_filter_state();
		ir_video->container.display_data(updated_user_request);
	}

	emit playback_controller->update_frame(playback_controller->get_counter());
}

void QtGuiApplication1::set_color_correction_slider_labels()
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

void QtGuiApplication1::toggle_video_playback_options(bool input)
{
	btn_fast_forward->setEnabled(input);
	btn_slow_back->setEnabled(input);
	btn_video_menu->setEnabled(input);

	btn_frame_record->setEnabled(input);
	btn_frame_save->setEnabled(input);
	btn_zoom->setEnabled(input);

	slider_video->setEnabled(input);
	btn_play->setEnabled(input);
	btn_pause->setEnabled(input);
	btn_next_frame->setEnabled(input);
	btn_prev_frame->setEnabled(input);
	btn_reverse->setEnabled(input);
	btn_video_menu->setEnabled(input);

	if (input)
	{
		//playback_controller->start_timer();
	}
	else
	{
		playback_controller->stop_timer();
		slider_video->setValue(1);
		lbl_video_time_midnight->setText("");
		lbl_video_frame->setText("");
		lbl_fps->setText("");
	}
}

void QtGuiApplication1::enable_engineering_plot_options(bool input)
{
	// ------------------------------------------ Set Dropdown Menu ------------------------------------------
	tab_plots->setCurrentIndex(1);

	cmb_plot_xaxis->clear();
	cmb_plot_yaxis->clear();
	rad_linear->setChecked(true);
	rad_linear->setChecked(true);

	cmb_plot_yaxis->setEnabled(input);
	cmb_plot_yaxis->addItem(QString("Irradiance"));
	cmb_plot_yaxis->addItem(QString("Azimuth"));
	cmb_plot_yaxis->addItem(QString("Elevation"));
	cmb_plot_yaxis->setCurrentIndex(0);

	cmb_plot_xaxis->setEnabled(input);
	cmb_plot_xaxis->addItem(QString("Frames"));
	cmb_plot_xaxis->addItem(QString("Seconds from Midnight"));
	cmb_plot_xaxis->addItem(QString("Seconds from Epoch"));
	cmb_plot_xaxis->setCurrentIndex(0);

	// ------------------------------------------ Set Plot Options ------------------------------------------

	rad_decimal->setEnabled(input);
	rad_scientific->setEnabled(input);

	rad_log->setEnabled(input);
	rad_linear->setEnabled(input);

	btn_plot_menu->setEnabled(input);
	btn_save_plot->setEnabled(input);

}

bool QtGuiApplication1::check_min_max_frame_input(int min_frame, int max_frame)
{
	
	// if non-numeric data is entered...
	if ((min_frame < 0) || (max_frame < 0))
	{
		if (min_frame < 0)
			DEBUG << "GUI: User entered non-numeric data to start frame. Entered: " << txt_nuc_start->text().toLocal8Bit().constData();
		if (min_frame < 0)
			DEBUG << "GUI: User entered non-numeric data to stop frame. Entered: " << txt_nuc_stop->text().toLocal8Bit().constData();
		
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Non-Numeric Data"));
		msgBox.setText("Non-numeric data entered for the start/end frames");

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
		return false;
	}

	// if minimum frame is greater than maximum frame...
	if (min_frame > max_frame)
	{
		DEBUG << "GUI: User entered minimum frame (" << min_frame << ") is greater than maximum frame (" << max_frame << ")";
		
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Bad Data Entered"));
		msgBox.setText("Start frame is greater than the end frame");

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();

		return false;
	}

	//find frame start / stop 
	int frame_start = 1;
	int frame_stop = eng_data->frame_numbers.back();

	bool min_within_range = check_value_within_range(min_frame, frame_start, frame_stop);
	bool max_within_range = check_value_within_range(max_frame, frame_start, frame_stop);

	// if values outside range...
	if (!min_within_range || !max_within_range)
	{
		if (min_frame < frame_start)
			DEBUG << "GUI: Start frame before minimum frame. Entered: " << min_frame << " Minimum: " << frame_start;
		if (min_frame < 0)
			DEBUG << "GUI: Stop frame after maximum frame. Entered: " << max_frame << "Maximum: " << frame_stop;

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Outside of Data Range"));
		QString box_text = "Data must be within valid range (" + QString::number(frame_start) + "-" + QString::number(frame_stop) + ")";
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
		return false;
	}
	
	
	return true;
}

void QtGuiApplication1::update_epoch_string(QString new_epoch_string)
{

	QString out = "Applied Epoch: ";
	out = out +new_epoch_string;
	lbl_current_epoch->setText(out);
	
}

void QtGuiApplication1::display_original_epoch(QString new_epoch_string) 
{
	QString out = "Original Epoch: ";
	out = out + new_epoch_string;
	lbl_current_epoch->setToolTip(out);
}

QString QtGuiApplication1::create_epoch_string(std::vector<double> new_epoch) {

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