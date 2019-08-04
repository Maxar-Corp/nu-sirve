#include "QtGuiApplication1.h"
#include "location_input.h"

QtGuiApplication1::QtGuiApplication1(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	QString gary;
	LocationInput bob;
	auto jane = bob.exec();

	ui.tabMenu->setTabEnabled(1, false);
	ui.tabMenu->setTabEnabled(2, false);
	ui.tabMenu->setTabEnabled(3, false);
	   
	INFO << "GUI: Initializing GUI";

	max_used_bits = 14;

	videos = new Video_Container(); 

	//---------------------------------------------------------------------------
	eng_data = NULL;

	playback_controller = new Playback(1);
	
	//---------------------------------------------------------------------------	

	ir_video = new Video(1, 1);
	
	QObject::connect(videos, &Video_Container::update_display_video, ir_video, &Video::receive_video_data);
	QObject::connect(playback_controller, &Playback::update_frame, ir_video, &Video::update_specific_frame);
	QObject::connect(&color_correction, &Lift_Gamma_Gain::update_lift_gamma_gain, ir_video, &Video::update_color_correction);
	
	//---------------------------------------------------------------------------	
	
	int number_bits = 8;
	histogram_plot = new HistogramLine_Plot(number_bits);
		
	QObject::connect(videos, &Video_Container::update_display_video, histogram_plot, &HistogramLine_Plot::receive_video_data);
	QObject::connect(playback_controller, &Playback::update_frame, histogram_plot, &HistogramLine_Plot::update_specific_histogram);
	QObject::connect(&color_correction, &Lift_Gamma_Gain::update_lift_gamma_gain, histogram_plot, &HistogramLine_Plot::update_color_correction);

	//---------------------------------------------------------------------------

	color_plot_layout = new QGridLayout();
	color_plot_layout->addWidget(color_correction_plot.chart_view);
	ui.frmColorCorrection->setLayout(color_plot_layout);

	QObject::connect(&color_correction, &Lift_Gamma_Gain::update_lift_gamma_gain, &color_correction_plot, &ColorPlotter::update_color_correction);
	QObject::connect(ui.btn_reset_color_correction, &QPushButton::clicked, this, &QtGuiApplication1::reset_color_correction);

	//---------------------------------------------------------------------------

	video_layout = new QGridLayout();
	video_layout->addWidget(ir_video->label);
	ui.frm_video->setLayout(video_layout);

	histogram_layout = new QGridLayout();
	histogram_layout->addWidget(histogram_plot->chart_view);
	//histogram_layout->addWidget(histogram_plot->text, 0, 0);
	ui.frm_histogram->setLayout(histogram_layout);

	//---------------------------------------------------------------------------

	// Link color correction sliders to changing color correction values
	QObject::connect(ui.slider_gain, &QSlider::valueChanged, this, &QtGuiApplication1::color_correction_toggled);
	QObject::connect(ui.slider_gamma, &QSlider::valueChanged, this, &QtGuiApplication1::color_correction_toggled);
	QObject::connect(ui.slider_lift, &QSlider::valueChanged, this, &QtGuiApplication1::color_correction_toggled);

	int max_lift, min_lift, max_gamma, min_gamma, max_gain, min_gain;
	color_correction.get_lift_slider_range(min_lift, max_lift);
	color_correction.get_gamma_slider_range(min_gamma, max_gamma);
	color_correction.get_gain_slider_range(min_gain, max_gain);
	
	ui.slider_lift->setMinimum(min_lift);
	ui.slider_lift->setMaximum(max_lift);
	ui.slider_gamma->setMinimum(min_gamma);
	ui.slider_gamma->setMaximum(max_gamma);
	ui.slider_gain->setMinimum(min_gain);
	ui.slider_gain->setMaximum(max_gain);

	reset_color_correction();

	//---------------------------------------------------------------------------

	ui.cmb_deinterlace_options->addItem("Max Absolute Value");
	ui.cmb_deinterlace_options->addItem("Centroid");
	ui.cmb_deinterlace_options->addItem("Avg Cross Correlation");

	QObject::connect(ui.cmb_deinterlace_options, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtGuiApplication1::change_deinterlace_options);

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
	
	QObject::connect(ui.btn_fast_forward, &QPushButton::clicked, this, &QtGuiApplication1::update_fps);
	QObject::connect(ui.btn_slow_back, &QPushButton::clicked, this, &QtGuiApplication1::update_fps);

	//---------------------------------------------------------------------------

	//Link buttons to functions
	QObject::connect(ui.btn_load_osm, &QPushButton::clicked, this, &QtGuiApplication1::load_osm_data);
	QObject::connect(ui.btn_get_frames, &QPushButton::clicked, this, &QtGuiApplication1::load_abir_data);

	QObject::connect(ui.btn_create_nuc, &QPushButton::clicked, this, &QtGuiApplication1::create_non_uniformity_correction);
	QObject::connect(ui.btn_bgs, &QPushButton::clicked, this, &QtGuiApplication1::create_background_subtraction_correction);
	QObject::connect(ui.btn_deinterlace, &QPushButton::clicked, this, &QtGuiApplication1::create_deinterlace);

	QObject::connect(ui.chk_apply_nuc, &QCheckBox::stateChanged, this, &QtGuiApplication1::toggle_video_filters);
	QObject::connect(ui.chk_bgs, &QCheckBox::stateChanged, this, &QtGuiApplication1::toggle_video_filters);
	QObject::connect(ui.chk_deinterlace, &QCheckBox::stateChanged, this, &QtGuiApplication1::toggle_video_filters);

	//---------------------------------------------------------------------------

	// Show Import Menu on load
	ui.tabMenu->setCurrentIndex(0);

	//---------------------------------------------------------------------------

	//Add icons to video playback buttons
	QPixmap play_image("icons/play.png");
	QIcon play_icon(play_image);
	ui.btn_play->setIcon(play_icon);
	ui.btn_play->setText("");

	QPixmap pause_image("icons/pause.png");
	QIcon pause_icon(pause_image);
	ui.btn_pause->setIcon(pause_icon);
	ui.btn_pause->setText("");

	QPixmap reverse_image("icons/reverse.png");
	QIcon reverse_icon(reverse_image);
	ui.btn_reverse->setIcon(reverse_icon);
	ui.btn_reverse->setText("");

	QPixmap speed_up_image("icons/chevron-double-up.png");
	QIcon speed_up_icon(speed_up_image);
	ui.btn_fast_forward->setIcon(speed_up_icon);
	ui.btn_fast_forward->setText("");

	QPixmap next_frame_image("icons/skip-next.png");
	QIcon next_frame_icon(next_frame_image);
	ui.btn_next_frame->setIcon(next_frame_icon);
	ui.btn_next_frame->setText("");

	QPixmap slow_down_image("icons/chevron-double-down.png");
	QIcon slow_down_icon(slow_down_image);
	ui.btn_slow_back->setIcon(slow_down_icon);
	ui.btn_slow_back->setText("");

	QPixmap prev_frame_image("icons/skip-previous.png");
	QIcon prev_frame_icon(prev_frame_image);
	ui.btn_prev_frame->setIcon(prev_frame_icon);
	ui.btn_prev_frame->setText("");

	QPixmap record_frame("icons/record.png");
	QIcon record_frame_icon(record_frame);
	ui.btn_frame_record->setIcon(record_frame_icon);
	ui.btn_frame_record->setText("");

	QPixmap save_frame("icons/content-save.png");
	QIcon save_frame_icon(save_frame);
	ui.btn_frame_save->setIcon(save_frame_icon);
	ui.btn_frame_save->setText("");

	ui.btn_save_plot->setIcon(save_frame_icon);
	ui.btn_save_plot->setText("");

	create_menu_actions();

	menu = new QMenu(this);
	menu->addAction(menu_osm);
	menu->addAction(menu_add_primary_data);
	menu->addAction(menu_sensor_boresight);
	menu->addAction(menu_add_banner);

	QPixmap menu_image("icons/menu.png");
	QIcon menu_icon(menu_image);
	ui.btn_video_menu->setIcon(menu_icon);
	ui.btn_video_menu->setText("");

	ui.btn_video_menu->setMenu(menu);

	show();

	INFO << "GUI: GUI Initialized";
}

