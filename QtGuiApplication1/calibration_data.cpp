#include "calibration_data.h"

CalibrationData::CalibrationData()
{
	// set check variable to false to start
	calibration_available = false;

}

CalibrationData::~CalibrationData()
{

}

double CalibrationData::measure_irradiance(int ul_row, int ul_col, int lr_row, int lr_col, arma::mat x)
{
	
	arma::mat sub_b = b.submat(ul_row, ul_col, lr_row, lr_col);
	arma::mat sub_m = m.submat(ul_row, ul_col, lr_row, lr_col);

	arma::mat radiance = sub_m * x + sub_b;

	double average_radiance = arma::mean(arma::mean(radiance));

	return average_radiance;
}

void CalibrationData::setup_model(arma::mat input_m, arma::mat input_b)
{
	calibration_available = true;
	m = input_m;
	b = input_b;
}

bool CalibrationData::set_calibration_details(QString path_to_nuc, QString path_to_image, SelectedData selection1, SelectedData selection2)
{

	bool valid_paths = check_path(path_to_nuc) && check_path(path_to_image);

	if (!valid_paths)
		return false;
		
	path_nuc = path_to_nuc;
	path_image = path_to_image;
	
	user_selection1 = selection1;
	user_selection2 = selection2;

	return true;
}

bool CalibrationData::check_path(QString path)
{
	QString info_msg("");

	QFileInfo check_file(path);
	bool file_isFile = check_file.isFile();
	bool file_exists = check_file.exists();

	return file_exists && file_isFile;
}

// --------------------------------------------------------------------------------------------------------------------

CalibrationDialog::CalibrationDialog(CalibrationData & input_model, QWidget* parent)
{

	// initiliaze chart parameters
	chart_temperature = new QChart();
	chart_view_temperatures = new Clickable_QChartView(chart_temperature);

	initialize_gui();

	selection1 = new QLineSeries();
	selection2 = new QLineSeries();

	if (input_model.calibration_available) {

		user_selection1 = input_model.user_selection1;
		user_selection2 = input_model.user_selection2;

		path_nuc = input_model.path_nuc;
		path_image = input_model.path_image;

		import_nuc_file();

		draw_series(user_selection1);
		update_user_selection_labels(user_selection1);

		draw_series(user_selection2);
		update_user_selection_labels(user_selection2);

		btn_ok->setEnabled(true);
	}

	else {

		user_selection1.valid_data = false;
		user_selection1.color = colors.Get_Color(1);
		user_selection1.id = 1;

		user_selection2.valid_data = false;
		user_selection2.color = colors.Get_Color(3);
		user_selection2.id = 2;
	}

}

CalibrationDialog::~CalibrationDialog()
{
	delete selection1;
	delete selection2;
}

