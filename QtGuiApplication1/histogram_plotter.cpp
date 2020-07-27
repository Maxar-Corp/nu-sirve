#include "histogram_plotter.h"


HistogramLine_Plot::HistogramLine_Plot(unsigned int max_levels, QWidget *parent)
{
	chart = new QChart();
	rel_chart = new QChart();

	chart_view = new Clickable_QChartView(chart);
	rel_chart_view = new QChartView(rel_chart);

	text = new QLabel(this);

	chart->legend()->hide();
	rel_chart->legend()->hide();

	pen.setColor(colors.GetCurrentColor());
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(3);

	pen_limits.setColor(colors.Get_Color(2));
	pen_limits.setStyle(Qt::SolidLine);
	pen_limits.setWidth(3);

	//Assumes video data is 8 bits and 256 bins (one for each bit level)
	maximum_levels = max_levels;
	number_of_bins = 256;

	// ------------------------------------------------------------------------------

	initialize_histogram_plot();

	connect(chart_view, &Clickable_QChartView::click_drag, this, &HistogramLine_Plot::adjust_color_correction);
}

HistogramLine_Plot::~HistogramLine_Plot(){

	delete chart;
	delete text;
	delete chart_view;
}

void HistogramLine_Plot::receive_video_data(video_details &new_input)
{
	
	counter = 0;

	//maximum_levels = 14;
	//number_of_bins = 256;
	//number_of_frames = new_input.frames_8bit.size();

	histogram_data = new_input.histogram_data;
	video_frames = new_input.frames_16bit;

	update_histogram_chart();

	QString frame_str = "Frame ";
	QString number;
	number.setNum(counter);

	frame_str.append(number);
	text->setText(frame_str);
}

void HistogramLine_Plot::toggle_enhanced_dynamic_range(bool input)
{
	color_correction.enhanced_dynamic_range = input;
}

void HistogramLine_Plot::adjust_color_correction(double x0, double x1)
{
	int a = 1;
}

