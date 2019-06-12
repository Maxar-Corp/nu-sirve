#include "plot_engineering_data.h"


Engineering_Plots::Engineering_Plots(QWidget *parent) : QtPlotting(parent)
{
		
	x_axis_units = frames;
	
}

Engineering_Plots::~Engineering_Plots()
{

}

void Engineering_Plots::plot_azimuth() {
	// ---------------------------------------------------------------------------------------------------------------
	// Clear chart
	chart->removeAllSeries();
	colors.reset_colors();
	// ---------------------------------------------------------------------------------------------------------------

	int number_pts = engineering_data.size();
	
	std::vector<double>az;
	std::vector<double>x;

	for (int i = 0; i < number_pts; i++)
	{
		az.push_back(engineering_data[i].azimuth_sensor);
	}

	get_xaxis_value(x);
	y_title = QString("Azimuth (deg)");

	QColor base_color(colors.GetCurrentColor());
	QLineSeries *series = new QLineSeries();
	series->setColor(base_color);
	add_series(series, x, az);
	
	std::vector<double>min_max_x, min_max_y;
	min_max_x = find_min_max(x);
	min_max_y = find_min_max(az);

	full_plot_xmin = min_max_x[0];
	full_plot_xmax = find_max_for_axis(min_max_x);

	sub_plot_xmin = x[index_sub_plot_xmin];
	sub_plot_xmax = x[index_sub_plot_xmax];
	
	title = QString("");

	if (plot_all_data)
		chart_options(min_max_x[0], full_plot_xmax, 0, 360, x_title, y_title, title);
	else
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, 360, x_title, y_title, title);
}

void Engineering_Plots::plot_elevation() {
	
	// ---------------------------------------------------------------------------------------------------------------
	// Clear chart
	chart->removeAllSeries();
	colors.reset_colors();
	// ---------------------------------------------------------------------------------------------------------------

	int number_pts = engineering_data.size();

	std::vector<double>el;
	std::vector<double>x;

	for (int i = 0; i < number_pts; i++)
	{
		el.push_back(engineering_data[i].elevation_sensor);
	}

	get_xaxis_value(x);
	y_title = QString("Elevation (deg)");

	QColor base_color(colors.GetCurrentColor());
	QLineSeries *series = new QLineSeries();
	series->setColor(base_color);
	add_series(series, x, el);

	std::vector<double>min_max_x, min_max_y;
	min_max_x = find_min_max(x);
	min_max_y = find_min_max(el);

	full_plot_xmin = min_max_x[0];
	full_plot_xmax = find_max_for_axis(min_max_x);

	sub_plot_xmin = x[index_sub_plot_xmin];
	sub_plot_xmax = x[index_sub_plot_xmax];

	title = QString("");

	if (plot_all_data)
		chart_options(min_max_x[0], full_plot_xmax, 0, 90, x_title, y_title, title);
	else
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, 90, x_title, y_title, title);
}

void Engineering_Plots::plot_irradiance(int number_tracks)
{
	// ---------------------------------------------------------------------------------------------------------------
	// Clear chart
	chart->removeAllSeries();
	colors.reset_colors();
	// ---------------------------------------------------------------------------------------------------------------

	int number_pts = engineering_data.size();

	std::vector<double> x_points, y_points;

	for (int i = 0; i < number_tracks; i++)
	{
		std::vector<double>x, irradiance;
		for (int j = 0; j < number_pts; j++)
		{
			int num_tracks_present = engineering_data[j].ir_data.size();
			if (num_tracks_present > 0 && i < num_tracks_present)
			{
				
				double value = engineering_data[j].ir_data[i].irradiance;
				irradiance.push_back(value);
				y_points.push_back(value);

				if (x_axis_units == frames) {
					x.push_back(j + 1);
					x_points.push_back(j + 1);
					x_title = QString("Frame #");
				}
				else if(x_axis_units == seconds_past_midnight) {
					x.push_back(past_midnight[j]);
					x_points.push_back(past_midnight[j]);
					x_title = QString("Seconds Past Midnight");
				}
			}
		}
		
		// add to series to chart
		QLineSeries *series = new QLineSeries();
		QColor base_color(colors.GetCurrentColor());
		series->setColor(base_color);
		add_series(series, x, irradiance, true);

		// get next color for series
		colors.GetNextColor();
	}

	std::vector<double>min_max_x, min_max_y;
	min_max_x = find_min_max(x_points);
	min_max_y = find_min_max(y_points);

	full_plot_xmin = min_max_x[0];
	full_plot_xmax = find_max_for_axis(min_max_x);

	switch (x_axis_units)
	{
	case frames:
		sub_plot_xmin = frame_numbers[index_sub_plot_xmin];
		sub_plot_xmax = frame_numbers[index_sub_plot_xmax];
		break;
	case seconds_past_midnight:
		sub_plot_xmin = past_midnight[index_sub_plot_xmin];
		sub_plot_xmax = past_midnight[index_sub_plot_xmax];
		break;
	case seconds_from_epoch:
		sub_plot_xmin = past_epoch[index_sub_plot_xmin];
		sub_plot_xmax = past_epoch[index_sub_plot_xmax];
		break;
	default:
		break;
	}

	y_title = QString("Irradiance Counts");
	title = QString("");

	if (plot_all_data)
		chart_options(min_max_x[0], full_plot_xmax, 0, find_max_for_axis(min_max_y), x_title, y_title, title);
	else
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, find_max_for_axis(min_max_y), x_title, y_title, title);
	
}

