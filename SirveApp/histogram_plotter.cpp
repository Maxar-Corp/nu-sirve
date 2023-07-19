#include "histogram_plotter.h"


HistogramLine_Plot::HistogramLine_Plot(unsigned int max_levels, QWidget *parent)
{
	chart = new QChart();
	chart_full = new QChart();
	rel_chart = new QChart();

	chart_view = new Clickable_QChartView(chart);
	chart_full_view = new Clickable_QChartView(chart_full);
	rel_chart_view = new QChartView(rel_chart);

	text = new QLabel(this);

	chart->legend()->hide();
	chart_full->legend()->hide();
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
	connect(chart_full_view, &Clickable_QChartView::click_drag, this, &HistogramLine_Plot::adjust_color_correction);
}

HistogramLine_Plot::~HistogramLine_Plot(){

	delete chart;
	delete chart_full;
	delete rel_chart;
	
	delete chart_view;
	delete chart_full_view;
	delete rel_chart_view;
	
	delete text;
}

void HistogramLine_Plot::adjust_color_correction(double x0, double x1)
{
	emit click_drag_histogram(x0, x1);
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

	if (input_chart == rel_chart)
		input_chart->setTitle(QString("Relative Histogram"));
	else
		input_chart->setTitle(QString("Absolute Histogram"));

	input_chart->setMargins(QMargins(0.01, 0.01, 0.01, 0.01));
	input_chart->setContentsMargins(0, 0, 0, 0);
}

void HistogramLine_Plot::initialize_histogram_plot()
{

	// Initialize the histogram plots for displaying
	QLineSeries *series1 = new QLineSeries();
	QLineSeries *series2 = new QLineSeries();
	QLineSeries* series3 = new QLineSeries();

	series1->append(QPointF(0, 0));
	series1->append(QPointF(0, 0));
	
	series2->append(QPointF(0, 0));
	series2->append(QPointF(0, 0));
	
	series3->append(QPointF(0, 0));
	series3->append(QPointF(0, 0));

	chart->addSeries(series1);
	chart_full->addSeries(series3);
	rel_chart->addSeries(series2);

	setup_histogram_plot(chart);
	setup_histogram_plot(chart_full);
	setup_histogram_plot(rel_chart);

}

void HistogramLine_Plot::remove_histogram_plots()
{
	chart->removeAllSeries();
	chart_full->removeAllSeries();
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
	plot_histogram(histogram_line, min, max, max_hist_value, chart_full);
}

void HistogramLine_Plot::plot_relative_histogram(arma::vec & values)
{
	arma::vec bin_midpoints = create_histogram_midpoints(0, 1, 0.01);
	arma::uvec bin_counts = arma::hist(values, bin_midpoints);

	QList<QPointF> histogram_line = create_qpoints(bin_midpoints, bin_counts);

	double max_hist_value = bin_counts.max();
	plot_histogram(histogram_line, 0, 1, max_hist_value, rel_chart);
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