void HistogramLine_Plot::update_histogram_chart() {

	// Takes ~ 10 ms to run which equates to a max update rate of 100 fps

	if (counter == number_of_frames)
		counter = 0;
	
	//--------------------------------------------------------------------------
	//Convert current frame to armadillo matrix
	std::vector<double> frame_vector(video_frames[counter].begin(), video_frames[counter].end());
	arma::vec color_corrected_matrix(frame_vector);

	// ------------------------------------------------------------------------------

	int max_value = std::pow(2, maximum_levels);
	double normalized_min_value, normalized_max_value;

	color_correction.get_updated_color(color_corrected_matrix, max_value, normalized_min_value, normalized_max_value);

	color_corrected_matrix = color_corrected_matrix * 255;

	//---------------------------------------------------------------------------------
	
	// ------------------------------------------------------------------------------
	//Setup box-whiskers plot
	color_corrected_matrix = arma::sort(color_corrected_matrix);
	int num_pixels = color_corrected_matrix.n_elem;
	int median = arma::median(color_corrected_matrix);
	int upper_quartile = color_corrected_matrix(num_pixels * 0.75);
	int lower_quartile = color_corrected_matrix(num_pixels * 0.25);
	int inter_quartile_range = upper_quartile - lower_quartile;
	int abs_max_value = color_corrected_matrix(num_pixels * 0.99);
	int abs_min_value = color_corrected_matrix(num_pixels * 0.01);

	int max_value_box;
	if (abs_max_value < upper_quartile + 1.5 * inter_quartile_range) {
		max_value_box = abs_max_value;
	}
	else {
		max_value_box = upper_quartile + 1.5 * inter_quartile_range;
	}

	int min_value_box;
	if (abs_min_value < lower_quartile - 1.5 * inter_quartile_range) {
		min_value_box = lower_quartile - 1.5 * inter_quartile_range;
	}
	else {
		min_value_box = abs_min_value;
	}

	QBoxPlotSeries *box_series = new QBoxPlotSeries();

	QBoxSet *box = new QBoxSet();
	box->setValue(QBoxSet::LowerExtreme, min_value_box);
	box->setValue(QBoxSet::UpperExtreme, max_value_box);
	box->setValue(QBoxSet::Median, median);
	box->setValue(QBoxSet::LowerQuartile, lower_quartile);
	box->setValue(QBoxSet::UpperQuartile, upper_quartile);

	box_series->append(box);

	//debug
	//arma::uvec index1 = arma::find(color_corrected_matrix > max_value_box);
	//arma::uvec index2 = arma::find(color_corrected_matrix < min_value_box);
	//int max_index = index1(0);
	//int min_index = index2.max();
	//int dmax_value = color_corrected_matrix(max_index);
	//int dmin_value = color_corrected_matrix(min_index);
	

	// ------------------------------------------------------------------------------
	/*
	int max_value = std::pow(2, maximum_levels);
	double normalized_min_value, normalized_max_value;

	color_correction.get_updated_color(color_corrected_matrix, max_value, normalized_min_value, normalized_max_value);

	color_corrected_matrix = color_corrected_matrix * 255;
	*/
	//---------------------------------------------------------------------------------
	// Bin pixels into histogram
	arma::vec bin_midpoints = arma::linspace(0.5, 254.5, 255);
	arma::uvec bin_counts = arma::hist(color_corrected_matrix, bin_midpoints);

	QList<QPointF> histogram_line = create_qpoints(bin_midpoints, bin_counts);

	//---------------------------------------------------------------------------------
	/*
	//abs_chart = new QChart();
	//abs_chart_view = new QChartView(abs_chart);
	
	abs_chart->removeAllSeries();

	abs_chart->addSeries(box_series);

	double max_plot = 255 * normalized_max_value;
	double min_plot = 255 * normalized_min_value;

	QScatterSeries *series0 = new QScatterSeries();
	// series0->setName("scatter1");
	series0->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	series0->setMarkerSize(15.0);
	series0->append(QPoint(0, min_plot));
	series0->append(QPoint(0, max_plot));

	QScatterSeries *series1 = new QScatterSeries();
	// series0->setName("scatter1");
	series1->setMarkerShape(QScatterSeries::MarkerShapeCircle);
	series1->setMarkerSize(15.0);
	series1->append(QPoint(0, abs_max_value));
	series1->append(QPoint(0, abs_min_value));

	abs_chart->addSeries(series0);
	abs_chart->addSeries(series1);
	int num = abs_chart->axes(Qt::Horizontal).size();

	//Setup axis for absolute histogram chart
	if (abs_chart->axes(Qt::Horizontal).size() == 0)
	{

		//QStringList categories;
		//categories << "Jan";
		//abs_xaxis = new QBarCategoryAxis();
		//abs_xaxis->append(categories);
		//abs_chart->setAxisX(abs_xaxis, series0);
		//abs_chart->setAxisX(abs_xaxis, box_series);

		//abs_yaxis = new QValueAxis();
		//abs_chart->setAxisY(abs_yaxis, series0);
		//abs_chart->setAxisY(abs_yaxis, series1);
		//abs_chart->setAxisY(abs_yaxis, box_series);
		//abs_yaxis->setRange(0, max_value);
	}

	//abs_chart->setAxisX(abs_xaxis, series0);
	//abs_chart->setAxisX(abs_xaxis, series1);
	//abs_chart->setAxisX(abs_xaxis, box_series);

	//abs_chart->setAxisY(abs_yaxis, series0);
	//abs_chart->setAxisY(abs_yaxis, series1);
	//abs_chart->setAxisY(abs_yaxis, box_series);
	//abs_yaxis->setRange(0, 255);
	

	/*
	abs_chart->createDefaultAxes();
	QAbstractAxis *x_axis_abs = abs_chart->axes(Qt::Horizontal)[0];
	QAbstractAxis *y_axis_ab = abs_chart->axes(Qt::Vertical)[0];
	y_axis_ab->setMin(0);
	y_axis_ab->setMax(max_value);
	*/

	//---------------------------------------------------------------------------------

	QLineSeries *series = new QLineSeries();
	series->setPen(pen);

	QColor base_color(colors.GetCurrentColor());
	series->setColor(base_color);
	series->append(histogram_line);

	//---------------------------------------------------------------------------------
	// Validation check of histogram
	//QLineSeries *series2 = new QLineSeries();
	//series2->setPen(pen);
	//arma::uvec bin_counts2 = create_histogram_data(color_corrected_matrix);
	//QList<QPointF> histogram_line2 = create_qpoints(bin_counts2);
	//QColor color2(colors.Get_Color(1));
	//series2->setColor(color2);
	//series2->append(histogram_line2);
	//---------------------------------------------------------------------------------

	chart->removeAllSeries();
	chart->addSeries(series);
	//chart->addSeries(series2);
	chart->createDefaultAxes();
	QAbstractAxis *x_axis = chart->axes(Qt::Horizontal)[0];
	QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];

	x_axis->setTitleText("Luminance");
	y_axis->setTitleText("Count");

	y_axis->setMinorGridLineVisible(true);
	y_axis->setLabelsVisible(false);

	x_axis->setMin(0);
	x_axis->setMax(number_of_bins);
	x_axis->setMinorGridLineVisible(true);
	x_axis->setLabelsVisible(false);

	chart->setTitle(QString("Histogram"));
	chart->setMargins(QMargins(0, 0, 0, 0));
	chart->setContentsMargins(0, 0, 0, 0);

	//---------------------------------------------------------------------------------

	

	counter++;
}

