#include "QtGuiApplication1.h"
#include "location_input.h"
//#include "new_location.h"

QtGuiApplication1::QtGuiApplication1(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.tabMenu->setTabEnabled(1, false);
	ui.tabMenu->setTabEnabled(2, false);
	ui.tabMenu->setTabEnabled(3, false);
	   
	INFO << "GUI: Initializing GUI";

	//---------------------------------------------------------------------------

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

	//---------------------------------------------------------------------------

	videos = new Video_Container(); 
	
	// Initialize epoch time box
	//ui.dt_epoch->setDate(QDate(2000, 1, 1));
	//ui.dt_epoch->setTime(QTime(0, 0, 0, 0));

	//---------------------------------------------------------------------------
	eng_data = NULL;

	
	data_plot_yformat.addButton(ui.rad_decimal);
	data_plot_yformat.addButton(ui.rad_scientific);
	ui.rad_decimal->setChecked(true);

	data_plot_yloglinear.addButton(ui.rad_log);
	data_plot_yloglinear.addButton(ui.rad_linear);
	ui.rad_linear->setChecked(true);

	connect(ui.rad_log, &QRadioButton::toggled, this, &QtGuiApplication1::yaxis_log_toggled);
	connect(ui.rad_decimal, &QRadioButton::toggled, this, &QtGuiApplication1::yaxis_decimal_toggled);
	connect(ui.rad_linear, &QRadioButton::toggled, this, &QtGuiApplication1::yaxis_linear_toggled);
	connect(ui.rad_scientific, &QRadioButton::toggled, this, &QtGuiApplication1::yaxis_scientific_toggled);

	//---------------------------------------------------------------------------

	playback_controller = new Playback(1);
	playback_controller->moveToThread(&thread_timer);
	QObject::connect(&thread_video, &QThread::started, playback_controller, &Playback::start_timer);
	
	//---------------------------------------------------------------------------	
	
	ui.btn_copy_directory->setEnabled(true);
	clipboard = QApplication::clipboard();
	QObject::connect(ui.btn_copy_directory, &QPushButton::clicked, this, &QtGuiApplication1::copy_osm_directory);
	//---------------------------------------------------------------------------	

	ir_video = new Video(1, 1, max_used_bits);

	ir_video->moveToThread(&thread_video);

	QObject::connect(&thread_video, &QThread::started, ir_video, &Video::update_display_frame);
	
	QObject::connect(videos, &Video_Container::update_display_video, ir_video, &Video::receive_video_data);
	QObject::connect(playback_controller, &Playback::update_frame, ir_video, &Video::update_specific_frame);
	QObject::connect(&color_correction, &Min_Max_Value::update_min_max, ir_video, &Video::update_color_correction);

	record_video = false;

	//---------------------------------------------------------------------------	
	
	int number_bits = max_used_bits;
	
	video_layout = new QGridLayout();
	video_layout->addWidget(ir_video->label);
	ui.frm_video->setLayout(video_layout);

	histogram_layout = new QGridLayout();
	histogram_layout->addWidget(ir_video->histogram_plot->rel_chart_view);
	ui.frm_histogram->setLayout(histogram_layout);
	
	histogram_abs_layout = new QGridLayout();
	histogram_abs_layout->addWidget(ir_video->histogram_plot->chart_view);
	ui.frm_histogram_abs->setLayout(histogram_abs_layout);

	QObject::connect(ui.tabMenu, &QTabWidget::currentChanged, this, &QtGuiApplication1::auto_change_plot_display);
	QObject::connect(ui.chk_relative_histogram, &QCheckBox::toggled, this, &QtGuiApplication1::toggle_relative_histogram);
	toggle_relative_histogram(false);
	//---------------------------------------------------------------------------

	// Link color correction sliders to changing color correction values
	QObject::connect(ui.slider_gain, &QSlider::valueChanged, this, &QtGuiApplication1::gain_slider_toggled);
	QObject::connect(ui.slider_lift, &QSlider::valueChanged, this, &QtGuiApplication1::lift_slider_toggled);

	ui.lbl_lift->setToolTip("Dark Set Point pushes the image darker");
	ui.lbl_gain->setToolTip("Light Set Point pushes the image lighter");

	int max_lift, min_lift, max_gain, min_gain;
	color_correction.get_min_slider_range(min_lift, max_lift);
	color_correction.get_max_slider_range(min_gain, max_gain);
	
	ui.slider_lift->setMinimum(min_lift);
	ui.slider_lift->setMaximum(max_lift);
	ui.slider_gain->setMinimum(min_gain);
	ui.slider_gain->setMaximum(max_gain);

	reset_color_correction();
	QObject::connect(ui.btn_reset_color_correction, &QPushButton::clicked, this, &QtGuiApplication1::reset_color_correction);

	//---------------------------------------------------------------------------

	ui.cmb_deinterlace_options->addItem("Max Absolute Value");
	ui.cmb_deinterlace_options->addItem("Centroid");
	ui.cmb_deinterlace_options->addItem("Avg Cross Correlation");

	QObject::connect(ui.cmb_deinterlace_options, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtGuiApplication1::toggle_video_filters);

	//---------------------------------------------------------------------------

	// Link horizontal slider to playback controller
	QObject::connect(playback_controller, &Playback::update_frame, ui.sldrVideo, &QSlider::setValue);
	QObject::connect(ui.sldrVideo, &QSlider::valueChanged, playback_controller, &Playback::set_counter);

	//---------------------------------------------------------------------------

	// Link playback to play controls
	QObject::connect(ui.btn_play, &QPushButton::clicked, playback_controller, &Playback::start_timer);
	QObject::connect(ui.btn_pause, &QPushButton::clicked, playback_controller, &Playback::stop_timer);
	QObject::connect(ui.btn_reverse, &QPushButton::clicked, playback_controller, &Playback::reverse);


	QObject::connect(ui.btn_fast_forward, &QPushButton::clicked, playback_controller, &Playback::speed_timer);
	QObject::connect(ui.btn_slow_back, &QPushButton::clicked, playback_controller, &Playback::slow_timer);
	QObject::connect(ui.btn_next_frame, &QPushButton::clicked, playback_controller, &Playback::next_frame);
	QObject::connect(ui.btn_prev_frame, &QPushButton::clicked, playback_controller, &Playback::prev_frame);
	QObject::connect(ui.btn_frame_record, &QPushButton::clicked, this, &QtGuiApplication1::start_stop_video_record);
		
	QObject::connect(ui.btn_fast_forward, &QPushButton::clicked, this, &QtGuiApplication1::update_fps);
	QObject::connect(ui.btn_slow_back, &QPushButton::clicked, this, &QtGuiApplication1::update_fps);

	//---------------------------------------------------------------------------

	//Link buttons to functions
	QObject::connect(ui.btn_load_osm, &QPushButton::clicked, this, &QtGuiApplication1::load_osm_data);
	QObject::connect(ui.btn_get_frames, &QPushButton::clicked, this, &QtGuiApplication1::load_abir_data);
	QObject::connect(ui.txt_end_frame, &QLineEdit::returnPressed, this, &QtGuiApplication1::load_abir_data);

	QObject::connect(ui.btn_create_nuc, &QPushButton::clicked, this, &QtGuiApplication1::create_non_uniformity_correction);
	QObject::connect(ui.txt_nuc_stop, &QLineEdit::returnPressed, this, &QtGuiApplication1::create_non_uniformity_correction);
	
	QObject::connect(ui.btn_bgs, &QPushButton::clicked, this, &QtGuiApplication1::create_background_subtraction_correction);
	QObject::connect(ui.txt_bgs_num_frames, &QLineEdit::returnPressed, this, &QtGuiApplication1::create_background_subtraction_correction);

	QObject::connect(ui.btn_deinterlace, &QPushButton::clicked, this, &QtGuiApplication1::create_deinterlace);

	QObject::connect(ui.chk_apply_nuc, &QCheckBox::stateChanged, this, &QtGuiApplication1::toggle_video_filters);
	QObject::connect(ui.chk_bgs, &QCheckBox::stateChanged, this, &QtGuiApplication1::toggle_video_filters);
	QObject::connect(ui.chk_deinterlace, &QCheckBox::stateChanged, this, &QtGuiApplication1::toggle_video_filters);

	//---------------------------------------------------------------------------

	// Show Import Menu on load
	ui.tabMenu->setCurrentIndex(0);

	//---------------------------------------------------------------------------

	// Connect epoch button click to function
	QObject::connect(ui.btn_apply_epoch, &QPushButton::clicked, this, &QtGuiApplication1::apply_epoch_time);
	
	//---------------------------------------------------------------------------

	//Enable clearing image processing filters
	QObject::connect(ui.btn_clear_filters, &QPushButton::clicked, this, &QtGuiApplication1::clear_image_processing);

	//Enable saving frame
	QObject::connect(ui.btn_frame_save, &QPushButton::clicked, this, &QtGuiApplication1::save_frame);

	//Update frame number label
	QObject::connect(playback_controller, &Playback::update_frame, this, &QtGuiApplication1::set_frame_number_label);

	//---------------------------------------------------------------------------


	//Add icons to video playback buttons
	QPixmap play_image("icons/play.png");
	QIcon play_icon(play_image);
	ui.btn_play->setIcon(play_icon);
	ui.btn_play->setText("");
	ui.btn_play->setToolTip("Play Video");
	ui.btn_play->setEnabled(false);

	QPixmap pause_image("icons/pause.png");
	QIcon pause_icon(pause_image);
	ui.btn_pause->setIcon(pause_icon);
	ui.btn_pause->setText("");
	ui.btn_pause->setToolTip("Pause Video");
	ui.btn_pause->setEnabled(false);

	QPixmap reverse_image("icons/reverse.png");
	QIcon reverse_icon(reverse_image);
	ui.btn_reverse->setIcon(reverse_icon);
	ui.btn_reverse->setText("");
	ui.btn_reverse->setToolTip("Reverse Video");
	ui.btn_reverse->setEnabled(false);

	QPixmap speed_up_image("icons/chevron-double-up.png");
	QIcon speed_up_icon(speed_up_image);
	ui.btn_fast_forward->setIcon(speed_up_icon);
	ui.btn_fast_forward->setText("");
	ui.btn_fast_forward->setToolTip("Speed Up");
	ui.btn_fast_forward->setEnabled(false);

	QPixmap next_frame_image("icons/skip-next.png");
	QIcon next_frame_icon(next_frame_image);
	ui.btn_next_frame->setIcon(next_frame_icon);
	ui.btn_next_frame->setText("");
	ui.btn_next_frame->setToolTip("Next Frame");
	ui.btn_next_frame->setEnabled(false);

	QPixmap slow_down_image("icons/chevron-double-down.png");
	QIcon slow_down_icon(slow_down_image);
	ui.btn_slow_back->setIcon(slow_down_icon);
	ui.btn_slow_back->setText("");
	ui.btn_slow_back->setToolTip("Slow Down");
	ui.btn_slow_back->setEnabled(false);

	QPixmap prev_frame_image("icons/skip-previous.png");
	QIcon prev_frame_icon(prev_frame_image);
	ui.btn_prev_frame->setIcon(prev_frame_icon);
	ui.btn_prev_frame->setText("");
	ui.btn_prev_frame->setToolTip("Previous Frame");
	ui.btn_prev_frame->setEnabled(false);

	QPixmap record_frame("icons/record.png");
	QIcon record_frame_icon(record_frame);
	ui.btn_frame_record->setIcon(record_frame_icon);
	ui.btn_frame_record->setText("");
	ui.btn_frame_record->setToolTip("Record Video");

	QPixmap save_frame("icons/content-save.png");
	QIcon save_frame_icon(save_frame);
	ui.btn_frame_save->setIcon(save_frame_icon);
	ui.btn_frame_save->setText("");
	ui.btn_frame_save->setToolTip("Save Frame");

	ui.btn_save_plot->setIcon(save_frame_icon);
	ui.btn_save_plot->setText("");
	ui.btn_save_plot->setToolTip("Save Plot");

	create_menu_actions();

	menu = new QMenu(this);
	menu->addAction(menu_osm);
	menu->addAction(menu_add_primary_data);
	menu->addAction(menu_sensor_boresight);
	menu->addAction(menu_add_banner);
	menu->addAction(menu_change_color_banner);
	menu->addAction(menu_change_color_tracker);

	plot_menu = new QMenu(this);
	plot_menu->addAction(menu_plot_all_data);
	plot_menu->addAction(menu_plot_primary);
	plot_menu->addAction(menu_plot_frame_marker);
	plot_menu->addAction(menu_plot_edit_banner);

	QPixmap menu_image("icons/menu.png");
	QIcon menu_icon(menu_image);
	ui.btn_video_menu->setIcon(menu_icon);
	ui.btn_video_menu->setText("");
	ui.btn_video_menu->setEnabled(false);

	ui.btn_plot_menu->setIcon(menu_icon);
	ui.btn_plot_menu->setText("");

	ui.btn_video_menu->setMenu(menu);
	ui.btn_plot_menu->setMenu(plot_menu);

	show();

	INFO << "GUI: GUI Initialized";
}

