#include "plot_engineering_data.h"


Engineering_Plots::Engineering_Plots(int number_of_frames, QWidget *parent) : QtPlotting(parent)
{
	num_frames = number_of_frames;
	for (unsigned int i = 0; i < num_frames; i++) {
		frame_indeces.push_back(i);
	}
		
	x_axis_units = frames;
	plot_all_data = true;
	plot_primary_only = false;
	plot_current_marker = false;
	yaxis_is_log = false;
	yaxis_is_scientific = false;
	set_plot_title("EDIT CLASSIFICATION");
	chart_is_zoomed = false;
	current_chart_id = 0;

	index_sub_plot_xmin = 0;
	index_sub_plot_xmax = num_frames - 1;

	QObject::connect(chart_view, &NewChartView::zoom_changed, this, &Engineering_Plots::set_zoom_limits);
}

Engineering_Plots::~Engineering_Plots()
{	

}

void Engineering_Plots::plot_azimuth() {
	// ---------------------------------------------------------------------------------------------------------------
	// Clear chart
	chart->removeAllSeries();
	colors.reset_colors();
	start_new_chart();
	create_current_marker();

	current_chart_id = 1;
	// ---------------------------------------------------------------------------------------------------------------

	std::vector<double> x_points, y_points;

	int number_tracks = track_irradiance_data.size();
	int plot_number_tracks = number_tracks;
	if (plot_primary_only && plot_number_tracks > 0)
		plot_number_tracks = 1;

	for (int i = 0; i < plot_number_tracks; i++)
	{
		QLineSeries* series = new QLineSeries();
		QColor base_color(colors.GetCurrentColor());
		series->setColor(base_color);

		std::vector<double> x_values = get_individual_x_track(i);
		std::vector<double> y_values = track_irradiance_data[i].azimuth;

		add_series(series, x_values, y_values, true);

		// get next color for series
		colors.GetNextColor();

		// Add all x/y points to a common vector for processing later
		x_points.insert(x_points.end(), x_values.begin(), x_values.end());
		y_points.insert(y_points.end(), y_values.begin(), y_values.end());
	}

	//std::vector<double>min_max_x, min_max_y;
	//min_max_x = find_min_max(x_points);
	//min_max_y = find_min_max(y_points);

	establish_plot_limits();

	y_title = QString("Azimuth (deg)");

	if (chart_is_zoomed) {
		std::vector<double> x_data;
		get_xaxis_value(x_data);
		chart_options(x_data[index_zoom_min], x_data[index_zoom_max], 0, 360, x_title, y_title);
	}
	else if (plot_all_data) {
		chart_options(full_plot_xmin, full_plot_xmax, 0, 360, x_title, y_title);
	}
	else{
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, 360, x_title, y_title);
	}


	draw_title();
	
}

void Engineering_Plots::plot_elevation() {
	
	// ---------------------------------------------------------------------------------------------------------------
	// Clear chart
	chart->removeAllSeries();
	colors.reset_colors();
	start_new_chart();
	create_current_marker();

	current_chart_id = 2;
	// ---------------------------------------------------------------------------------------------------------------

	std::vector<double> x_points, y_points;

	int number_tracks = track_irradiance_data.size();
	int plot_number_tracks = number_tracks;
	if (plot_primary_only && plot_number_tracks > 0)
		plot_number_tracks = 1;

	for (int i = 0; i < plot_number_tracks; i++)
	{
		QLineSeries *series = new QLineSeries();
		QColor base_color(colors.GetCurrentColor());
		series->setColor(base_color);

		std::vector<double> x_values = get_individual_x_track(i);
		std::vector<double> y_values = track_irradiance_data[i].elevation;

		add_series(series, x_values, y_values, true);

		// get next color for series
		colors.GetNextColor();

		// Add all x/y points to a common vector for processing later
		x_points.insert(x_points.end(), x_values.begin(), x_values.end());
		y_points.insert(y_points.end(), y_values.begin(), y_values.end());
	}

	//std::vector<double>min_max_x, min_max_y;
	//min_max_x = find_min_max(x_points);
	//min_max_y = find_min_max(y_points);

	establish_plot_limits();

	y_title = QString("Elevation (deg)");

	if (chart_is_zoomed) {
		std::vector<double> x_data;
		get_xaxis_value(x_data);
		chart_options(x_data[index_zoom_min], x_data[index_zoom_max], 0, 90, x_title, y_title);
	}
	else if (plot_all_data)
		chart_options(full_plot_xmin, full_plot_xmax, 0, 90, x_title, y_title);
	else
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, 90, x_title, y_title);


	draw_title();
}