std::vector<double> Engineering_Plots::find_min_max(std::vector<double> data)
{
	
	arma::vec input_data(data);
	double min_value = arma::min(input_data);
	double max_value = arma::max(input_data);

	return {min_value, max_value};
}

void Engineering_Plots::get_xaxis_value(std::vector<double>& values)
{

	switch (x_axis_units)
	{
	case frames:
		values = frame_numbers;
		x_title = "Frame #";
		break;
	case seconds_past_midnight:
		values = past_midnight;
		x_title = "Seconds Past Midnight";
		break;
	case seconds_from_epoch:
		//TODO switch to past_epoch
		values = past_midnight;
		x_title = "Seconds Past Epoch";
		break;
	default:
		break;
	}
}

void Engineering_Plots::plot_az_el_boresite_data(Plotting_Data data, bool plot_azimuth, bool plot_frames)
{

	// ---------------------------------------------------------------------------------------------------------------
	// Clear chart
	chart->removeAllSeries();
	colors.reset_colors();
	// ---------------------------------------------------------------------------------------------------------------

	QLineSeries *series = new QLineSeries();
	QString x_title, y_title, title;
	double min_x, max_x, min_y, max_y;

	QColor base_color(colors.GetCurrentColor());
	series->setColor(base_color);

	if (plot_frames) {
		x_title = QString("Frames");

		if (plot_azimuth) {
			add_series(series, data.frame_number, data.azimuth);
			y_title = QString("Azimuth (deg)");
			title = QString("Azimuth Over Frames");
			
			arma::vec azimuth_data(data.azimuth);
			min_y = arma::min(azimuth_data);
			max_y = arma::max(azimuth_data);
		}
		else{
			add_series(series, data.frame_number, data.elevation);
			y_title = QString("Elevation (deg)");
			title = QString("Elevation Over Frames");
			
			arma::vec elevation_data(data.elevation);
			min_y = arma::min(elevation_data);
			max_y = arma::max(elevation_data);
		}

		min_x = data.min_frame;
		max_x = data.max_frame;
	}
	else
	{
		arma::vec jdates(data.frame_time);
		arma::vec time = (jdates - jdates(0)) * 86400.;
		std::vector<double> seconds = arma::conv_to<std::vector<double>>::from(time);
		
		if (plot_azimuth) {
			add_series(series, seconds, data.azimuth);
			y_title = QString("Azimuth (deg)");
			title = QString("Azimuth Over Time");

			arma::vec azimuth_data(data.azimuth);
			min_y = arma::min(azimuth_data);
			max_y = arma::max(azimuth_data);
		}
		else {
			add_series(series, seconds, data.elevation);
			y_title = QString("Elevation (deg)");
			title = QString("Elevation Over Time");

			arma::vec elevation_data(data.elevation);
			min_y = arma::min(elevation_data);
			max_y = arma::max(elevation_data);
		}

		min_x = seconds[0];
		max_x = seconds.back();
	}

	chart_options(min_x, max_x, min_y, max_y, x_title, y_title, title);
}