QtGuiApplication1::~QtGuiApplication1() {

	delete ir_video;
	delete playback_controller;
	//delete histogram_plot;

	videos->something.clear();
	delete videos;

	//delete ir_video;
	//delete histogram_plot;
	//delete data_plots;
	
	delete eng_data;

	thread_video.terminate();
	thread_timer.terminate();

	DEBUG << "GUI: GUI destructor called";
}

void QtGuiApplication1::load_osm_data()
{

	INFO << "GUI: Start OSM load process";

	ui.txt_start_frame->setEnabled(false);
	ui.txt_end_frame->setEnabled(false);
	ui.btn_get_frames->setEnabled(false);
	ui.btn_load_osm->setEnabled(false);
	ui.btn_copy_directory->setEnabled(false);

	INFO << "GUI: Loading OSM data";
	bool valid_files = file_data.load_osm_file();
	
	// if no file was selected, do nothing
	if (file_data.file_name.compare(""))
	{
		return;
	}
	
	// if file selected was invalid, then tell user
	if (!valid_files) {
		WARN << "GUI: OSM file failed cursory check";

		ui.btn_load_osm->setEnabled(true);

		if (eng_data != NULL) {
			// if eng_data already initialized, allow user to re-select frames
			ui.txt_start_frame->setEnabled(true);
			ui.txt_end_frame->setEnabled(true);
			ui.btn_get_frames->setEnabled(true);
			ui.btn_load_osm->setEnabled(true);
		}

		ui.lbl_file_load->setText(file_data.info_msg);
		ui.lbl_file_name->setText(file_data.file_name);
		ui.lbl_file_name->setToolTip(file_data.directory_path);

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Error Loading OSM"));
		QString box_text = "An unexpected error occurred while loading the OSM \n\nSee file status box (lower left corner) for details \n\nMore detailed information can be found in the log";
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();

		return;
	}
		
	INFO << "GUI: OSM file has valid path";

	ui.lbl_file_load->setText(file_data.info_msg);
	ui.lbl_file_name->setText(file_data.file_name);
	ui.lbl_file_name->setToolTip(file_data.directory_path);

	if (valid_files) {
		ui.txt_start_frame->setEnabled(true);
		ui.txt_end_frame->setEnabled(true);
		ui.btn_get_frames->setEnabled(true);

		QString osm_max_frames = QString::number(file_data.osm_data.num_messages);
		ui.txt_start_frame->setText(QString("1"));
		ui.txt_end_frame->setText(osm_max_frames);

		QString max_frame_text("Max Frames: ");
		max_frame_text.append(osm_max_frames);
		ui.lbl_max_frames->setText(max_frame_text);

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

			videos->something.clear();
			ir_video->remove_frame();

			clear_frame_label();

			video_layout->addWidget(ir_video->label);
			ui.frm_video->setLayout(video_layout);
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
		ui.dt_epoch->setEnabled(true);
		ui.btn_apply_epoch->setEnabled(true);
				
		std::vector<double> epoch0 = eng_data->get_epoch();
		std::vector<double> epoch_min = eng_data->get_adj_epoch(-2);
		std::vector<double> epoch_max = eng_data->get_adj_epoch(2);
		update_epoch_string(create_epoch_string(epoch0));
		display_original_epoch(create_epoch_string(epoch0));

		QDate new_date(epoch0[0], epoch0[1], epoch0[2]);
		QDate min_date(epoch_min[0], epoch_min[1], epoch_min[2]);
		QDate max_date(epoch_max[0], epoch_max[1], epoch_max[2]);

		ui.dt_epoch->setDate(new_date);
		ui.dt_epoch->setMinimumDate(min_date);
		ui.dt_epoch->setMaximumDate(max_date);

		ui.dt_epoch->setTime(QTime(epoch0[3], epoch0[4], epoch0[5]));
		//--------------------------------------------------------------------------------

		engineering_plot_layout = new QGridLayout();
		engineering_plot_layout->addWidget(data_plots->chart_view);
		ui.frm_plots->setLayout(engineering_plot_layout);

		//--------------------------------------------------------------------------------
		
		// Reset settings on video playback to defaults
		menu_osm->setIconVisibleInMenu(false);
		ir_video->toggle_osm_tracks(false);

		menu_add_primary_data->setIconVisibleInMenu(false);
		ir_video->toggle_primary_track_data(false);

		menu_sensor_boresight->setIconVisibleInMenu(false);
		ir_video->toggle_sensor_boresight_data(false);

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

	ui.btn_load_osm->setEnabled(true);
	ui.btn_copy_directory->setEnabled(true);

	return;
}

void QtGuiApplication1::load_abir_data()
{

	ui.btn_get_frames->setEnabled(false);

	INFO << "GUI: Starting ABIR load process";

	// -----------------------------------------------------------------------------------------

	// Get frame numbers from text boxes
	int min_frame = get_integer_from_txt_box(ui.txt_start_frame->text());
	int max_frame = get_integer_from_txt_box(ui.txt_end_frame->text());

	bool check_data = check_min_max_frame_input(min_frame, max_frame);

	if (!check_data) {
		ui.btn_get_frames->setEnabled(true);

		INFO << "GUI: No video loaded";
		return;
	}
	
	//-----------------------------------------------------------------------------------

	DEBUG << "GUI: Frame numbers are valid";

	//---------------------------------------------------------------------------

	// Load the ABIR data
	
	videos->something.clear();
	
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
		ui.btn_get_frames->setEnabled(true);
		return;
	}
		
	int x_pixels = file_data.abir_data.ir_data[0].header.image_x_size;
	int y_pixels = file_data.abir_data.ir_data[0].header.image_y_size;

	DEBUG << "GUI: Frames are of size " << x_pixels << " x " << y_pixels;
		
	primary.set_image_size(x_pixels, y_pixels);
	primary.set_video_frames(video_frames);
	videos->something.push_back(primary);
		
	int number_frames = primary.frames_16bit.size();
	QString status_txt = ui.lbl_file_load->text();
	QString update_text("\nFrames ");
	update_text.append(QString::number(min_frame));
	update_text.append(" to ");
	update_text.append(QString::number(min_frame + number_frames - 1));
	update_text.append(" were loaded");

	status_txt.append(update_text);
	ui.lbl_file_load->setText(status_txt);

	//---------------------------------------------------------------------------
	// Set frame number for playback controller and valid values for slider
	playback_controller->set_number_of_frames(number_frames);
	ui.sldrVideo->setRange(0, number_frames);	

	//---------------------------------------------------------------------------
	// Set the video and histogram plots to this data
	videos->display_original_data();	
	
	//playback_controller->start_timer();
	// Start threads...
	thread_video.start();
	thread_timer.start();

	std::vector<Plotting_Frame_Data>::const_iterator first = eng_data->frame_data.begin() + min_frame - 1;
	std::vector<Plotting_Frame_Data>::const_iterator last = eng_data->frame_data.begin() + (min_frame) + (max_frame - min_frame);
	std::vector<Plotting_Frame_Data> subset_data(first, last);
	ir_video->set_frame_data(subset_data);

	// Reset engineering plots with new sub plot indices
	data_plots->index_sub_plot_xmin = min_frame - 1;
	data_plots->index_sub_plot_xmax = max_frame - 1;
	plot_change(1);

	//Update frame marker on engineering plot
	QObject::connect(playback_controller, &Playback::update_frame, data_plots, &Engineering_Plots::plot_current_step);
	
	playback_controller->set_speed_index(10);
	update_fps();

	ui.tabPlots->setCurrentIndex(1);
	plot_full_data();
	plot_current_frame_marker();

	ui.btn_get_frames->setEnabled(true);

	ui.tabMenu->setTabEnabled(1, true);
	ui.tabMenu->setTabEnabled(2, true);

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
		ui.btn_frame_record->setIcon(record_icon);
		ui.btn_frame_record->setText("");
		ui.btn_frame_record->setToolTip("Start Record");
		ui.btn_frame_record->setEnabled(true);

		record_video = false;
	}
	else {
		//Starting record video
		bool file_opened = ir_video->start_recording(playback_controller->get_fps());

		if (file_opened) {

			QPixmap stop_image("icons/stop.png");
			QIcon stop_icon(stop_image);
			ui.btn_frame_record->setIcon(stop_icon);
			ui.btn_frame_record->setText("");
			ui.btn_frame_record->setToolTip("Stop Record");
			ui.btn_frame_record->setEnabled(true);

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

void QtGuiApplication1::update_fps()
{
	QString fps = QString::number(playback_controller->speeds[playback_controller->index_speed], 'g', 2);
	fps.append(" fps");

	ui.lbl_fps->setText(fps);
}

void QtGuiApplication1::lift_slider_toggled(int value) {

	double lift_value = color_correction.min_convert_slider_to_value(ui.slider_lift->value());
	double gain_value = color_correction.max_convert_slider_to_value(ui.slider_gain->value());

	//Prevent lift from being higher than gain value
	if (lift_value >= gain_value) {

		int new_value = color_correction.get_ui_slider_value(gain_value);
		ui.slider_lift->setValue(new_value - 1);
		lift_value = color_correction.min_convert_slider_to_value(ui.slider_lift->value());;
	}

	color_correction_toggled(lift_value, gain_value);
}

void QtGuiApplication1::gain_slider_toggled(int value) {

	double lift_value = color_correction.min_convert_slider_to_value(ui.slider_lift->value());
	double gain_value = color_correction.max_convert_slider_to_value(ui.slider_gain->value());

	// Prevent gain going below lift value
	if (gain_value <= lift_value) {

		int new_value = color_correction.get_ui_slider_value(lift_value);
		ui.slider_gain->setValue(new_value + 1);
		gain_value = color_correction.max_convert_slider_to_value(ui.slider_gain->value());;
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
			
	ui.slider_lift->setValue(0);
	ui.slider_gain->setValue(1000);
	ui.chk_relative_histogram->setChecked(false);

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
		ui.tabPlots->setCurrentIndex(0);
	}

	// When processing tab is selected, the engineering plots are automically displayed
	if (index == 2) {
		ui.tabPlots->setCurrentIndex(1);
	}
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

	// Connect x-axis and y-axis changes to functions
	connect(ui.cmb_plot_yaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtGuiApplication1::plot_change);
	connect(ui.cmb_plot_xaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtGuiApplication1::plot_change);

	// Connect save button functions
	connect(ui.btn_save_plot, &QPushButton::clicked, this, &QtGuiApplication1::save_plot);

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

int QtGuiApplication1::get_color_index(QList<QString> colors, QColor input_color) {

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


void QtGuiApplication1::edit_banner_color()
{
	
	QList<QString> colors{};
	colors.append("red");
	colors.append("orange");
	colors.append("yellow");
	colors.append("green");
	colors.append("blue");
	colors.append("violet");

	QStringList color_list(colors);
	int index = get_color_index(colors, ir_video->banner_color);

	bool ok;
	QString input_text = QInputDialog::getItem(0, "Banner Color", "Banner Color", color_list, index, false, &ok);

	if (ok) {
		emit change_banner_color(input_text);
		DEBUG << "GUI: Banner color changed";
		ir_video->update_display_frame();
	}
	else {
		DEBUG << "GUI: Banner color change cancelled";
	}
}

void QtGuiApplication1::edit_tracker_color()
{

	QList<QString> colors{};
	colors.append("red");
	colors.append("orange");
	colors.append("yellow");
	colors.append("green");
	colors.append("blue");
	colors.append("violet");

	QStringList color_list(colors);
	int index = get_color_index(colors, ir_video->tracker_color);

	bool ok;
	QString input_text = QInputDialog::getItem(0, "Tracker Color", "Tracker Color", color_list, index, false, &ok);


	if (ok) {
		emit change_tracker_color(input_text);
		DEBUG << "GUI: Banner color changed";
		ir_video->update_display_frame();
	}
	else {
		DEBUG << "GUI: Banner color change cancelled";
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

	int x_index = ui.cmb_plot_xaxis->currentIndex();
	int y_index = ui.cmb_plot_yaxis->currentIndex();

	// Check that indices are all positive
	if (x_index >= 0 && y_index >= 0)
	{

		bool scientific_is_checked = ui.rad_scientific->isChecked();
		bool log_is_checked = ui.rad_log->isChecked();
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

	int index = data_plots->index_sub_plot_xmin;
		
	int frame_number = eng_data->frame_numbers[index + counter];
	QString frame_text("Frame # ");
	frame_text.append(QString::number(frame_number));
	ui.lbl_video_frame->setText(frame_text);

	double seconds_midnight = eng_data->seconds_from_midnight[index + counter];
	QString seconds_text("From Midnight ");
	seconds_text.append(QString::number(seconds_midnight, 'g', 8));
	ui.lbl_video_time_midnight->setText(seconds_text);

}

void QtGuiApplication1::clear_frame_label()
{

	QString frame_text("");
	ui.lbl_video_frame->setText(frame_text);

	QString seconds_text("");
	ui.lbl_video_time_midnight->setText(seconds_text);

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

		ui.frm_histogram_abs->move(10, 300);
		ui.frm_histogram_abs->resize(651, 281);

		ir_video->update_display_frame();

	}
	else
	{
		ir_video->show_relative_histogram = false;

		ui.frm_histogram_abs->move(10, 10);
		ui.frm_histogram_abs->resize(651, 571);
	}
}

void QtGuiApplication1::apply_epoch_time()
{
	double year, month, day, hr, min;
	double sec, msec, epoch_jdate;

	// --------------------------------------------------------------------
	// Get new date

	QDate date = ui.dt_epoch->date();
	QTime time = ui.dt_epoch->time();

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
	int min_frame = get_integer_from_txt_box(ui.txt_nuc_start->text());
	int max_frame = get_integer_from_txt_box(ui.txt_nuc_stop->text());

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
	int index_current_state = videos->find_data_index(current_state);

	video_details original = videos->something[index_current_state];

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
	int index_nuc = videos->find_data_index(nuc_video);
	if (index_nuc > 0) {
		nuc_exists = true;
		videos->something[index_nuc] = nuc_video;

		INFO << "GUI: Previous NUC video was replaced";
	}
	else
	{
		videos->something.push_back(nuc_video);
		INFO << "GUI: NUC video added";
	}
	

	show_available_filter_options();
}

void QtGuiApplication1::create_deinterlace()
{
	INFO << "GUI: Creating de-interlace file from original data";
		
	deinterlace_type deinterlace_method_type = find_deinterlace_type(ui.cmb_deinterlace_options->currentIndex());
	Video_Parameters deinterlace_video_type = find_deinterlace_video_type(ui.cmb_deinterlace_options->currentIndex());
	
	video_details deinterlace_video;
	video_details current_state = get_current_filter_state();
	int index_current_state = videos->find_data_index(current_state);

	video_details original = videos->something[index_current_state];

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
	int index_deinterlace = videos->find_data_index(deinterlace_video);
	if (index_deinterlace > 0) {
		deinterlace_exists = true;
		videos->something[index_deinterlace] = deinterlace_video;
		INFO << "GUI: Previous de-interlace video was replaced";
	}
	else
	{
		videos->something.push_back(deinterlace_video);
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
	
	int n = videos->something.size();

	ui.txt_nuc_start->setText("");
	ui.txt_nuc_stop->setText("");
	ui.txt_bgs_num_frames->setText("");

	int num_videos = videos->something.size();
	if (num_videos > 0)
	{
		videos->something.erase(videos->something.begin() + 1, videos->something.begin() + 1 + (n - 1));

		ui.chk_apply_nuc->setChecked(false);
		ui.chk_bgs->setChecked(false);
		ui.chk_deinterlace->setChecked(false);

		show_available_filter_options();
	}

}

video_details QtGuiApplication1::get_current_filter_state()
{
	video_details current_status;
	bool no_filters_applied = true;

	if (ui.chk_apply_nuc->isChecked()) {
		current_status.properties[Video_Parameters::non_uniformity_correction] = true;
		no_filters_applied = false;
	}
	if (ui.chk_bgs->isChecked()) {
		current_status.properties[Video_Parameters::background_subtraction] = true;
		no_filters_applied = false;
	}
	if (ui.chk_deinterlace->isChecked()) {
		Video_Parameters deinterlace_video_type = find_deinterlace_video_type(ui.cmb_deinterlace_options->currentIndex());
		current_status.properties[deinterlace_video_type] = true;

		no_filters_applied = false;
	}

	if (no_filters_applied)
		current_status.properties[Video_Parameters::original] = true;

	return current_status;
}

bool QtGuiApplication1::check_filter_selection(video_details filter_state)
{
		
	int index_exists = videos->find_data_index(filter_state);
	
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
			ui.chk_apply_nuc->setEnabled(true);
		else {
			ui.chk_apply_nuc->setChecked(false);
			ui.chk_apply_nuc->setEnabled(false);
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
			ui.chk_bgs->setEnabled(true);
		else {
			ui.chk_bgs->setChecked(false);
			ui.chk_bgs->setEnabled(false);
		}

		//reset filter state
		current_state.properties[Video_Parameters::background_subtraction] = false;
	}

	//Check De-interlace
	Video_Parameters deinterlace_video_type = find_deinterlace_video_type(ui.cmb_deinterlace_options->currentIndex());
	if (!current_state.properties[deinterlace_video_type])
	{
		current_state.properties[Video_Parameters::original] = false;
		current_state.properties[deinterlace_video_type] = true;
		if (check_filter_selection(current_state))
			ui.chk_deinterlace->setEnabled(true);
		else {
			ui.chk_deinterlace->setChecked(false);
			ui.chk_deinterlace->setEnabled(false);
		}

		//reset filter state
		current_state.properties[deinterlace_video_type] = false;
	}

}

void QtGuiApplication1::create_background_subtraction_correction() {

	INFO << "GUI: Background subtraction video being created";
	
	//-----------------------------------------------------------------------------------------------
	// Get frame numbers from text boxes
	int num_frames_subtract = get_integer_from_txt_box(ui.txt_bgs_num_frames->text());

	// if non-numeric data is entered...
	if (num_frames_subtract < 0)
	{
		DEBUG << "GUI: User entered non-numeric data in number of frames for background subtraction. Entered: " << ui.txt_bgs_num_frames->text().toLocal8Bit().constData();
			
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
	int index_current_state = videos->find_data_index(current_state);

	video_details original = videos->something[index_current_state];

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
	int index_background_subtraction = videos->find_data_index(background_subraction_video);
	if (index_background_subtraction > 0) {
		background_subtraction_exists = true;
		videos->something[index_background_subtraction] = background_subraction_video;
		INFO << "GUI: Background subtraction video was replaced";
	}
	else
	{
		videos->something.push_back(background_subraction_video);
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
		ui.chk_apply_nuc->setChecked(false);
		ui.chk_bgs->setChecked(false);
		ui.chk_deinterlace->setChecked(false);

		INFO << "GUI: No matching video. Resetting filters";
	}

	show_available_filter_options();

	if (request_exists)
		videos->display_data(user_requested);
	else {
		video_details updated_user_request = get_current_filter_state();
		videos->display_data(updated_user_request);
	}

	emit playback_controller->update_frame(playback_controller->get_counter());
}

void QtGuiApplication1::set_color_correction_slider_labels()
{
	double lift_value = color_correction.min_convert_slider_to_value(ui.slider_lift->value());
	double gain_value = color_correction.max_convert_slider_to_value(ui.slider_gain->value());
		
	QString lift_string;
	lift_string.setNum(lift_value);
	ui.lbl_lift_value->setText(lift_string);

	QString gain_string;
	gain_string.setNum(gain_value);
	ui.lbl_gain_value->setText(gain_string);
}

void QtGuiApplication1::toggle_video_playback_options(bool input)
{
	ui.btn_fast_forward->setEnabled(input);
	ui.btn_slow_back->setEnabled(input);
	ui.btn_video_menu->setEnabled(input);

	ui.btn_frame_record->setEnabled(input);
	ui.btn_frame_save->setEnabled(input);

	ui.sldrVideo->setEnabled(input);
	ui.btn_play->setEnabled(input);
	ui.btn_pause->setEnabled(input);
	ui.btn_next_frame->setEnabled(input);
	ui.btn_prev_frame->setEnabled(input);
	ui.btn_reverse->setEnabled(input);
	ui.btn_video_menu->setEnabled(input);

	if (input)
	{
		//playback_controller->start_timer();
	}
	else
	{
		playback_controller->stop_timer();
		ui.sldrVideo->setValue(1);
		ui.lbl_video_time_midnight->setText("");
		ui.lbl_video_frame->setText("");
		ui.lbl_fps->setText("");
	}
}

void QtGuiApplication1::enable_engineering_plot_options(bool input)
{
	// ------------------------------------------ Set Dropdown Menu ------------------------------------------
	ui.tabPlots->setCurrentIndex(1);

	ui.cmb_plot_xaxis->clear();
	ui.cmb_plot_yaxis->clear();
	ui.rad_linear->setChecked(true);
	ui.rad_linear->setChecked(true);

	ui.cmb_plot_yaxis->setEnabled(input);
	ui.cmb_plot_yaxis->addItem(QString("Irradiance"));
	ui.cmb_plot_yaxis->addItem(QString("Azimuth"));
	ui.cmb_plot_yaxis->addItem(QString("Elevation"));
	ui.cmb_plot_yaxis->setCurrentIndex(0);

	ui.cmb_plot_xaxis->setEnabled(input);
	ui.cmb_plot_xaxis->addItem(QString("Frames"));
	ui.cmb_plot_xaxis->addItem(QString("Seconds from Midnight"));
	ui.cmb_plot_xaxis->addItem(QString("Seconds from Epoch"));
	ui.cmb_plot_xaxis->setCurrentIndex(0);

	// ------------------------------------------ Set Plot Options ------------------------------------------

	ui.rad_decimal->setEnabled(input);
	ui.rad_scientific->setEnabled(input);

	ui.rad_log->setEnabled(input);
	ui.rad_linear->setEnabled(input);

	ui.btn_plot_menu->setEnabled(input);
	ui.btn_save_plot->setEnabled(input);

	ui.chk_plot_full_data->setEnabled(input);
	ui.chk_plot_primary_data->setEnabled(input);
	ui.chk_plot_show_line->setEnabled(input);

}

bool QtGuiApplication1::check_min_max_frame_input(int min_frame, int max_frame)
{
	
	// if non-numeric data is entered...
	if ((min_frame < 0) || (max_frame < 0))
	{
		if (min_frame < 0)
			DEBUG << "GUI: User entered non-numeric data to start frame. Entered: " << ui.txt_nuc_start->text().toLocal8Bit().constData();
		if (min_frame < 0)
			DEBUG << "GUI: User entered non-numeric data to stop frame. Entered: " << ui.txt_nuc_stop->text().toLocal8Bit().constData();
		
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
	ui.lbl_current_epoch->setText(out);
	
}

void QtGuiApplication1::display_original_epoch(QString new_epoch_string) 
{
	QString out = "Original Epoch: ";
	out = out + new_epoch_string;
	ui.lbl_current_epoch->setToolTip(out);
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