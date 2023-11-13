#include "plot_engineering_data.h"


Engineering_Plots::Engineering_Plots(std::vector<Frame> const &osm_frames) : QtPlotting()
{
	num_frames = static_cast<unsigned int>(osm_frames.size());

	for (size_t i = 0; i < num_frames; i++) {
		/* Scaling each val by 1e6 to convert to microradians. */
		sensor_i_fov_x.push_back(osm_frames[i].data.i_fov_x * 1e6);
		sensor_i_fov_y.push_back(osm_frames[i].data.i_fov_y * 1e6);

		boresight_az.push_back(osm_frames[i].data.az_el_boresight[0]);
		boresight_el.push_back(osm_frames[i].data.az_el_boresight[1]);
	}

	x_axis_units = frames;
	plot_all_data = true;
	plot_primary_only = false;
	plot_current_marker = false;
	yaxis_is_log = false;
	yaxis_is_scientific = false;
	set_plot_title("EDIT CLASSIFICATION");
	current_chart_id = 0;

	index_sub_plot_xmin = 0;
	index_sub_plot_xmax = num_frames - 1;
}

Engineering_Plots::~Engineering_Plots()
{	

}

void Engineering_Plots::set_yaxis_chart_id(int yaxis_chart_id)
{
	current_chart_id = yaxis_chart_id;
}

void Engineering_Plots::plot()
{
	// ---------------------------------------------------------------------------------------------------------------
	// Clear chart
	chart->removeAllSeries();
	colors.reset_colors();
	start_new_chart();
	create_current_marker();

	size_t plot_number_tracks = number_of_tracks;
	if (plot_primary_only && plot_number_tracks > 0)
		plot_number_tracks = 1;

	switch (current_chart_id) {
		case 0:
			plot_irradiance(plot_number_tracks);
			break;
		case 1:
			plot_azimuth(plot_number_tracks);
			break;
		case 2:
			plot_elevation(plot_number_tracks);
			break;
		case 3:
			plot_fov_x();
			break;
		case 4:
			plot_fov_y();
			break;
		case 5:
			plot_boresight_az();
			break;
		case 6:
			plot_boresight_el();
		default:
			break;
	}
}

void Engineering_Plots::plot_boresight_az()
{
	establish_plot_limits();
	y_title = QString("Boresight Azimuth");
	QLineSeries* series = new QLineSeries();
	QColor base_color(colors.GetCurrentColor());
	series->setColor(base_color);

	if (plot_all_data) {
		std::vector<double> y_values = boresight_az;

		add_series(series, get_x_axis_values(0, num_frames - 1), y_values, true);
		chart_options(full_plot_xmin, full_plot_xmax, 0, 360, x_title, y_title);
	}
	else {
		std::vector<double> y_values(boresight_az.begin() + index_sub_plot_xmin, boresight_az.begin() + index_sub_plot_xmax + 1);

		add_series(series, get_x_axis_values(index_sub_plot_xmin, index_sub_plot_xmax), y_values, true);
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, 360, x_title, y_title);
	}

	draw_title();
}

void Engineering_Plots::plot_boresight_el()
{
	establish_plot_limits();
	y_title = QString("Boresight Elevation");
	QLineSeries* series = new QLineSeries();
	QColor base_color(colors.GetCurrentColor());
	series->setColor(base_color);

	if (plot_all_data) {
		std::vector<double> y_values = boresight_el;

		add_series(series, get_x_axis_values(0, num_frames - 1), y_values, true);
		chart_options(full_plot_xmin, full_plot_xmax, 0, 90, x_title, y_title);
	}
	else {
		std::vector<double> y_values(boresight_el.begin() + index_sub_plot_xmin, boresight_el.begin() + index_sub_plot_xmax + 1);

		add_series(series, get_x_axis_values(index_sub_plot_xmin, index_sub_plot_xmax), y_values, true);
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, 90, x_title, y_title);
	}

	draw_title();
}