QtGuiApplication1::~QtGuiApplication1() {

	delete ir_video;
	delete playback_controller;
	delete histogram_plot;

	delete videos;
	//delete ir_video;
	//delete histogram_plot;
	//delete data_plots;
	delete eng_data;

	DEBUG << "GUI: GUI destructor called";
}

void QtGuiApplication1::load_osm_data()
{

	INFO << "GUI: Start OSM load process";

	ui.txt_start_frame->setEnabled(false);
	ui.txt_end_frame->setEnabled(false);
	ui.btn_get_frames->setEnabled(false);
	ui.btn_load_osm->setEnabled(false);

	INFO << "GUI: Loading OSM data";
	bool valid_files = file_data.load_osm_file();
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

		return;
	}
		
	INFO << "GUI: OSM file has valid path";

	ui.lbl_file_load->setText(file_data.info_msg);
	ui.lbl_directory_path->setText(file_data.directory_path);
	ui.lbl_file_name->setText(file_data.file_name);

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

			// delete objects with existing data within them
			delete eng_data;
			delete data_plots;
			delete engineering_plot_layout;
		}

		DEBUG << "GUI: Creating new objects for engineering data, data plots, and layout";

		eng_data = new Engineering_Data(file_data.osm_data.data);
		data_plots = new Engineering_Plots();

		data_plots->frame_numbers = eng_data->frame_numbers;
		data_plots->past_midnight = eng_data->seconds_from_midnight;
		data_plots->index_sub_plot_xmin = 0;
		data_plots->index_sub_plot_xmax = data_plots->frame_numbers.size() - 1;

		data_plots->engineering_data = eng_data->frame_data;
		data_plots->track_irradiance_data = eng_data->track_irradiance_data;
		data_plots->plot_irradiance(eng_data->max_number_tracks);
			
		ui.tabPlots->setCurrentIndex(1);

		ui.cmb_plot_yaxis->clear();
		ui.cmb_plot_yaxis->setEnabled(true);
		ui.cmb_plot_yaxis->clear();
		ui.cmb_plot_yaxis->addItem(QString("Irradiance"));
		ui.cmb_plot_yaxis->addItem(QString("Azimuth"));
		ui.cmb_plot_yaxis->addItem(QString("Elevation"));
		ui.cmb_plot_yaxis->setCurrentIndex(0);

		ui.cmb_plot_xaxis->clear();
		ui.cmb_plot_xaxis->setEnabled(true);
		ui.cmb_plot_xaxis->clear();
		ui.cmb_plot_xaxis->addItem(QString("Frames"));
		ui.cmb_plot_xaxis->addItem(QString("Seconds from Midnight"));
		//ui.cmb_plot_xaxis->addItem(QString("Seconds from Epoch"));
		ui.cmb_plot_xaxis->setCurrentIndex(0);

		ui.chk_plot_full_data->setEnabled(true);
		ui.chk_plot_primary_data->setEnabled(true);
		ui.chk_plot_show_line->setEnabled(true);
		connect(ui.chk_plot_full_data, &QCheckBox::stateChanged, this, &QtGuiApplication1::plot_full_data);
		connect(ui.chk_plot_primary_data, &QCheckBox::stateChanged, this, &QtGuiApplication1::plot_primary_only);
		connect(ui.chk_plot_show_line, &QCheckBox::stateChanged, this, &QtGuiApplication1::plot_current_frame_marker);
						
		connect(ui.cmb_plot_yaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtGuiApplication1::plot_change);
		connect(ui.cmb_plot_xaxis, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QtGuiApplication1::plot_change);

		engineering_plot_layout = new QGridLayout();
		engineering_plot_layout->addWidget(data_plots->chart_view);
		ui.frm_plots->setLayout(engineering_plot_layout);

		ui.btn_save_plot->setEnabled(true);
		connect(ui.btn_save_plot, &QPushButton::clicked, this, &QtGuiApplication1::save_plot);

		INFO << "GUI: OSM successfully loaded";

	}

	ui.btn_load_osm->setEnabled(true);

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

	// if non-numeric data is entered...
	if ((min_frame < 0) || (max_frame < 0))
	{
		if (min_frame < 0)
			DEBUG << "GUI: User entered non-numeric data to start frame. Entered: " << ui.txt_nuc_start->text().toLocal8Bit().constData();
		if (min_frame < 0)
			DEBUG << "GUI: User entered non-numeric data to stop frame. Entered: " << ui.txt_nuc_stop->text().toLocal8Bit().constData();

		INFO << "GUI: No video loaded";

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Non-Numeric Data"));
		msgBox.setText("Non-numeric data entered for video load min/max frames");

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();

		ui.btn_get_frames->setEnabled(true);
		return;
	}

	//find frame start / stop 
	int frame_start = 0;
	int frame_stop = eng_data->frame_numbers.back();

	bool min_within_range = check_value_within_range(min_frame, frame_start, frame_stop);
	bool max_within_range = check_value_within_range(max_frame, frame_start, frame_stop);

	// if values outside range...
	if (!min_within_range || !max_within_range)
	{
		if (min_frame < frame_start)
			DEBUG << "GUI: Start frame before minimum frame. Entered: " << std::to_string(min_frame) << " Minimum: " << std::to_string(frame_start);
		if (min_frame < 0)
			DEBUG << "GUI: Stop frame after maximum frame. Entered: " << std::to_string(max_frame) << "Maximum: " << std::to_string(frame_stop);

		INFO << "GUI: Video load not completed";

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Outside of Data Range"));
		QString box_text = "Data must be within valid range (" + QString::number(frame_start) + "-" + QString::number(frame_stop) + ")";
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
		
		ui.btn_get_frames->setEnabled(true);

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
	std::vector<std::vector<uint16_t>> video_frames = file_data.load_image_file(min_frame, max_frame, 4.2);
		
	int x_pixels = file_data.abir_data.ir_data[0].header.image_x_size;
	int y_pixels = file_data.abir_data.ir_data[0].header.image_y_size;

	DEBUG << "GUI: Frames are of size " << std::to_string(x_pixels) << " x " << std::to_string(y_pixels);
		
	primary.set_image_size(x_pixels, y_pixels);
	primary.set_video_frames(video_frames);
	videos->something.push_back(primary);
		
	int number_frames = primary.frames_16bit.size();
	QString status_txt = ui.lbl_file_load->text();
	QString update_text("\nFrames ");
	update_text.append(QString::number(min_frame));
	update_text.append(" to ");
	update_text.append(QString::number(max_frame));
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
	playback_controller->start_timer();

	std::vector<Plotting_Frame_Data>::const_iterator first = eng_data->frame_data.begin() + min_frame - 1;
	std::vector<Plotting_Frame_Data>::const_iterator last = eng_data->frame_data.begin() + (min_frame - 1) + max_frame;
	std::vector<Plotting_Frame_Data> subset_data(first, last);
	ir_video->set_frame_data(subset_data);

	// Reset engineering plots with new sub plot indices
	data_plots->index_sub_plot_xmin = min_frame - 1;
	data_plots->index_sub_plot_xmax = max_frame - 1;
	plot_change(1);

	//Enable saving frame
	ui.btn_frame_save->setEnabled(true);
	connect(ui.btn_frame_save, &QPushButton::clicked, this, &QtGuiApplication1::save_frame);

	//Update frame number label
	connect(playback_controller, &Playback::update_frame, this, &QtGuiApplication1::set_frame_number_label);

	//Update frame marker on engineering plot
	connect(playback_controller, &Playback::update_frame, data_plots, &Engineering_Plots::plot_current_step);
		
	update_fps();

	ui.tabPlots->setCurrentIndex(0);

	ui.btn_get_frames->setEnabled(true);

	ui.tabMenu->setTabEnabled(1, true);
	ui.tabMenu->setTabEnabled(2, true);

	INFO << "GUI: ABIR file load complete";

	//---------------------------------------------------------------------------
}

