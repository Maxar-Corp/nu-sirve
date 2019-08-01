#include "color_correction_plot.h"

ColorPlotter::ColorPlotter(QWidget * parent)
{
	chart = new QChart();
	chart_view = new QChartView(chart);
	QList<QPointF> original_line;
	
	for (int i = 0; i <= 100; i++)
	{
		x_points.push_back(i / 100.);
		QPoint temp(i / 100., i / 100.);
		original_line.push_back(temp);
	}

	// ----------------------------------------------------

	original_series = new QLineSeries();
	updated_series = new QLineSeries();
	
	QColor original_color(colors.GetCurrentColor());
	original_pen.setStyle(Qt::DashLine);
	original_pen.setWidth(3);

	original_series->setPen(original_pen);
	original_series->append(original_line);

	chart->addSeries(original_series);
	// ----------------------------------------------------

	QColor updated_color(colors.GetCurrentColor());
	updated_pen.setStyle(Qt::SolidLine);
	updated_pen.setWidth(3);

	// ----------------------------------------------------

	update_chart_properties();

}

ColorPlotter::~ColorPlotter()
{
	delete chart;
	delete chart_view;
	delete original_series;
	delete updated_series;
}

void ColorPlotter::update_chart_properties() {
	
	chart->createDefaultAxes();
	QAbstractAxis *x_axis = chart->axes(Qt::Horizontal)[0];
	QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];
	
	x_axis->setTitleText("Input");
	y_axis->setTitleText("Output");

	chart->setMargins(QMargins(0, 0, 0, 0));
	
	y_axis->setMinorGridLineVisible(true);
	y_axis->setLabelsVisible(false);

	y_axis->setMin(0);
	y_axis->setMax(1);
	x_axis->setMin(0);
	x_axis->setMax(1);

	x_axis->setMinorGridLineVisible(true);
	x_axis->setLabelsVisible(false);

	chart->legend()->hide();
}

void ColorPlotter::update_color_chart() {

	QList<QPointF> updated_line;
	double x_value, y_value;

	for (int i = 0; i <= 100; i++)
	{
		x_value = i / 100.;
		y_value = color_correction.get_updated_color(i, 101) / 100.;
		QPointF temp(x_value, y_value);
		updated_line.append(temp);
	}

	chart->removeSeries(updated_series);
	delete updated_series;

	updated_series = new QLineSeries();
	updated_series->setPen(updated_pen);
	updated_series->setColor(colors.GetCurrentColor());
	updated_series->append(updated_line);

	chart->addSeries(updated_series);

	update_chart_properties();
}

void ColorPlotter::update_color_correction(double lift, double gamma, double gain)
{
	color_correction.set_lift(lift);
	color_correction.set_gamma(gamma);
	color_correction.set_gain(gain);
}