void Engineering_Plots::plot_irradiance()
{
	// ---------------------------------------------------------------------------------------------------------------
	// Clear chart
	chart->removeAllSeries();
	colors.reset_colors();
	start_new_chart();
	create_current_marker();

	current_chart_id = 0;
	// ---------------------------------------------------------------------------------------------------------------
	
	std::vector<double> x_points, y_points;

	int number_tracks = track_irradiance_data.size();
	int plot_number_tracks = number_tracks;
	if (plot_primary_only && plot_number_tracks > 0)
		plot_number_tracks = 1;

	for (int i = 0; i < plot_number_tracks; i++)
	{
		QLineSeries *series = new QLineSeries();
		QColor base_color(colors.GetCurrentColor());
		series->setColor(base_color);

		std::vector<double> x_values = get_individual_x_track(i);

		add_series(series, x_values, track_irradiance_data[i].irradiance, true);

		// get next color for series
		colors.GetNextColor();

		// Add all x/y points to a common vector for processing later
		x_points.insert(x_points.end(), x_values.begin(), x_values.end());
		y_points.insert(y_points.end(), track_irradiance_data[i].irradiance.begin(), track_irradiance_data[i].irradiance.end());
	}

	std::vector<double>min_max_x, min_max_y;
	//min_max_x = find_min_max(x_points);
	min_max_y = find_min_max(y_points);

	establish_plot_limits();	

	y_title = QString("Irradiance Counts");

	if (chart_is_zoomed) {
		std::vector<double> x_data;
		get_xaxis_value(x_data);
		chart_options(x_data[index_zoom_min], x_data[index_zoom_max], 0, find_max_for_axis(min_max_y), x_title, y_title);
	}
	else if (plot_all_data)
		chart_options(full_plot_xmin, full_plot_xmax, 0, find_max_for_axis(min_max_y), x_title, y_title);
	else
		chart_options(sub_plot_xmin, sub_plot_xmax, 0, find_max_for_axis(min_max_y), x_title, y_title);


	draw_title();
}

std::vector<double> Engineering_Plots::get_individual_x_track(int i)
{

	std::vector<double> x_values;

	switch (x_axis_units)
	{
	case frames:
		x_title = QString("Frame #");
		x_values = track_irradiance_data[i].frame_number;
		break;
	case seconds_past_midnight:
		x_title = QString("Seconds Past Midnight");
		x_values = track_irradiance_data[i].past_midnight;
		break;
	case seconds_from_epoch:
		x_title = QString("Seconds Past Epoch");
		x_values = track_irradiance_data[i].past_epoch;
		break;
	default:

		break;
	}
	return x_values;
}

void Engineering_Plots::establish_plot_limits() {

	switch (x_axis_units)
	{
		case frames:
			sub_plot_xmin = index_sub_plot_xmin + 1;
			sub_plot_xmax = index_sub_plot_xmax + 1;

			full_plot_xmin = 1;
			full_plot_xmax = num_frames;
			break;
		case seconds_past_midnight:
			sub_plot_xmin = past_midnight[index_sub_plot_xmin];
			sub_plot_xmax = past_midnight[index_sub_plot_xmax];

			full_plot_xmin = past_midnight[0];
			full_plot_xmax = past_midnight[past_midnight.size() - 1];
			break;
		case seconds_from_epoch:
			sub_plot_xmin = past_epoch[index_sub_plot_xmin];
			sub_plot_xmax = past_epoch[index_sub_plot_xmax];

			full_plot_xmin = past_epoch[0];
			full_plot_xmax = past_epoch[past_epoch.size() - 1];
			break;
		default:
			break;
	}
}