void Engineering_Plots::plot_fov_x()
{
	establish_plot_limits();
	y_title = QString("Sensor IFOV (microns)");
	QLineSeries* series = new QLineSeries();
	QColor base_color(colors.GetCurrentColor());
	series->setColor(base_color);

	if (plot_all_data) {
		std::vector<double> y_values = sensor_i_fov_x;
		
		add_series(series, get_x_axis_values(0, num_frames - 1), y_values, true);
		chart_options(full_plot_xmin, full_plot_xmax, 0, 750, x_title, y_title);
	}
	else{
		std::vector<double> y_values(sensor_i_fov_x.begin() + index_sub_plot_xmin, sensor_i_fov_x.begin() + index_sub_plot_xmax + 1);

		add_series(series, get_x_axis_values(index_sub_plot_xmin, index_sub_plot_xmax), y_values, true);
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, 750, x_title, y_title);
	}

	draw_title();
}

void Engineering_Plots::plot_fov_y()
{
	establish_plot_limits();
	y_title = QString("Sensor IFOV (microns)");
	QLineSeries* series = new QLineSeries();
	QColor base_color(colors.GetCurrentColor());
	series->setColor(base_color);

	if (plot_all_data) {
		std::vector<double> y_values = sensor_i_fov_y;

		add_series(series, get_x_axis_values(0, num_frames - 1), y_values, true);
		chart_options(full_plot_xmin, full_plot_xmax, 0, 750, x_title, y_title);
	}
	else{
		std::vector<double> y_values(sensor_i_fov_y.begin() + index_sub_plot_xmin, sensor_i_fov_y.begin() + index_sub_plot_xmax + 1);

		add_series(series, get_x_axis_values(index_sub_plot_xmin, index_sub_plot_xmax), y_values, true);
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, 750, x_title, y_title);
	}

	draw_title();
}

void Engineering_Plots::plot_azimuth(size_t plot_number_tracks)
{
	for (size_t i = 0; i < plot_number_tracks; i++)
	{
		QLineSeries* series = new QLineSeries();
		QColor base_color(colors.GetCurrentColor());
		series->setColor(base_color);

		std::vector<double> x_values = get_individual_x_track(i);
		std::vector<double> y_values = get_individual_y_track_azimuth(i);

		add_series(series, x_values, y_values, true);

		// get next color for series
		colors.GetNextColor();
	}

	establish_plot_limits();

	y_title = QString("Azimuth (deg)");

	if (plot_all_data) {
		chart_options(full_plot_xmin, full_plot_xmax, 0, 360, x_title, y_title);
	}
	else{
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, 360, x_title, y_title);
	}


	draw_title();
	
}

void Engineering_Plots::plot_elevation(size_t plot_number_tracks)
{
	for (size_t i = 0; i < plot_number_tracks; i++)
	{
		QLineSeries *series = new QLineSeries();
		QColor base_color(colors.GetCurrentColor());
		series->setColor(base_color);

		std::vector<double> x_values = get_individual_x_track(i);
		std::vector<double> y_values = get_individual_y_track_elevation(i);

		add_series(series, x_values, y_values, true);

		// get next color for series
		colors.GetNextColor();
	}

	establish_plot_limits();

	y_title = QString("Elevation (deg)");

	if (plot_all_data)
		chart_options(full_plot_xmin, full_plot_xmax, 0, 90, x_title, y_title);
	else
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, 90, x_title, y_title);


	draw_title();
}

void Engineering_Plots::plot_irradiance(size_t plot_number_tracks)
{
	std::vector<double> y_points;

	for (size_t i = 0; i < plot_number_tracks; i++)
	{
		QLineSeries *series = new QLineSeries();
		QColor base_color(colors.GetCurrentColor());
		series->setColor(base_color);

		std::vector<double> x_values = get_individual_x_track(i);
		std::vector<double> y_values = get_individual_y_track_irradiance(i);

		add_series(series, x_values, y_values, true);

		// get next color for series
		colors.GetNextColor();

		y_points.insert(y_points.end(), y_values.begin(), y_values.end());
	}

	establish_plot_limits();	

	y_title = QString("Irradiance Counts");

	if (plot_all_data)
		chart_options(full_plot_xmin, full_plot_xmax, 0, find_max_for_axis(y_points), x_title, y_title);
	else
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, find_max_for_axis(y_points), x_title, y_title);


	draw_title();
}

