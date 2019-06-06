#include "plot_engineering_data.h"


Engineering_Plots::Engineering_Plots(QWidget *parent)
{
	chart = new QChart();

	chart_view = new QChartView(chart);
	chart_view->setRubberBand(QChartView::RectangleRubberBand);
	plot_all_data = true;
}

Engineering_Plots::~Engineering_Plots()
{
	delete chart;
	delete chart_view;

}

void Engineering_Plots::plot_az_el_boresite_data(Az_El_Data data, bool plot_azimuth, bool plot_frames)
{
	// Clear chart
	chart->removeAllSeries();

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
			min_y = data.min_az;
			max_y = data.max_az;
		}
		else{
			add_series(series, data.frame_number, data.elevation);
			y_title = QString("Elevation (deg)");
			title = QString("Elevation Over Frames");
			min_y = data.min_el;
			max_y = data.max_el;
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
			min_y = data.min_az;
			max_y = data.max_az;
		}
		else {
			add_series(series, seconds, data.elevation);
			y_title = QString("Elevation (deg)");
			title = QString("Elevation Over Time");
			min_y = data.min_el;
			max_y = data.max_el;
		}

		min_x = seconds[0];
		max_x = seconds.back();
	}

	chart_options(min_x, max_x, min_y, max_y, x_title, y_title, title);
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
		add_series(series, data[i].frame_number, data[i].irradiance);

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
// Generic plotting functions

void Engineering_Plots::add_series(QXYSeries *series, std::vector<double> x, std::vector<double> y, bool broken_data)
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

double Engineering_Plots::find_tick_spacing(double data_range, int min_number_ticks, int max_number_ticks) {

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

void Engineering_Plots::remove_series_legend() {
	int num_legend_entries = chart->legend()->markers().size();
	if (num_legend_entries > 1) {
		QLegendMarker *last_entry = chart->legend()->markers().last();
		last_entry->setVisible(false);
	}
}

void Engineering_Plots::chart_options(double min_x, double max_x, double min_y, double max_y, QString x_label_title, QString y_label_title, QString title) {

	// ------------------------------------------------------------------------------------------
	// Define chart properties
	chart->createDefaultAxes();
	QAbstractAxis *x_axis = chart->axes(Qt::Horizontal)[0];
	QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];

	double x_range = max_x - min_x;
	double y_range = max_y - min_y;

	double tick_spacing_x = find_tick_spacing(x_range, 3, 10);
	double max_limit_x = std::floor(max_x / tick_spacing_x) * tick_spacing_x + 0.5 * tick_spacing_x;

	double tick_spacing_y = find_tick_spacing(y_range, 3, 10);
	double max_limit_y = std::floor(max_y / tick_spacing_y) * tick_spacing_y + 0.5 * tick_spacing_y;

	x_axis->setMin(min_x);
	x_axis->setMax(max_limit_x);
	x_axis->setTitleText(x_label_title);

	y_axis->setMin(min_y);
	y_axis->setMax(max_limit_y);
	y_axis->setLabelsVisible(false);

	// Set chart title
	chart->setTitle(title);

	// Legend properties
	chart->legend()->setVisible(false);
	//chart->legend()->setAlignment(Qt::AlignRight);
	//chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

}