void CalibrationDialog::initialize_gui()
{

	// define dialog buttons
	btn_ok = new QPushButton(tr("OK"));
	btn_ok->setEnabled(false);
	btn_cancel = new QPushButton(tr("Cancel"));

	connect(btn_ok, &QPushButton::pressed, this, &CalibrationDialog::ok);
	connect(btn_cancel, &QPushButton::pressed, this, &CalibrationDialog::close_window);

	// ------------------------------------------------------------
	// set layout

	mainLayout = new QVBoxLayout();

	btn_get_nuc_file = new QPushButton("Import NUC File");
	lbl_nuc_filename = new QLabel("File: ");

	QFrame* horizontal_segment1 = new QFrame();
	horizontal_segment1->setFrameShape(QFrame::HLine);

	mainLayout->addWidget(btn_get_nuc_file, Qt::AlignCenter);
	mainLayout->addWidget(lbl_nuc_filename, Qt::AlignCenter);
	mainLayout->addWidget(horizontal_segment1);

	mainLayout->setStretch(0, 0);
	mainLayout->setStretch(1, 0);
	mainLayout->setStretch(2, 0);

	// ------------------------------------------------------------
	
	frame_plot = new FixedAspectRatioFrame();
	frame_plot->enable_fixed_aspect_ratio(true);
	frame_plot->resize(640, 480);

	QHBoxLayout* hlayout_plot = new QHBoxLayout();
	hlayout_plot->addWidget(chart_view_temperatures);
	frame_plot->setLayout(hlayout_plot);

	mainLayout->addWidget(frame_plot);
	//mainLayout->setStretch(3, 1);

	// ------------------------------------------------------------

	QLabel* description = new QLabel("Select two temperature ranges to calibrate between. To select a temperature range, left click and drag across the chart");

	radio_temperature1 = new QRadioButton(QString("Temperature #1: No data available"));
	radio_temperature2 = new QRadioButton(QString("Temperature #2: No data available"));
	QButtonGroup group_temperatures;
	group_temperatures.addButton(radio_temperature1);
	group_temperatures.addButton(radio_temperature2);
	radio_temperature1->setChecked(true);
	radio_temperature1->setEnabled(false);
	radio_temperature2->setEnabled(false);	

	QHBoxLayout* hlayout_temperature_info = new QHBoxLayout();
	hlayout_temperature_info->addWidget(radio_temperature1);
	hlayout_temperature_info->addWidget(radio_temperature2);

	hlayout_temperature_info->setStretch(0, 0);
	hlayout_temperature_info->setStretch(1, 0);
	hlayout_temperature_info->insertStretch(1, 0);

	mainLayout->addWidget(description);
	mainLayout->setStretch(4, 0);

	mainLayout->addLayout(hlayout_temperature_info);
	mainLayout->setStretch(5, 0);

	// ------------------------------------------------------------

	QHBoxLayout* hlayout_buttons = new QHBoxLayout();
	hlayout_buttons->addWidget(btn_ok, Qt::AlignCenter);
	hlayout_buttons->addWidget(btn_cancel, Qt::AlignCenter);

	hlayout_buttons->setStretch(0, 0);
	hlayout_buttons->setStretch(1, 0);
	hlayout_buttons->insertStretch(0, 0);

	mainLayout->addLayout(hlayout_buttons);
	mainLayout->setStretch(6, 0);

	// ------------------------------------------------------------
	// setup connections
	QObject::connect(btn_get_nuc_file, &QPushButton::clicked, this, &CalibrationDialog::get_new_nuc_file);

	// ------------------------------------------------------------

	setLayout(mainLayout);
	setWindowTitle("Set Calibration Data");

	// resize width
	int width = this->width();
	double ar = frame_plot->aspect_ratio_width * 1.0 / frame_plot->aspect_ratio_height;
	int height = int(width / ar);
	frame_plot->setFixedHeight(height);

}

void CalibrationDialog::get_new_nuc_file()

{
	QString user_selection = QFileDialog::getOpenFileName(this, ("Open Calibration File"), "", ("NUC File(*.abpnuc)"));

	// if no image path is selected then reset image path and return
	int compare = QString::compare(user_selection, "", Qt::CaseInsensitive);
	if (compare == 0) {
	
		return;
	}

	path_nuc = user_selection;

	import_nuc_file();
}

void CalibrationDialog::import_nuc_file()
{
			 
	// -----------------------------------------------------------------------------
	// import the abpnuc data 

	QByteArray ba = path_nuc.toLocal8Bit();
	char* file_path = ba.data();
	ABPNUC_Data nuc_data(file_path);
	
	if (nuc_data.read_status == 0) {

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Issue Reading File"));
		QString box_text("Error occurred when reading from apbnuc file");
		msgBox.setText(box_text);

		return;
	}

	if (nuc_data.number_of_frames == 0) {
		
		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Issue Reading File"));
		QString box_text("Zero frames read from apbnuc file");
		msgBox.setText(box_text);

		return;
	}

	// -----------------------------------------------------------------------------
	// retrieve relevant data from calibration data
	get_plotting_data(nuc_data);

	// -----------------------------------------------------------------------------
	// get file name to display
	int index_file_start, index_file_end;
	index_file_start = path_nuc.lastIndexOf("/");
	index_file_end = path_nuc.lastIndexOf(".");
	
	QString filename = QString("File: ");
	filename.append(path_nuc.mid(index_file_start + 1, index_file_end - index_file_start - 1));

	lbl_nuc_filename->setText(filename);

	return;
}