void Engineering_Plots::set_plotting_track_frames(std::vector<PlottingTrackFrame> frames, int num_unique)
{
	track_frames = frames;
	number_of_tracks = num_unique;
}

std::vector<double> Engineering_Plots::get_individual_x_track(size_t i)
{
	std::vector<double> x_values;

	for (int track_frame_index = 0; track_frame_index < track_frames.size(); track_frame_index += 1)
	{
		if (i < track_frames[track_frame_index].details.size()) {
			x_values.push_back(get_single_x_axis_value(track_frame_index));
		}
	}

	return x_values;
}

std::vector<double> Engineering_Plots::get_individual_y_track_irradiance(size_t i)
{
	std::vector<double> y_values;
	for (int track_frame_index = 0; track_frame_index < track_frames.size(); track_frame_index += 1)
	{
		if (i < track_frames[track_frame_index].details.size())
		{
			y_values.push_back(track_frames[track_frame_index].details[i].irradiance);
		}
	}

	return y_values;
}

std::vector<double> Engineering_Plots::get_individual_y_track_azimuth(size_t i)
{
	std::vector<double> y_values;
	for (int track_frame_index = 0; track_frame_index < track_frames.size(); track_frame_index += 1)
	{
		if (i < track_frames[track_frame_index].details.size())
		{
			y_values.push_back(track_frames[track_frame_index].details[i].azimuth);
		}
	}

	return y_values;
}

std::vector<double> Engineering_Plots::get_individual_y_track_elevation(size_t i)
{
	std::vector<double> y_values;
	for (int track_frame_index = 0; track_frame_index < track_frames.size(); track_frame_index += 1)
	{
		if (i < track_frames[track_frame_index].details.size())
		{
			y_values.push_back(track_frames[track_frame_index].details[i].elevation);
		}
	}

	return y_values;
}

void Engineering_Plots::establish_plot_limits() {

	sub_plot_xmin = get_single_x_axis_value(index_sub_plot_xmin);
	sub_plot_xmax = get_single_x_axis_value(index_sub_plot_xmax);

	full_plot_xmin = get_single_x_axis_value(0);
	full_plot_xmax = get_max_x_axis_value();
}

void Engineering_Plots::set_xaxis_units(x_plot_variables unit_choice)
{
	x_axis_units = unit_choice;
	switch (x_axis_units)
	{
		case frames:
			x_title = "Frame #";
			break;
		case seconds_past_midnight:
			x_title = "Seconds Past Midnight";
			break;
		case seconds_from_epoch:
			x_title = "Seconds Past Epoch";
			break;
		default:
			break;
	}
}

std::vector<double> Engineering_Plots::get_x_axis_values(unsigned int start_idx, unsigned int end_idx)
{
	switch (x_axis_units)
	{
		case frames:
		{
			std::vector<double> x_values(end_idx - start_idx + 1);
			std::iota(std::begin(x_values), std::end(x_values), start_idx + 1);
			return x_values;
		}
		case seconds_past_midnight:
			return std::vector<double>(past_midnight.begin() + start_idx, past_midnight.begin() + end_idx + 1);
		case seconds_from_epoch:
			return std::vector<double>(past_epoch.begin() + start_idx, past_epoch.begin() + end_idx + 1);
		default:
			return std::vector<double>();
	}
}