void Engineering_Plots::toggle_subplot()
{
	double min_value, max_value;

	if (plot_all_data)
	{
		set_xaxis_limits(full_plot_xmin, full_plot_xmax, x_title, y_title, title);
	}
	else
	{
		set_xaxis_limits(sub_plot_xmin, sub_plot_xmax, x_title, y_title, title);
	}
}

void Engineering_Plots::plot_irradiance_data(std::vector<Track_Irradiance> data) {

	// Clear chart
	chart->removeAllSeries();

	// ------------------------------------------------------------------------------------------
	// Define series data

	unsigned int num_plot_tracks = data.size();
	unsigned int min_frame, max_frame;
	double min_irradiance, max_irradiance;

	for (unsigned int i = 0; i < num_plot_tracks; ++i) {

		// Define series object
		std::string label = "Track " + std::to_string(data[i].track_id);
		const char *series_name = label.c_str();

		QLineSeries *series = new QLineSeries();
		series->setName(QString(series_name));

		QColor base_color(colors.GetCurrentColor());
		series->setColor(base_color);

		// add to series to chart
		add_series(series, data[i].frame_number, data[i].irradiance, true);

		// check data ranges
		if (i == 0)
		{
			min_frame = data[i].minimum_frame_number;
			max_frame = data[i].maximum_frame_number;
			min_irradiance = data[i].minimum_irradiance;
			max_irradiance = data[i].maximum_irradiance;
		}
		else
		{
			if (data[i].minimum_frame_number < min_frame)
			{
				min_frame = data[i].minimum_frame_number;
			}
			if (data[i].maximum_frame_number > max_frame)
			{
				max_frame = data[i].maximum_frame_number;
			}
			if (data[i].minimum_irradiance < min_irradiance)
			{
				min_irradiance = data[i].minimum_irradiance;
			}
			if (data[i].maximum_irradiance > max_irradiance)
			{
				max_irradiance = data[i].maximum_irradiance;
			}
		}

		// get next color for series
		colors.GetNextColor();
	}

	// ------------------------------------------------------------------------------------------
	// Define chart properties
	chart->createDefaultAxes();
	QAbstractAxis *x_axis = chart->axes(Qt::Horizontal)[0];
	QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];

	QString x_label_title("Frame #");
	QString y_label_title("Radiance");

	int frame_range = max_frame - min_frame;
	double irradiance_range = max_irradiance;

	double tick_spacing_frames = find_tick_spacing(frame_range, 3, 10);
	double max_limit_frame = std::floor(max_frame / tick_spacing_frames) * tick_spacing_frames + 0.5 * tick_spacing_frames;

	double tick_spacing_radiance = find_tick_spacing(irradiance_range, 3, 10);
	double max_limit_radiance = std::floor(max_irradiance / tick_spacing_radiance) * tick_spacing_radiance + 0.5 * tick_spacing_radiance;

	x_axis->setMin(min_frame);
	x_axis->setMax(max_limit_frame);
	x_axis->setTitleText(x_label_title);

	y_axis->setMin(0);
	y_axis->setMax(max_limit_radiance);
	y_axis->setLabelsVisible(false);

	// Set chart title
	chart->setTitle(QString("Radiance"));

	// Legend properties
	chart->legend()->setVisible(false);
	//chart->legend()->setAlignment(Qt::AlignRight);
	//chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
}

// ---------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------
// Generic plotting functions

NewChartView::NewChartView(QChart* chart)
	:QChartView(chart)
{
	newchart = chart;
	
	setMouseTracking(true);
	setInteractive(true);
	setRubberBand(RectangleRubberBand);
}

void NewChartView::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() == Qt::RightButton)
	{
		newchart->zoomReset();
		return; //event doesn't go further
	}
	QChartView::mouseReleaseEvent(e);//any other event
}

QtPlotting::QtPlotting(QWidget *parent)
{
	chart = new QChart();

	chart_view = new NewChartView(chart);
	
}

QtPlotting::~QtPlotting()
{
	delete chart;
	delete chart_view;

}