void HistogramLine_Plot::plot_histogram(QList<QPointF> & pts, double min, double max, double maximum_histogram_level, QChart *input_chart) {

	input_chart->removeAllSeries();
	
	// Histogram line
	QLineSeries *series = new QLineSeries();
	series->setPen(pen);
	series->append(pts);
	
	// Limit lines
	QLineSeries *hist_min = new QLineSeries();
	hist_min->append(min, 0);
	hist_min->append(min, maximum_histogram_level);
	hist_min->setPen(pen_limits);
	
	QLineSeries *hist_max = new QLineSeries();
	hist_max->append(max, 0);
	hist_max->append(max, maximum_histogram_level);
	hist_max->setPen(pen_limits);
	
	// Add to chart
	input_chart->addSeries(series);
	input_chart->addSeries(hist_min);
	input_chart->addSeries(hist_max);

	// ---------------------------------------------------------------------------------

	setup_histogram_plot(input_chart);

}

void  HistogramLine_Plot::setup_histogram_plot(QChart *input_chart) {

	input_chart->createDefaultAxes();
	QAbstractAxis *x_axis = input_chart->axes(Qt::Horizontal)[0];
	QAbstractAxis *y_axis = input_chart->axes(Qt::Vertical)[0];

	// Define histogram y-axis
	y_axis->setTitleText("Frequency");
	y_axis->setMinorGridLineVisible(true);
	y_axis->setLabelsVisible(false);

	// Define histogram x-axis
	x_axis->setMin(0);
	x_axis->setMax(1);
	x_axis->setMinorGridLineVisible(true);
	x_axis->setLabelsVisible(true);
	x_axis->setTitleText("Luminance");

	if (input_chart == chart)
		input_chart->setTitle(QString("Absolute Histogram"));
	else
		input_chart->setTitle(QString("Relative Histogram"));

	input_chart->setMargins(QMargins(0.01, 0.01, 0.01, 0.01));
	input_chart->setContentsMargins(0, 0, 0, 0);
}

void HistogramLine_Plot::initialize_histogram_plot()
{

	// Initialize the histogram plots for displaying
	QLineSeries *series1 = new QLineSeries();
	QLineSeries *series2 = new QLineSeries();

	series1->append(QPointF(0, 0));
	series1->append(QPointF(0, 0));
	series2->append(QPointF(0, 0));
	series2->append(QPointF(0, 0));

	chart->addSeries(series1);
	rel_chart->addSeries(series2);

	setup_histogram_plot(chart);
	setup_histogram_plot(rel_chart);

}

void HistogramLine_Plot::remove_histogram_plots()
{
	chart->removeAllSeries();
	rel_chart->removeAllSeries();
}