void CalibrationDialog::point_selected(double x0, double x1) {

	if (radio_temperature1->isChecked())
	{
		show_user_selection(user_selection1, x0, x1);
		update_user_selection_labels(user_selection1);

	}
	else
	{
		show_user_selection(user_selection2, x0, x1);
		update_user_selection_labels(user_selection2);

	}

	if (user_selection1.valid_data && user_selection2.valid_data) {
		btn_ok->setEnabled(true);
	}
	else {
		btn_ok->setEnabled(false);
	}


}

void CalibrationDialog::show_user_selection(SelectedData &user_selection, double x0, double x1) {

	
	std::vector<double> vector_temperature;
	
	
	if (user_selection.points.size() > 0) {
		user_selection.points.clear();
	}

	int num_points = temperature.length();
	int initial_frame = 0;

	for (int i = 0; i < num_points; i++)
	{
		if (temperature[i].x() >= std::floor(x0) && temperature[i].x() < std::ceil(x1)) {
			user_selection.points.append(temperature[i]);
			vector_temperature.push_back(temperature[i].y());

			if (vector_temperature.size() == 1) {
				user_selection.start_time = all_frame_times[i];
				user_selection.initial_frame = i;
			}
		}
	}

	arma::vec arma_temperatures(vector_temperature);

	user_selection.temperature_mean = arma::mean(arma_temperatures);
	user_selection.num_frames = arma_temperatures.size();
	user_selection.temperature_std = arma::stddev(arma_temperatures);
	user_selection.stop_time = all_frame_times[(user_selection.initial_frame + user_selection.num_frames) - 1];
	
	if (user_selection.points.size() > 0) {

		user_selection.valid_data = true;
		draw_series(user_selection);
	}
	else {
		user_selection.valid_data = false;
	}


}

void CalibrationDialog::draw_series(SelectedData& data)
{
	data.valid_data = true;

	QPen pen;
	pen.setColor(data.color);
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(3);

	QLineSeries* series;

	if (data.id == 1) {
		series = selection1;
	}
	if (data.id == 2) {
		series = selection2;
	}


	series->clear();
	for (int i = 0; i < data.points.size(); i++)
	{
		series->append(data.points[i].x(), data.points[i].y());
	}

	series->setPen(pen);
	chart_temperature->addSeries(series);
	draw_axes();
}

void CalibrationDialog::update_user_selection_labels(SelectedData& data) {

	QRadioButton* radio_temperature;

	if (data.id == 1) {
		radio_temperature = radio_temperature1;
	}
	if (data.id == 2) {
		radio_temperature = radio_temperature2;
	}


	if (data.valid_data) {
		radio_temperature->setText(QString("Temperature #%4: %1C, +/- %2 [Over %3 frames]").arg(data.temperature_mean, 0, 'f', 2).arg(data.temperature_std, 0, 'f', 3).arg(data.num_frames).arg(data.id));
	}
	else {
		radio_temperature->setText(QString("Temperature #%1: No data available").arg(data.id));
	}

}

