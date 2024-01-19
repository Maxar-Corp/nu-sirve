#include "histogram_plotter.h"


HistogramLine_Plot::HistogramLine_Plot(QWidget *parent)
{
	abs_chart = new QChart();
	rel_chart = new QChart();

	abs_chart_view = new Clickable_QChartView(abs_chart);
	rel_chart_view = new QChartView(rel_chart);

	text = new QLabel(this);

	abs_chart->legend()->hide();
	rel_chart->legend()->hide();

	pen.setColor(colors.GetCurrentColor());
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(3);

	pen_limits.setColor(colors.Get_Color(2));
	pen_limits.setStyle(Qt::SolidLine);
	pen_limits.setWidth(3);

	// ------------------------------------------------------------------------------

	bin_midpoints = create_histogram_midpoints();

	connect(abs_chart_view, &Clickable_QChartView::click_drag, this, &HistogramLine_Plot::adjust_color_correction);
}

HistogramLine_Plot::~HistogramLine_Plot(){

	delete abs_chart;
	delete rel_chart;
	
	delete abs_chart_view;
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

void HistogramLine_Plot::remove_histogram_plots()
{
	abs_chart->removeAllSeries();
	rel_chart->removeAllSeries();
}

arma::vec HistogramLine_Plot::create_histogram_midpoints()
{
	int number_of_bins = 100;
	double bin_size = 1.0 / number_of_bins;

	arma::vec bin_midpoints = arma::regspace(0, bin_size, 1);

	return bin_midpoints;
}

void HistogramLine_Plot::update_histogram_abs_plot(arma::vec & values, double lift, double gain)
{
	arma::uvec bin_counts = arma::hist(values, bin_midpoints);

	QList<QPointF> histogram_line = create_qpoints(bin_midpoints, bin_counts);

	double max_hist_value = bin_counts.max();

	plot_histogram(histogram_line, lift, gain, max_hist_value, abs_chart);
}

void HistogramLine_Plot::update_histogram_rel_plot(arma::vec & values)
{
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
	
	double bin_size, bin_x, bin_x_next, bin_delta;
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