double Engineering_Plots::get_single_x_axis_value(int x_index)
{
	switch (x_axis_units)
	{
		case frames:
			return x_index + 1;
		case seconds_past_midnight:
			return past_midnight[x_index];
		case seconds_from_epoch:
			return past_epoch[x_index];
		default:
			return 0;
	}
}

double Engineering_Plots::get_max_x_axis_value()
{
	switch (x_axis_units)
	{
		case frames:
			return num_frames;
		case seconds_past_midnight:
			return past_midnight[past_midnight.size() - 1];
		case seconds_from_epoch:
			return past_epoch[past_epoch.size() - 1];
		default:
			return 0;
	}
}

void Engineering_Plots::create_current_marker()
{
	current_frame_marker = new QLineSeries();

	QPen pen;
	pen.setColor(colors.Get_Color(2));
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(3);

	current_frame_marker->setPen(pen);
	
	current_frame_marker->append(0, 0);
	current_frame_marker->append(0, 0);
	
	chart->addSeries(current_frame_marker);
}

void Engineering_Plots::toggle_yaxis_log(bool input)
{
	yaxis_is_log = input;
}

void Engineering_Plots::toggle_yaxis_scientific(bool input)
{
	yaxis_is_scientific = input;
}

void Engineering_Plots::plot_current_step(int counter)
{
	if (plot_current_marker)
	{
		double current_x = get_single_x_axis_value(index_sub_plot_xmin + counter);
		double min_y, max_y;

		if (yaxis_is_log) {
			min_y = axis_ylog->min() * 1.01;
			max_y = axis_ylog->max() * 0.99;
		}
		else {
			min_y = axis_y->min() * 1.01;
			max_y = axis_y->max() * 0.99;
		}

		current_frame_marker->replace(0, current_x, min_y);
		current_frame_marker->replace(1, current_x, max_y);
	}
}

void Engineering_Plots::set_plot_title(QString input_title)
{
	
	title = input_title;
	draw_title();

}

void Engineering_Plots::draw_title() 
{
	QColor brush_color("black");
	QBrush brush(brush_color);

	QFont font;
	font.setPointSize(10);
	font.setBold(true);

	chart->setTitleBrush(brush);
	chart->setTitleFont(font);

	chart->setTitle(title);
}

void Engineering_Plots::toggle_subplot()
{
	if (plot_all_data)
	{
		set_xaxis_limits(full_plot_xmin, full_plot_xmax, x_title, y_title, title);
	}
	else
	{
		set_xaxis_limits(sub_plot_xmin, sub_plot_xmax, x_title, y_title, title);
	}
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
		return;
	}
	
	QChartView::mouseReleaseEvent(e);
}



void NewChartView::apply_nice_numbers()
{
	QList<QAbstractAxis*> axes_list = newchart->axes();
	for each (QAbstractAxis * abstract_axis in axes_list)
	{
		QValueAxis* value_axis = qobject_cast<QValueAxis*>(abstract_axis);
		if (value_axis)
		{
			value_axis->applyNiceNumbers();
		}
	}
}


// ---------------------------------------------------------------------------------------------

QtPlotting::QtPlotting()
{
	chart = new QChart();
	chart_view = new NewChartView(chart);

	axis_x = new QValueAxis;
	axis_y = new QValueAxis;
	axis_ylog = new QLogValueAxis;
}

QtPlotting::~QtPlotting()
{
	delete chart;
	delete chart_view;
}

void QtPlotting::start_new_chart()
{
	delete axis_x;
	axis_x = new QValueAxis();
	axis_x->setTitleText("x");
	axis_x->setRange(0, 10);

	delete axis_y;
	axis_y = new QValueAxis();
	axis_y->setTitleText("y");
	axis_y->setRange(0, 10);

	delete axis_ylog;
	axis_ylog = new QLogValueAxis();
	axis_ylog->setTitleText("y");
	axis_ylog->setBase(10);
	axis_ylog->setRange(1, 10);
	

	if (yaxis_is_scientific) {
		axis_y->setLabelFormat("%.4e");
		axis_ylog->setLabelFormat("%.4e");
	}
	else {
		axis_y->setLabelFormat("%i");
		axis_ylog->setLabelFormat("%i");
	}

	chart->addAxis(axis_x, Qt::AlignBottom);

	if (yaxis_is_log)
	{
		chart->addAxis(axis_ylog, Qt::AlignLeft);
	}
	else {
		chart->addAxis(axis_y, Qt::AlignLeft);
	}
}

