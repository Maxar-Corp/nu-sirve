#include "calibration_data.h"

CalibrationData::CalibrationData()
{
	// set check variable to false to start
	calibration_available = false;

}

double CalibrationData::measure_irradiance(int ul_row, int ul_col, int lr_row, int lr_col)
{
	
	return 0.0;
}

// --------------------------------------------------------------------------------------------------------------------

CalibrationDialog::CalibrationDialog(QWidget* parent)
{

	// initiliaze chart parameters
	chart_temperature = new QChart();
	chart_view_temperatures = new Clickable_QChartView(chart_temperature);

	initialize_gui();

	user_selection1.valid_data = false;
	user_selection1.series = new QLineSeries();
	user_selection1.color = colors.Get_Color(1);

	user_selection2.valid_data = false;
	user_selection2.series = new QLineSeries();
	user_selection2.color = colors.Get_Color(3);

}

CalibrationDialog::~CalibrationDialog()
{

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
	QObject::connect(btn_get_nuc_file, &QPushButton::clicked, this, &CalibrationDialog::import_nuc_file);

	// ------------------------------------------------------------

	setLayout(mainLayout);
	setWindowTitle("Set Calibration Data");

	// resize width
	int width = this->width();
	double ar = frame_plot->aspect_ratio_width * 1.0 / frame_plot->aspect_ratio_height;
	int height = int(width / ar);
	frame_plot->setFixedHeight(height);

}

void CalibrationDialog::import_nuc_file()
{
	// -----------------------------------------------------------------------------
	// get image path

	QString image_path = QFileDialog::getOpenFileName(this, ("Open Calibration File"), "", ("NUC File(*.abpnuc)"));
	QByteArray ba = image_path.toLocal8Bit();
	char* file_path = ba.data();

	ABPNUC_Data nuc_data(file_path);
	// nuc_data.read_apbnuc_file();

	if (nuc_data.read_status == 0) {

		// Add error message on import

		return;
	}

	if (nuc_data.number_of_frames == 0) {
		
		// Add error message saying file data was not imported

		return;
	}

	// -----------------------------------------------------------------------------
	// retrieve relevant data from calibration data
	get_plotting_data(nuc_data);

	// -----------------------------------------------------------------------------
	// get file name to display
	int index_file_start, index_file_end;
	index_file_start = image_path.lastIndexOf("/");
	index_file_end = image_path.lastIndexOf(".");
	
	QString filename = QString("File: ");
	filename.append(image_path.mid(index_file_start + 1, index_file_end - index_file_start - 1));

	lbl_nuc_filename->setText(filename);


	return;
}


void CalibrationDialog::point_selected(double x0, double x1) {

	if (radio_temperature1->isChecked())
	{
		show_user_selection(user_selection1, x0, x1);

		if (user_selection1.valid_data) {
			radio_temperature1->setText(QString("Temperature #1: %1C, +/- %2 [Over %3 frames]").arg(user_selection1.temperature_mean, 0, 'f', 2).arg(user_selection1.temperature_std, 0, 'f', 3).arg(user_selection1.num_frames));
		}
		else {
			radio_temperature1->setText(QString("Temperature #1: No data available"));
		}

	}
	else
	{
		show_user_selection(user_selection2, x0, x1);
		if (user_selection2.valid_data)
		{
			radio_temperature2->setText(QString("Temperature #2: %1C, +/- %2 [Over %3 frames]").arg(user_selection2.temperature_mean, 0, 'f', 2).arg(user_selection2.temperature_std, 0, 'f', 3).arg(user_selection2.num_frames));
		}
		else {
			radio_temperature2->setText(QString("Temperature #2: No data available"));
		}

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
	
	
	if (user_selection.series->count() > 0) {
		chart_temperature->removeSeries(user_selection.series);
		user_selection.series->clear();
	}

	int num_points = temperature.length();
	int initial_frame = 0;

	for (int i = 0; i < num_points; i++)
	{
		if (temperature[i].x() >= std::floor(x0) && temperature[i].x() < std::ceil(x1)) {
			user_selection.series->append(temperature[i].x(), temperature[i].y());

			vector_temperature.push_back(temperature[i].y());

			if (vector_temperature.size() == 1) {
				initial_frame = temperature[i].x();
			}

		}
	}

	arma::vec arma_temperatures(vector_temperature);

	user_selection.temperature_mean = arma::mean(arma_temperatures);
	user_selection.num_frames = arma_temperatures.size();
	user_selection.temperature_std = arma::stddev(arma_temperatures);
	user_selection.initial_frame = initial_frame;
	

	if (user_selection.series->count() > 0) {

		user_selection.valid_data = true;

		QPen pen;
		pen.setColor(user_selection.color);
		pen.setStyle(Qt::SolidLine);
		pen.setWidth(3);

		user_selection.series->setPen(pen);

		// Add all series to chart
		chart_temperature->addSeries(user_selection.series);
		draw_axes();
	}
	else {
		user_selection.valid_data = false;
	}


}

void CalibrationDialog::ok()
{
	done(QDialog::Accepted);
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

		QPointF temp_pt1(nuc_data.data[i].frame_number, nuc_data.data[i].tec_temperature_x100 / 100.0);
		temperature.push_back(temp_pt1);
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