std::vector<double> Engineering_Plots::find_min_max(std::vector<double> data)
{
	
	arma::vec input_data(data);

	if(data.size() == 0)
		return { 0.000001, 0.00001 };

	double min_value = arma::min(input_data);
	double max_value = arma::max(input_data);

	return {min_value, max_value};
}

void Engineering_Plots::set_xaxis_units(x_plot_variables unit_choice)
{
	x_axis_units = unit_choice;
}

void Engineering_Plots::get_xaxis_value(std::vector<double>& values)
{

	switch (x_axis_units)
	{
		case frames:
			values = frame_indeces;
			x_title = "Frame #";
			break;
		case seconds_past_midnight:
			values = past_midnight;
			x_title = "Seconds Past Midnight";
			break;
		case seconds_from_epoch:
			values = past_epoch;
			x_title = "Seconds Past Epoch";
			break;
		default:
			break;
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
	
	if (!current_frame_marker == NULL) {

		reset_current_marker();
	}

	if (plot_current_marker)
	{
		std::vector<double> x;
		get_xaxis_value(x);

		double current_x = x[index_sub_plot_xmin + counter + 1];
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

void Engineering_Plots::reset_current_marker() {

	current_frame_marker->replace(0, 0, 0);
	current_frame_marker->replace(1, 0, 0);
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

void Engineering_Plots::set_zoom_limits(bool active_zoom) {

	chart_is_zoomed = active_zoom;

	if (!chart_is_zoomed) {

		switch (current_chart_id)
		{

		case 0:
			plot_irradiance();
			break;
		case 1:
			plot_azimuth();
			break;
		case 2:
			plot_elevation();
			break;

		default:
			break;
		}

		return;
	}

	std::vector<double> x_data;
	get_xaxis_value(x_data);

	int length = x_data.size();
	double min_value = axis_x->min();
	double max_value = axis_x->max();

	if (min_value < x_data[0])
		min_value = x_data[0];
	if (max_value > x_data[length - 1])
		max_value = x_data[length - 1];

	int index0 = 0;
	int index1 = 0;

	double check_value = x_data[index0];
	while (check_value < min_value)
	{
		index0++;
		check_value = x_data[index0];
	}
	index0 = index0 - 1;
	if (index0 < 0)
		index0 = 0;
	index1 = index0;

	check_value = x_data[index1];
	while (check_value < max_value)
	{
		index1++;
		check_value = x_data[index1];
	}

	index_zoom_min = index0;
	index_zoom_max = index1;
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
	
	chart_zoomed = false;

}

void NewChartView::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() == Qt::RightButton)
	{
		newchart->zoomReset();
		chart_zoomed = false;

		emit zoom_changed(chart_zoomed);
		return; //event doesn't go further
	}
	
	QChartView::mouseReleaseEvent(e);//any other event
	
	chart_zoomed = true;
	emit zoom_changed(chart_zoomed);
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

QtPlotting::QtPlotting(QWidget *parent)
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

	uint num_data_pts = x.size();
	int num_breaks = 0;

	double base_x_distance = 1;
	if (num_data_pts > 1) {

		// distance to separate into different line series is the median difference of the x-value set

		arma::vec x_vector(x);
		arma::vec diff = arma::diff(x_vector);
		base_x_distance = 1.5; //arma::median(diff);
		
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

double QtPlotting::find_max_for_axis(std::vector<double>min_max_values) {

	double range_value = min_max_values[1] - min_max_values[0];

	double tick_spacing_x = find_tick_spacing(range_value, 3, 10);
	double max_limit_x = std::floor(min_max_values[1] / tick_spacing_x) * tick_spacing_x + 0.5 * tick_spacing_x;

	return max_limit_x;
}

void QtPlotting::save_plot()
{

	QPixmap p = chart_view->grab();
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Images (*.png)"));
	
	if (!fileName.isEmpty())
		p.save(fileName, "PNG");

}