void QtGuiApplication1::update_fps()
{
	QString fps = QString::number(playback_controller->speeds[playback_controller->index_speed], 'g', 2);
	fps.append(" fps");

	ui.lbl_fps->setText(fps);
}

void QtGuiApplication1::color_correction_toggled(int value) {
		
		
	double lift_value = color_correction.lift_convert_slider_to_value(ui.slider_lift->value()); 
	double gamma_value = color_correction.gamma_convert_slider_to_value(ui.slider_gamma->value());
	double gain_value = color_correction.gain_convert_slider_to_value(ui.slider_gain->value());
		
	emit color_correction.update_lift_gamma_gain(lift_value, gamma_value, gain_value);
	emit color_correction_plot.update_color_chart();

	set_color_correction_slider_labels();

	if (!playback_controller->timer->isActive() && file_data.valid_image)
	{
		int counter_value = playback_controller->get_counter();
		ir_video->update_specific_frame(counter_value);
		histogram_plot->update_specific_histogram(counter_value);
	}		

	DEBUG << "GUI: New values set for Lift/Gamma/Gain correction: " << std::to_string(lift_value) << "/" << std::to_string(gamma_value) << "/" << std::to_string(gain_value);
}

void QtGuiApplication1::reset_color_correction()
{
	double min_lift = color_correction.get_min_lift();
	int base_lift = min_lift * 100;
	if (base_lift < 0) {
		base_lift *= -1;
	}
		
	ui.slider_lift->setValue(base_lift);
	ui.slider_gain->setValue(100);
	ui.slider_gamma->setValue(100);

	DEBUG << "GUI: Color correction reset";
}