void CalibrationDialog::ok()
{
	
	file_data.load_osm_file();

	//----------------------------------------------------------------------------
	QString path = "config/config.json";
	QFile file(path);
	double version = 0;

	if (!file.open(QFile::ReadOnly)) {
		INFO << "CALIBRATION: Cannot open configuration file " + path.toStdString();
		INFO << "CALIBRATION: Version file being set on loading of image data";
	}
	else {
		QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
		QJsonObject jsonObj = jsonDoc.object();

		if (jsonObj.contains("version")) {
			version = jsonObj.value("version").toDouble();
			INFO << "CALIBRATION: Overriding version of image file to " << version;
		}
		else {
			INFO << "CALIBRATION: Cannot find key 'version' in configuration file " + path.toStdString();
			INFO << "CALIBRATION: Version file being set on loading of image data";
		}
	}
	//----------------------------------------------------------------------------

	ImportFrames abp_frames = find_frames_in_osm();
	path_image = file_data.image_path;

	if (abp_frames.all_frames_found) {

		// get wavelength in microns
		double wavelength = QInputDialog::getDouble(this, "Input Wavelength", "Input wavelength in microns: ", 0, 0, 200, 5);

		double irradiance1 = calculate_black_body_radiance(wavelength * std::pow(10, -6), user_selection1.temperature_mean + 273.15);
		double irradiance2 = calculate_black_body_radiance(wavelength * std::pow(10, -6), user_selection2.temperature_mean + 273.15);

		std::vector<std::vector<uint16_t>> video_frames1 = file_data.load_image_file(abp_frames.start_frame1, abp_frames.stop_frame1, version);
		std::vector<std::vector<uint16_t>> video_frames2 = file_data.load_image_file(abp_frames.start_frame2, abp_frames.stop_frame2, version);

		arma::mat average_count1 = average_multiple_frames(video_frames1);
		arma::mat average_count2 = average_multiple_frames(video_frames2);

		arma::mat dx = average_count2 - average_count1;
		double dy = irradiance2 - irradiance1;

		arma::mat m = dy / dx;
		arma::mat b = irradiance1 - m % average_count1;

		// reshape arrays to image size
		int x_pixels = file_data.abir_data.ir_data[0].header.image_x_size;
		int y_pixels = file_data.abir_data.ir_data[0].header.image_y_size;

		m.reshape(x_pixels, y_pixels);
		m = m.t();

		b.reshape(x_pixels, y_pixels);
		b = b.t();

		model.setup_model(m, b);
		model.set_calibration_details(path_nuc, path_image, user_selection1, user_selection2);

		done(QDialog::Accepted);
	}
	else {

		INFO << "CALIBRATION: Cannot find the calibration frames within the selected abpimage file.";

		QMessageBox msgBox;
		msgBox.setWindowTitle(QString("Data Not Found"));
		QString box_text("Cannot find the calibration frames within the selected abpimage file.");
		msgBox.setText(box_text);

		msgBox.exec();

	}

}

ImportFrames CalibrationDialog::find_frames_in_osm() {

	
	// initialize ImportFrames struct
	ImportFrames output;

	output.start_frame1 = -1;
	output.start_frame2 = -1;
	output.stop_frame1 = -1;
	output.stop_frame2 = -1;
	output.all_frames_found = false;

	double frame_time;
	
	int num_messages = file_data.osm_data.num_messages;
	for (int i = 0; i < num_messages; i++)
	{
		
		frame_time = file_data.osm_data.data[i].data.frametime;

		if (frame_time >= user_selection1.start_time && frame_time <= user_selection1.stop_time)
		{

			output.stop_frame1 = i;
			if (output.start_frame1 < 0)
				output.start_frame1 = i;

		}

		if (frame_time >= user_selection2.start_time && frame_time <= user_selection2.stop_time)
		{

			output.stop_frame2 = i;
			if (output.start_frame2 < 0)
				output.start_frame2 = i;

		}

		if (frame_time > user_selection1.stop_time && frame_time > user_selection2.stop_time)
			break;
	}

	if (output.start_frame1 >= 0 && output.start_frame2 >= 0)
	{
		output.all_frames_found = true;
	}

	return output;
}

void CalibrationDialog::close_window()
{
	done(QDialog::Rejected);
}

void CalibrationDialog::closeEvent(QCloseEvent* event)
{
	close_window();
}