void QtPlotting::add_series(QXYSeries *series, std::vector<double> x, std::vector<double> y, bool broken_data)
{

	uint num_data_pts = x.size();
	int num_breaks = 0;

	double base_x_distance = 1;
	if (num_data_pts > 1) {
		base_x_distance = (x[1] - x[0]) * 1.01;
	}

	for (uint i = 0; i < num_data_pts; i++) {
		if (i == 0) { //If first pt in series then continue...
			series->append(x[i], y[i]);

		}
		else if (x[i] - x[i - 1] > base_x_distance & broken_data) { //if current point is greater than 1 frame away then start new series...
			chart->addSeries(series);

			if (num_breaks > 0)
				remove_series_legend();
			num_breaks++;

			series = new QLineSeries();
			series->setColor(colors.GetCurrentColor());
			series->append(x[i], y[i]);
		}
		else { //if continuation of current series...
			series->append(x[i], y[i]);
		}
	}

	chart->addSeries(series);
	if (num_breaks > 0 & broken_data)
		remove_series_legend();
}

double QtPlotting::find_tick_spacing(double data_range, int min_number_ticks, int max_number_ticks) {

	double min_spacing = data_range / max_number_ticks;
	double magnitude_spacing = std::pow(10, std::floor(std::log10(min_spacing)));
	double residual = min_spacing / magnitude_spacing;

	double tick_spacing;
	if (residual > 5) {
		tick_spacing = magnitude_spacing * 10;
	}
	else if (residual > 2) {
		tick_spacing = magnitude_spacing * 5;
	}
	else if (residual > 1) {
		tick_spacing = magnitude_spacing * 2;
	}
	else {
		tick_spacing = magnitude_spacing * 1;
	}

	double num_ticks = std::floor(data_range / tick_spacing);
	if (num_ticks < min_number_ticks) {
		tick_spacing = tick_spacing / 2;
	}

	return tick_spacing;
}

void QtPlotting::remove_series_legend() {
	int num_legend_entries = chart->legend()->markers().size();
	if (num_legend_entries > 1) {
		QLegendMarker *last_entry = chart->legend()->markers().last();
		last_entry->setVisible(false);
	}
}

void QtPlotting::chart_options(double min_x, double max_x, double min_y, double max_y, QString x_label_title, QString y_label_title, QString title) {

	// ------------------------------------------------------------------------------------------
	// Define chart properties
	chart->createDefaultAxes();
	QAbstractAxis *x_axis = chart->axes(Qt::Horizontal)[0];
	QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];

	x_axis->setTitleText(x_label_title);
	set_axis_limits(x_axis, min_x, max_x);

	y_axis->setTitleText(y_label_title);
	set_axis_limits(y_axis, min_y, max_y);
	//y_axis->setLabelsVisible(true);
	
	// Set chart title
	chart->setTitle(title);
	chart->setMargins(QMargins(0, 0, 0, 0));

	// Legend properties
	chart->legend()->setVisible(false);
	//chart->legend()->setAlignment(Qt::AlignRight);
	//chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
}

void QtPlotting::set_axis_limits(QAbstractAxis *axis, double min_x, double max_x) {

	axis->setMin(min_x);
	axis->setMax(max_x);
}

void QtPlotting::set_xaxis_limits(double min_x, double max_x, QString x_label_title, QString y_label_title, QString title) {

	chart->createDefaultAxes();
	QAbstractAxis *x_axis = chart->axes(Qt::Horizontal)[0];
	QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];

	x_axis->setTitleText(x_label_title);
	set_axis_limits(x_axis, min_x, max_x);

	y_axis->setTitleText(y_label_title);
	
	// Set chart title
	chart->setTitle(title);
	chart->setMargins(QMargins(0, 0, 0, 0));
}

void QtPlotting::set_yaxis_limits(double min_y, double max_y) {

	chart->createDefaultAxes();
	QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];

	y_axis->setMin(min_y);
	y_axis->setMax(max_y);
}

double QtPlotting::find_max_for_axis(std::vector<double>min_max_values) {

	double range_value = min_max_values[1] - min_max_values[0];

	double tick_spacing_x = find_tick_spacing(range_value, 3, 10);
	double max_limit_x = std::floor(min_max_values[1] / tick_spacing_x) * tick_spacing_x + 0.5 * tick_spacing_x;

	return max_limit_x;
}