void QtPlotting::add_series(QXYSeries *series, std::vector<double> x, std::vector<double> y, bool broken_data)
{

	size_t num_data_pts = x.size();
	int num_breaks = 0;

	double base_x_distance = 1;
	if (num_data_pts > 1) {

		// distance to separate into different line series is the median difference of the x-value set

		arma::vec x_vector(x);
		arma::vec diff = arma::diff(x_vector);
		base_x_distance = 1.5; //arma::median(diff);
		
	}

	for (size_t i = 0; i < num_data_pts; i++) {
		if (i == 0) { //If first pt in series then continue...
			series->append(x[i], y[i]);

		}
		else if ((x[i] - x[i - 1] > base_x_distance) & broken_data) { //if current point is greater than 1 frame away then start new series...
			
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
	if ((num_breaks > 0) & broken_data)
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

void QtPlotting::chart_options(double min_x, double max_x, double min_y, double max_y, QString x_label_title, QString y_label_title) {

	// ------------------------------------------------------------------------------------------
	// Define chart properties
		
	axis_x->setRange(min_x, max_x);
	axis_x->setTitleText(x_label_title);

	if (yaxis_is_log) {
		if (min_y <= 0.001)
			min_y = 0.01;
		if (max_y <= 0.001)
			max_y = 0.01;
		axis_ylog->setRange(min_y, max_y);
		axis_ylog->setTitleText(y_label_title);
	}
	else {
		axis_y->setRange(min_y, max_y);
		axis_y->setTitleText(y_label_title);
	}

	int num_series = chart->series().size();

	bool check1, check2;
	QList<QAbstractSeries *> all_series = chart->series();
	
	for (int i = 0; i < num_series; i++)
	{
		check1 = all_series[i]->attachAxis(axis_x);

		if (yaxis_is_log) {
			check2 = all_series[i]->attachAxis(axis_ylog);
		}
		else {
			check2 = all_series[i]->attachAxis(axis_y);
		}
	}
	
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

	//chart->createDefaultAxes();
	//QAbstractAxis *x_axis = chart->axes(Qt::Horizontal)[0];
	//QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];

	//x_axis->setTitleText(x_label_title);
	//set_axis_limits(x_axis, min_x, max_x);
	chart->axisX()->setRange(min_x, max_x);

	//y_axis->setTitleText(y_label_title);
	
	// Set chart title
	//chart->setTitle(title);
	//chart->setMargins(QMargins(0, 0, 0, 0));
}

void QtPlotting::set_yaxis_limits(double min_y, double max_y) {

	//chart->createDefaultAxes();
	//QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];

	//y_axis->setMin(min_y);
	//y_axis->setMax(max_y);

	chart->axisY()->setRange(min_y, max_y);
}

double QtPlotting::find_max_for_axis(std::vector<double> data) {
	double min, max;

	arma::vec input_data(data);

	if(data.size() == 0)
	{
		min = 0.000001;
		max = 0.00001;
	}
	else
	{
		min = arma::min(input_data);
		max = arma::max(input_data);
	}

	double range_value = max - min;

	double tick_spacing_x = find_tick_spacing(range_value, 3, 10);
	double max_limit_x = std::floor(max / tick_spacing_x) * tick_spacing_x + 0.5 * tick_spacing_x;

	return max_limit_x;
}

void QtPlotting::save_plot()
{

	QPixmap p = chart_view->grab();
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Images (*.png)"));
	
	if (!fileName.isEmpty())
		p.save(fileName, "PNG");

}
