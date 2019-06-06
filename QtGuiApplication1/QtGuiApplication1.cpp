#include "QtGuiApplication1.h"


QtGuiApplication1::QtGuiApplication1(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	videos = new Video_Container(); 

	//---------------------------------------------------------------------------

	playback_controller = new Playback(1);
	
	//---------------------------------------------------------------------------	

	ir_video = new Video(640, 480);
	
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
	//video_layout->addWidget(ir_video->text, 0, 0);
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

	// Link horizontal slider to playback controller
	QObject::connect(playback_controller, &Playback::update_frame, ui.sldrVideo, &QSlider::setValue);
	QObject::connect(ui.sldrVideo, &QSlider::valueChanged, playback_controller, &Playback::set_counter);

	//---------------------------------------------------------------------------

	// Link playback to play controls
	QObject::connect(ui.btn_play_pause, &QPushButton::clicked, this, &QtGuiApplication1::video_toggled);
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

	//---------------------------------------------------------------------------

	// Show Import Menu on load
	ui.tabMenu->setCurrentIndex(0);

	//---------------------------------------------------------------------------

	//Add icons to video playback buttons
	QPixmap play_image("play-pause.png");
	QIcon play_icon(play_image);
	ui.btn_play_pause->setIcon(play_icon);
	ui.btn_play_pause->setText("");

	QPixmap speed_up_image("skip-forward.png");
	QIcon speed_up_icon(speed_up_image);
	ui.btn_fast_forward->setIcon(speed_up_icon);
	ui.btn_fast_forward->setText("");

	QPixmap next_frame_image("skip-next.png");
	QIcon next_frame_icon(next_frame_image);
	ui.btn_next_frame->setIcon(next_frame_icon);
	ui.btn_next_frame->setText("");

	QPixmap slow_down_image("skip-backward.png");
	QIcon slow_down_icon(slow_down_image);
	ui.btn_slow_back->setIcon(slow_down_icon);
	ui.btn_slow_back->setText("");

	QPixmap prev_frame_image("skip-previous.png");
	QIcon prev_frame_icon(prev_frame_image);
	ui.btn_prev_frame->setIcon(prev_frame_icon);
	ui.btn_prev_frame->setText("");

	create_menu_actions();

	menu = new QMenu(this);
	menu->addAction("OSM Tracks");
	menu->addAction("Primary Target Metrics");
	menu->addAction("Sensor Boresight");
	menu->addAction(menu_add_banner);

	QPixmap menu_image("menu.png");
	QIcon menu_icon(menu_image);
	ui.btn_video_menu->setIcon(menu_icon);
	ui.btn_video_menu->setText("");

	ui.btn_video_menu->setMenu(menu);

	show();
}

	QtGuiApplication1::~QtGuiApplication1() {

		delete ir_video;
		delete playback_controller;
		delete histogram_plot;
		//delete engineering_data;

		//delete main_widget;
		//delete main_layout;
	}

	void QtGuiApplication1::video_toggled()
	{
		if (playback_controller->timer->isActive())
		{
			playback_controller->stop_timer();
		}
		else
		{
			playback_controller->start_timer();
		}

	}

	void QtGuiApplication1::load_osm_data()
	{

		ui.txt_start_frame->setEnabled(false);
		ui.txt_end_frame->setEnabled(false);
		ui.btn_get_frames->setEnabled(false);
		ui.btn_load_osm->setEnabled(false);

		bool valid_files = file_data.load_osm_file();

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

			eng_data = new Engineering_Data(file_data.osm_data.data);
			data_plots = new Engineering_Plots();

			QObject::connect(eng_data, &Engineering_Data::plot_luminosity, data_plots, &Engineering_Plots::plot_irradiance_data);

			eng_data->get_luminosity_data();
			ui.tabPlots->setCurrentIndex(1);

			engineering_plot_layout = new QGridLayout();
			engineering_plot_layout->addWidget(data_plots->chart_view);
			ui.frm_plots->setLayout(engineering_plot_layout);

		}

		ui.btn_load_osm->setEnabled(true);

		return;
	}

	void QtGuiApplication1::load_abir_data()
	{

		ui.btn_get_frames->setEnabled(false);

		// Get frame numbers from text boxes
		QString min_frame_text = ui.txt_start_frame->text();
		QString max_frame_text = ui.txt_end_frame->text();
		
		// Convert strings to integers
		bool converted_min_frame, converted_max_frame;
		unsigned int min_frame = min_frame_text.toInt(&converted_min_frame);
		unsigned int max_frame = max_frame_text.toInt(&converted_max_frame);
		
		// TODO test that inputs are valid (inputs are numbers and greater than zero)

		// Output converted data back to text box to provide feedback to user
		ui.txt_start_frame->setText(QString::number(min_frame));
		ui.txt_end_frame->setText(QString::number(max_frame));

		// Load the ABIR data
		std::vector<unsigned int> frame_numbers{ min_frame, max_frame };		
		bool load_complete = file_data.load_image_file(min_frame, max_frame, 4.2);

		//---------------------------------------------------------------------------

		//abir_data = file_data.abir_data;

		int x_pixels = file_data.abir_data.ir_data[0].header.image_x_size;
		int y_pixels = file_data.abir_data.ir_data[0].header.image_y_size;
		int number_frames = file_data.abir_data.video_frames_16bit.size();

		QString status_txt = ui.lbl_file_load->text();
		QString update_text("\nFrames ");
		update_text.append(QString::number(min_frame));
		update_text.append(" to ");
		update_text.append(QString::number(max_frame));
		update_text.append(" were loaded");

		status_txt.append(update_text);
		ui.lbl_file_load->setText(status_txt);

		//---------------------------------------------------------------------------

		// Create the video properties data
		video_details primary;
		primary.properties[Video_Parameters::original] = true;
		primary.set_image_size(x_pixels, y_pixels);
		primary.set_number_of_bits(14);
		primary.set_video_frames(file_data.abir_data.video_frames_16bit);
		
		videos->something.push_back(primary);

		//---------------------------------------------------------------------------

		// Set frame number for playback controller and valid values for slider
		playback_controller->set_number_of_frames(number_frames);
		ui.sldrVideo->setRange(0, number_frames);	

		//---------------------------------------------------------------------------

		QObject::connect(eng_data, &Engineering_Data::plot_azimuth_elevation, data_plots, &Engineering_Plots::plot_az_el_boresite_data);
		eng_data->get_azimuth_elevation_data();

		//---------------------------------------------------------------------------

		// Set the video and histogram plots to this data
		videos->display_original_data();
		playback_controller->start_timer();
		
		update_fps();

		ui.tabPlots->setCurrentIndex(0);

		ui.btn_get_frames->setEnabled(true);

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
	}

	void QtGuiApplication1::create_menu_actions()
	{

		menu_add_banner = new QAction(tr("&Add Banner"), this);
		menu_add_banner->setStatusTip(tr("Add banner text to video"));
		connect(menu_add_banner, &QAction::triggered, this, &QtGuiApplication1::edit_banner_text);
		connect(this, &QtGuiApplication1::change_banner, ir_video, &Video::update_banner_text);
	}

	void QtGuiApplication1::edit_banner_text()
	{
		QString input_text = QInputDialog::getText(0, "Banner Text", "Input Banner Text", QLineEdit::Normal);
		
		emit change_banner(input_text, QColor("Red"));
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