void QtGuiApplication1::allow_epoch()
{

}

void QtGuiApplication1::plot_full_data(int index)
{
	if (ui.chk_plot_full_data->isChecked())
	{
		data_plots->plot_all_data = true;
		data_plots->toggle_subplot();
		DEBUG << "GUI: Toggle plot all data ";
	}
	else 
	{
		data_plots->plot_all_data = false;
		data_plots->toggle_subplot();
		DEBUG << "GUI: Toggle plot sub-set data ";
	}		
}

void QtGuiApplication1::plot_primary_only()
{
	if (ui.chk_plot_primary_data->isChecked())
		data_plots->plot_primary_only = true;
	else
		data_plots->plot_primary_only = false;

	plot_change(1);
}

void QtGuiApplication1::plot_current_frame_marker() {
	if (ui.chk_plot_show_line->isChecked())
		data_plots->plot_current_marker = true;
	else
		data_plots->plot_current_marker = false;

	plot_change(1);
}

void QtGuiApplication1::save_plot()
{
	data_plots->save_plot();
	INFO << "GUI: Plot saved";
}

void QtGuiApplication1::save_frame()
{
	ir_video->save_frame();
	INFO << "GUI: Video frame saved  ";
}

void QtGuiApplication1::create_menu_actions()
{

	menu_osm = new QAction(tr("&Toggle OSM Tracks"), this);
	menu_osm->setStatusTip(tr("Shows pixels OSM is tracking"));
	connect(menu_osm, &QAction::triggered, ir_video, &Video::toggle_osm_tracks);

	menu_add_primary_data = new QAction(tr("&Toggle Primary Tgt Metrics"), this);
	menu_add_primary_data->setStatusTip(tr("Shows Az/El of Primary Tgt"));
	connect(menu_add_primary_data, &QAction::triggered, ir_video, &Video::toggle_primary_track_data);

	menu_sensor_boresight = new QAction(tr("&Toggle Sensor Metrics"), this);
	menu_sensor_boresight->setStatusTip(tr("Shows Az/El of Sensor Pointing"));
	connect(menu_sensor_boresight, &QAction::triggered, ir_video, &Video::toggle_sensor_boresight_data);

	menu_add_banner = new QAction(tr("&Edit Banner"), this);
	menu_add_banner->setStatusTip(tr("Edit banner text to video"));
	connect(menu_add_banner, &QAction::triggered, this, &QtGuiApplication1::edit_banner_text);
	connect(this, &QtGuiApplication1::change_banner, ir_video, &Video::update_banner_text);
}