arma::vec HistogramLine_Plot::create_histogram_midpoints(double start, double stop, double bin_size) {

	double bin_start = bin_size / 2.0;
	double bin_stop = stop - bin_start;
	double num_bins = (1 - bin_size) / bin_size;

	//arma::vec bin_midpoints = arma::linspace(bin_start, bin_stop, num_bins);
	arma::vec bin_midpoints = arma::regspace(bin_start, bin_size, bin_stop);

	return bin_midpoints;
}

arma::uvec HistogramLine_Plot::create_histogram_data(arma::vec input)
{

	//int number_of_bins = std::pow(2, 8);
	int number_pixels = input.n_elem;

	arma::uvec frame_histogram(number_of_bins);
	frame_histogram.fill(0);

	double max = input.max();
	double min = input.min();

	//std::vector<unsigned int> frame_histogram(number_of_bins, 0);

	for (int pixel_index = 0; pixel_index < number_pixels; pixel_index++)
	{
		double value = input(pixel_index);
		int index = (int)value;

		// Check if exceeds maximum bins, puts in last bin
		if (index > number_of_bins - 1)
			index = number_of_bins - 1;
		if (index < 0)
			index = 0;

		frame_histogram(index) = frame_histogram(index) + 1;
	}

	return frame_histogram;
}

arma::uvec HistogramLine_Plot::create_histogram_data(arma::vec &values, arma::vec &bin_midpoints) {

	arma::uvec counts(bin_midpoints.n_elem, arma::fill::zeros);
	double diff = (bin_midpoints(1) - bin_midpoints(0)) / 2;

	int num_midpoints = bin_midpoints.n_elem;
	
	for (int i = 0; i < num_midpoints; i++)
	{
		double bin_min = bin_midpoints(i) - diff;
		double bin_max = bin_midpoints(i) + diff;

		arma::uvec temp = arma::find(values <= bin_min && values > bin_max);
		
		if (temp.n_elem > 0) {
			counts(i) = temp.n_elem;
		}
	}
	
	return counts;
}

void HistogramLine_Plot::plot_absolute_histogram(arma::vec & values, double min, double max)
{

	arma::vec bin_midpoints = create_histogram_midpoints(0, 1, 0.01);
	arma::uvec bin_counts = arma::hist(values, bin_midpoints);

	QList<QPointF> histogram_line = create_qpoints(bin_midpoints, bin_counts);

	double max_hist_value = bin_counts.max();
	plot_histogram(histogram_line, min, max, max_hist_value, chart);

}

void HistogramLine_Plot::plot_relative_histogram(arma::vec & values)
{

	arma::vec bin_midpoints = create_histogram_midpoints(0, 1, 0.01);
	arma::uvec bin_counts = arma::hist(values, bin_midpoints);

	QList<QPointF> histogram_line = create_qpoints(bin_midpoints, bin_counts);

	double max_hist_value = bin_counts.max();
	plot_histogram(histogram_line, 0, 1, max_hist_value, rel_chart);

}