void CalibrationDialog::get_plotting_data(ABPNUC_Data &nuc_data)
{

	// enable selection options
	radio_temperature1->setEnabled(true);
	radio_temperature2->setEnabled(true);

	temperature.clear();

	for (int i = 0; i < nuc_data.number_of_frames; i++) {

		QPointF temp_pt1(i + 1, nuc_data.data[i].tec_temperature_x100 / 100.0);
		temperature.push_back(temp_pt1);

		all_frame_times.push_back(nuc_data.data[i].frame_time);
	}

	create_temperature_plot(temperature);

}

void CalibrationDialog::draw_axes() {


	// set up axes
	chart_temperature->createDefaultAxes();
	QAbstractAxis* x_axis = chart_temperature->axes(Qt::Horizontal)[0];
	QAbstractAxis* y_axis = chart_temperature->axes(Qt::Vertical)[0];

	x_axis->setTitleText("Frame #");
	y_axis->setTitleText("Temperature (C)");

	y_axis->setMinorGridLineVisible(true);
	y_axis->setLabelsVisible(true);

	x_axis->setMinorGridLineVisible(true);
	x_axis->setLabelsVisible(true);

	//chart_temperature->setTitle(QString("Temperature vs Frames"));
	chart_temperature->setMargins(QMargins(0, 0, 0, 0));
	chart_temperature->setContentsMargins(0, 0, 0, 0);
}

void CalibrationDialog::create_temperature_plot(QList<QPointF> temperature) {


	chart_temperature->removeAllSeries();

	// ----------------------------------------------------------------------------------------------------------------
	QPen pen_temperature, pen_temperature1, pen_temperature2, pen_temperature3;

	// set pen settings for line series
	//pen_temperature.setColor(QString("black"));
	pen_temperature.setColor(colors.Get_Color(0));
	pen_temperature.setStyle(Qt::SolidLine);
	pen_temperature.setWidth(3);

	// ----------------------------------------------------------------------------------------------------------------
	// Temperature line
	QLineSeries* series1 = new QLineSeries();
	series1->setPen(pen_temperature);
	series1->append(temperature);

	// Add all series to chart
	chart_temperature->addSeries(series1);

	// set legend
	chart_temperature->legend()->setVisible(false);

	QObject::connect(chart_view_temperatures, &Clickable_QChartView::click_drag, this, &CalibrationDialog::point_selected);

	// ----------------------------------------------------------------------------------------------------------------

	draw_axes();
}

bool CalibrationDialog::check_path(QString path)
{

	QString info_msg("");

	QFileInfo check_file(path);
	bool file_isFile = check_file.isFile();
	bool file_exists = check_file.exists();

	return file_exists && file_isFile;
}

double CalibrationDialog::calculate_black_body_radiance(double wavelength, double temperature) {

	// wavelength should have units of meters
	// temperature should have units of K

	double speed_light = 299792458;  // m/s
	double planks_constant = 6.626068963 * std::pow(10, -34);  // m^2 * kg / s
	double boltzmann_constant = 1.3806504 * std::pow(10, -23);  // J / K

	double c1 = 2 * planks_constant * std::pow(speed_light, 2);  // W / m^3
	double c2 = planks_constant * speed_light / boltzmann_constant;  // K * m

	double radiance = c1 / (std::pow(wavelength, 5) * (std::exp(c2 / (wavelength * temperature)) - 1));

	// returns radiance in W/m^3
	return radiance;
}

arma::mat CalibrationDialog::average_multiple_frames(std::vector<std::vector<uint16_t>> &frames) {


	int num_pixels = frames[0].size();
	int num_frames = frames.size();

	arma::vec data = arma::zeros(num_pixels);

	for (int i = 0; i < num_frames; i++)
	{

		std::vector<double> converted_values(frames[i].begin(), frames[i].end());
		arma::vec original_frame(converted_values);

		data = data + original_frame;
	}

	// average all frames
	data = data / num_frames;

	return data;
}