void QtGuiApplication1::edit_banner_text()
{
	QString input_text = QInputDialog::getText(0, "Banner Text", "Input Banner Text", QLineEdit::Normal);
		
	emit change_banner(input_text, QColor("Red"));

	DEBUG << "GUI: Banner text changed";
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

	DEBUG << "GUI: Engineering plot units changed to case " << std::to_string(x_index);
	DEBUG << "GUI: Engineering plot changed to case " << std::to_string(y_index);

	switch (x_index)
	{
	case 0:
		data_plots->x_axis_units = frames;
		break;

	case 1:
		data_plots->x_axis_units = seconds_past_midnight;
		break;

	case 2:
		data_plots->x_axis_units = seconds_past_midnight;
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

}

int QtGuiApplication1::get_integer_from_txt_box(QString input)
{
	bool convert_value_numeric;
	int value = input.toInt(&convert_value_numeric);
		
	if (convert_value_numeric) {
		DEBUG << "GUI: Integer from text box is " << std::to_string(value);
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
		DEBUG << "GUI: " << std::to_string(input_value) << "is less than min value of " << std::to_string(min_value);
		return false;
	}
	if (input_value > max_value) {
		DEBUG << "GUI: " << std::to_string(input_value) << "is more than max value of " << std::to_string(max_value);
		return false;
	}
		
	DEBUG << "GUI: " << std::to_string(input_value) << " is between " << std::to_string(min_value) << " and " << std::to_string(max_value);

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

void QtGuiApplication1::create_non_uniformity_correction()
{
	QProgressDialog progress("", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.setWindowTitle(QString("Non-Uniformity Correction"));
	progress.setLabelText(QString("Copying data for non-uniformity correction..."));

	INFO << "GUI: Creating non-uniformity correction file from original data";

	video_details nuc_video;
	video_details original = videos->something[0];
		
	//----------------------------------------------------------------------------------------------------
	// Get frame numbers from text boxes
	int min_frame = get_integer_from_txt_box(ui.txt_nuc_start->text());
	int max_frame = get_integer_from_txt_box(ui.txt_nuc_stop->text());

	// if non-numeric data is entered...
	if ((min_frame < 0) || (max_frame < 0))
	{
		if(min_frame < 0)
			DEBUG << "GUI: User entered non-numeric data to start frame. Entered: " << ui.txt_nuc_start->text().toLocal8Bit().constData();
		if (min_frame < 0)
			DEBUG << "GUI: User entered non-numeric data to stop frame. Entered: " << ui.txt_nuc_stop->text().toLocal8Bit().constData();

		INFO << "GUI: NUC correction not completed";

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Non-Numeric Data"));
		msgBox.setText("Non-numeric data entered for NUC min/max frames");
			
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
		return;
	}
		
	//find frame start / stop 
	int frame_start = 0; 
	int frame_stop = eng_data->frame_numbers.back(); 

	bool min_within_range = check_value_within_range(min_frame, frame_start, frame_stop);
	bool max_within_range = check_value_within_range(max_frame, frame_start, frame_stop);

	// if values outside range...
	if (!min_within_range || !max_within_range)
	{
		if (min_frame < frame_start)
			DEBUG << "GUI: Start frame before minimum frame. Entered: " << std::to_string(min_frame) << " Minimum: " << std::to_string(frame_start);
		if (min_frame < 0)
			DEBUG << "GUI: Stop frame after maximum frame. Entered: " << std::to_string(max_frame) << "Maximum: " << std::to_string(frame_stop);

		INFO << "GUI: NUC correction not completed";
			
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Outside of Data Range"));
		QString box_text = "Data must be within valid range (" + QString::number(frame_start) + "-" + QString::number(frame_stop) + ")";
		msgBox.setText(box_text);

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
		return;
	}


	//----------------------------------------------------------------------------------------------------

	NUC nuc(file_data.image_path, min_frame, max_frame, file_data.file_version);
	std::vector<double> nuc_correction = nuc.get_nuc_correction();

	INFO << "Calculated NUC correction";

	nuc_video = original;
	nuc_video.clear_16bit_vector();
	nuc_video.clear_8bit_vector();
	nuc_video.histogram_data.clear();

	nuc_video.properties[Video_Parameters::original] = false;
	nuc_video.properties[Video_Parameters::non_uniformity_correction] = true;

	// Apply NUC to the frames		
	int number_frames = original.frames_16bit.size();

		
	progress.setMinimum(0);
	progress.setMaximum(number_frames - 1);
	progress.setLabelText(QString("Applying non-uniformity correction..."));

	for (int i = 0; i < number_frames; i++) {
		progress.setValue(i);
		DEBUG << "GUI: Applying NUC correction to " << std::to_string(i + 1) << " of " << std::to_string(number_frames) << "frames";
		nuc_video.frames_16bit.push_back(nuc.apply_nuc_correction(original.frames_16bit[i]));
		if (progress.wasCanceled())
			break;
	}

	if (progress.wasCanceled()) {
		INFO << "GUI: NUC process was canceled";
		return;
	}

	progress.setLabelText(QString("Down-converting video and creating histogram data..."));

	nuc_video.convert_16bit_to_8bit();
	nuc_video.create_histogram_data();		

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
		
	//TODO if more than one correction already exists, then apply NUC to it as well ? 
		
}

void QtGuiApplication1::create_deinterlace()
{
	INFO << "GUI: Creating de-interlace file from original data";

	QProgressDialog progress("", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setValue(0);
	progress.setLabelText(QString("Copying data for de-interlace..."));
	progress.setWindowTitle(QString("De-interlace Frames"));
		
	video_details deinterlace_video;
	video_details original = videos->something[0];

	deinterlace_type deinterlace_method_type = find_deinterlace_type(ui.cmb_deinterlace_options->currentIndex());
	Video_Parameters deinterlace_video_type = find_deinterlace_video_type(ui.cmb_deinterlace_options->currentIndex());

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
		
	progress.setMaximum(number_frames - 1);
	progress.setLabelText(QString("Creating de-interlaced frames..."));

	for (int i = 0; i < number_frames; i++)
	{
		progress.setValue(i);
		DEBUG << "GUI: Applying de-interlace to " << std::to_string(i + 1) << " of " << std::to_string(number_frames) << "frames";
		deinterlace_video.frames_16bit.push_back(deinterlace_method.deinterlace_frame(original.frames_16bit[i]));
		if (progress.wasCanceled())
			break;
	}

	if (progress.wasCanceled()) {
		INFO << "De-interlace process was canceled";
		return;
	}

	progress.setLabelText(QString("Down-converting video and creating histogram data..."));

	deinterlace_video.convert_16bit_to_8bit();
	deinterlace_video.create_histogram_data();

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

	change_deinterlace_options(ui.cmb_deinterlace_options->currentIndex());
}

void QtGuiApplication1::change_deinterlace_options(int index)
{
	Video_Parameters deinterlace_type = find_deinterlace_video_type(index);

	video_details deinterlace_check;
	deinterlace_check.properties[deinterlace_type] = true;

	int index_deinterlace_video = videos->find_data_index(deinterlace_check);

	if (index_deinterlace_video > 0)
	{
		ui.chk_deinterlace->setEnabled(true);
		if (ui.chk_deinterlace->isChecked())
			toggle_video_filters();

		DEBUG << "GUI: New de-interlace method selected and video found";
	}
	else {
		ui.chk_deinterlace->setCheckState(Qt::Unchecked);
		ui.chk_deinterlace->setEnabled(false);

		DEBUG << "GUI: New de-interlace method selected and video not found";
	}

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

void QtGuiApplication1::create_background_subtraction_correction() {

	INFO << "GUI: Background subtraction video being created";
	
	video_details background_subraction_video;
	video_details original = videos->something[0];

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
	int frame_start = 0;
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
		DEBUG << "GUI: Applying background subtraction to " << std::to_string(i + 1) << " of " << std::to_string(number_frames) << "frames";
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

	background_subraction_video.convert_16bit_to_8bit();
	background_subraction_video.create_histogram_data();

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
}

void QtGuiApplication1::toggle_video_filters()
{
	video_details user_requested;

	INFO << "GUI: User toggled new video for display";

	user_requested.properties[Video_Parameters::original] = true;
	user_requested.properties[Video_Parameters::non_uniformity_correction] = false;
	user_requested.properties[Video_Parameters::background_subtraction] = false;

	if (ui.chk_apply_nuc->checkState())
	{
		user_requested.properties[Video_Parameters::original] = false;
		user_requested.properties[Video_Parameters::non_uniformity_correction] = false;
		user_requested.properties[Video_Parameters::background_subtraction] = false;
		user_requested.properties[Video_Parameters::non_uniformity_correction] = true;
	}
	if (ui.chk_bgs->checkState()) {
		user_requested.properties[Video_Parameters::original] = false;
		user_requested.properties[Video_Parameters::non_uniformity_correction] = false;
		user_requested.properties[Video_Parameters::background_subtraction] = false;
		user_requested.properties[Video_Parameters::background_subtraction] = true;
	}

	if (ui.chk_deinterlace->checkState())
	{
		user_requested.properties[Video_Parameters::original] = false;
		user_requested.properties[Video_Parameters::non_uniformity_correction] = false;
		user_requested.properties[Video_Parameters::background_subtraction] = false;
		Video_Parameters deinterlace_video_type = find_deinterlace_video_type(ui.cmb_deinterlace_options->currentIndex());
		user_requested.properties[deinterlace_video_type] = true;
	}

	videos->display_data(user_requested);
}

void QtGuiApplication1::set_color_correction_slider_labels()
{
	double lift_value = color_correction.lift_convert_slider_to_value(ui.slider_lift->value());
	double gamma_value = color_correction.gamma_convert_slider_to_value(ui.slider_gamma->value());
	double gain_value = color_correction.gain_convert_slider_to_value(ui.slider_gain->value());
		
	QString lift_string;
	lift_string.setNum(lift_value);
	ui.lbl_lift_value->setText(lift_string);

	QString gamma_string;
	gamma_string.setNum(gamma_value);
	ui.lbl_gamma_value->setText(gamma_string);

	QString gain_string;
	gain_string.setNum(gain_value);
	ui.lbl_gain_value->setText(gain_string);
}