QList<QPointF> HistogramLine_Plot::create_qpoints()
{

	std::vector<std::vector<double>> color_corrected_histogram;
	color_corrected_histogram.reserve(number_of_bins);

	int min_total, max_total, bin_pixels;
	double new_index;

	min_total = 0;
	max_total = 0;

	// Map old bins to new bin values. Sum up values at 0 and max bin size
	for (int i = 0; i < number_of_bins; i++)
	{
		new_index = color_correction.get_updated_color(i, number_of_bins);
		bin_pixels = histogram_data[counter][i];

		if (new_index == 0) {
			min_total += bin_pixels;
		}
		else if (new_index == number_of_bins - 1) {
			max_total += bin_pixels;
		}
		else {
			std::vector<double> temp;
			temp.push_back(new_index);
			temp.push_back(bin_pixels);

			color_corrected_histogram.push_back(temp);
		}
	}

	QList<QPointF> histogram_line;
	histogram_line.reserve(number_of_bins * 2);

	double current_x, next_x, bin_size;
	int number_color_corrected_bins = color_corrected_histogram.size();

	// Create line for histogram using the new bin points
	for (int i = 0; i < number_color_corrected_bins; i++) {

		current_x = color_corrected_histogram[i][0];
		bin_size = color_corrected_histogram[i][1];

		// Add the 0 point on the first iteration
		if (i == 0)
		{
			QPointF temp_pt1(0, min_total);
			histogram_line.push_back(temp_pt1);

			QPointF temp_pt2(current_x, min_total);
			histogram_line.push_back(temp_pt2);
		}

		if (i < number_color_corrected_bins - 1) {

			next_x = color_corrected_histogram[i + 1][0];

			QPointF temp_pt1(current_x, bin_size);
			histogram_line.push_back(temp_pt1);

			QPointF temp_pt2(next_x, bin_size);
			histogram_line.push_back(temp_pt2);
		}
		else {
			double delta_x = color_corrected_histogram[i][0] - color_corrected_histogram[i - 1][0];
			next_x = color_corrected_histogram[i][0] + delta_x;

			if (next_x > number_of_bins - 1) {
				// Current value
				QPointF temp_pt1(current_x, bin_size);
				histogram_line.push_back(temp_pt1);

				// Connect bar to last value
				QPointF temp_pt2(number_of_bins - 1, bin_size);
				histogram_line.push_back(temp_pt2);
			}
			else
			{
				// Current value
				QPointF temp_pt1(current_x, bin_size);
				histogram_line.push_back(temp_pt1);

				// Create the bar for the current value
				QPointF temp_pt2(next_x, bin_size);
				histogram_line.push_back(temp_pt2);

				// Close the bar to zero
				QPointF temp_pt3(next_x, 0);
				histogram_line.push_back(temp_pt3);

				// Move to the end of the histogram
				QPointF temp_pt4(number_of_bins - 1, 0);
				histogram_line.push_back(temp_pt4);

			}

			// Last Value Pt1
			QPointF temp_pt5(number_of_bins - 1, max_total);
			histogram_line.push_back(temp_pt5);

			// Last Value Pt2
			QPointF temp_pt6(number_of_bins, max_total);
			histogram_line.push_back(temp_pt6);
		}
	}

	return histogram_line;
}

QList<QPointF> HistogramLine_Plot::create_qpoints(arma::vec & bins, arma::uvec & values)
{
	int num_bins = bins.n_elem;
	
	QList<QPointF> histogram_line;
	histogram_line.reserve(num_bins * 2);
	
	double current_x, next_x, bin_size, bin_x, bin_x_next, bin_delta;
	int number_color_corrected_bins = values.n_elem;

	bin_delta = (bins(1) - bins(0)) / 2;

	// Create line for histogram using the new bin points
	for (int i = 0; i < number_color_corrected_bins; i++) {

		bin_size = values(i);
		bin_x = bins(i) - bin_delta;

		// Add the 0 point on the first iteration
		if (i == 0)
		{
			QPointF temp_pt0(0, 0);
			histogram_line.push_back(temp_pt0);
		}

		if (i < number_color_corrected_bins - 1) {

			bin_x_next = bins(i) + bin_delta;

			QPointF temp_pt1(bin_x, bin_size);
			histogram_line.push_back(temp_pt1);

			QPointF temp_pt2(bin_x_next, bin_size);
			histogram_line.push_back(temp_pt2);
		}
		else {
			
			bin_x_next = bin_x + 2 * bin_delta;

			// Current value
			QPointF temp_pt1(bin_x, bin_size);
			histogram_line.push_back(temp_pt1);

			// Create the bar for the current value
			QPointF temp_pt2(bin_x_next, bin_size);
			histogram_line.push_back(temp_pt2);

			// Close the bar to zero
			QPointF temp_pt3(bin_x_next, 0);
			histogram_line.push_back(temp_pt3);
		}
	}

	return histogram_line;
}

void HistogramLine_Plot::update_specific_histogram(unsigned int frame_number)
{
	counter = frame_number;
	update_histogram_chart();
}

void HistogramLine_Plot::update_color_correction(double lift, double gamma, double gain)
{
	color_correction.set_lift(lift);
	color_correction.set_gamma(gamma);
	color_correction.set_gain(gain);
}
