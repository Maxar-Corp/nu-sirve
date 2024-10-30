
#include "color_scheme.h"
#include "qt_plotting.h"

#include "jkqtplotter/jkqtplotter.h"
#include "jkqtplotter/graphs/jkqtplines.h"

#include <armadillo>

QtPlotting::QtPlotting()
{

}

QtPlotting::~QtPlotting()
{
    delete chart;
    delete chart_view;
}

void QtPlotting::StartNewChart()
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


    if (yaxis_is_scientific)
    {
        axis_y->setLabelFormat("%.3e");
        axis_ylog->setLabelFormat("%.3e");
    }
    else
    {
        axis_y->setLabelFormat("%.3f");
        axis_ylog->setLabelFormat("%.3f");
    }

    if (xaxis_is_fixed_pt)
    {
        axis_x->setLabelFormat("%.3f");
    }
    else
    {
        axis_x->setLabelFormat("%d");
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

void QtPlotting::AddSeriesWithColor(std::vector<double> x, std::vector<double> y, QColor color)
{
    double base_x_distance = 1.5;

    QLineSeries *series = new QLineSeries();

    QPen pen;
    pen.setColor(color);
    series->setPen(pen);

    series->append(x[0], y[0]);
    for (size_t i = 1; i < x.size(); i++)
    {
        //If there was a break, add the old series and start a new one
        if (x[i] - x[i-1] > base_x_distance)
        {
            chart->addSeries(series);

            series = new QLineSeries();
            series->setPen(pen);
        }

        series->append(x[i], y[i]);
        base_x_distance = 1.5 * x[i] - x[i-1];
    }

    chart->addSeries(series);
}

void QtPlotting::AddSeries(QXYSeries *series, std::vector<double> x, std::vector<double> y, bool broken_data)
{
    size_t num_data_pts = x.size();
    int num_breaks = 0;

    if(osm_track_color == QColor(0,0,0,255)){
        osm_track_color = QColor(0,0,0);
    }
    double base_x_distance = 1;
    if (num_data_pts > 1)
    {
        // distance to separate into different line series is the median difference of the x-value set
        arma::vec x_vector(x);
        arma::vec diff = arma::diff(x_vector);
        base_x_distance = 1.5; //arma::median(diff);
    }

    for (size_t i = 0; i < num_data_pts; i++)
    {
        if (i == 0)
        { //If first pt in series then continue...
            series->append(x[i], y[i]);
            series->setColor(osm_track_color);
        }
        else if ((x[i] - x[i - 1] > base_x_distance) & broken_data)
        {
            //if current point is greater than 1 frame away then start new series...
            chart->addSeries(series);
            series->setColor(osm_track_color);

            if (num_breaks > 0)
                RemoveSeriesLegend();
            num_breaks++;

            series = new QLineSeries();
            series->append(x[i], y[i]);
            series->setColor(osm_track_color);
        }
        else
        {
            //if continuation of current series...
            series->append(x[i], y[i]);
            series->setColor(osm_track_color);
        }

    }

    QPen pen;
    pen.setColor(osm_track_color);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    series->setPen(pen);

    chart->addSeries(series);

    if ((num_breaks > 0) & broken_data)
        RemoveSeriesLegend();
}

double QtPlotting::FindTickSpacing(double data_range, int min_number_ticks, int max_number_ticks)
{
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

void QtPlotting::RemoveSeriesLegend()
{
    int num_legend_entries = chart->legend()->markers().size();
    if (num_legend_entries > 1)
    {
        QLegendMarker *last_entry = chart->legend()->markers().last();
        last_entry->setVisible(false);
    }
}

void QtPlotting::DefineChartProperties(double min_x, double max_x, double min_y, double max_y)
{
    axis_x->setRange(min_x, max_x);
    axis_x->setTitleText(x_title);

    if (yaxis_is_log)
    {
        if (min_y <= 0.001)
            min_y = 0.01;
        if (max_y <= 0.001)
            max_y = 0.01;

        axis_ylog->setRange(min_y, max_y);
        axis_ylog->setTitleText(y_title);
    }
    else
    {
        axis_y->setRange(min_y, max_y);
        axis_y->setTitleText(y_title);
    }

    int num_series = chart->series().size();

    QList<QAbstractSeries *> all_series = chart->series();

    for (int i = 0; i < num_series; i++)
    {
        all_series[i]->attachAxis(axis_x);

        if (yaxis_is_log)
        {
            all_series[i]->attachAxis(axis_ylog);
        }
        else
        {
            all_series[i]->attachAxis(axis_y);
        }
    }

    chart->setMargins(QMargins(0, 0, 0, 0));

    // Legend properties
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignTop);
    chart->legend()->setMarkerShape(QLegend::MarkerShapeDefault);

    // Get all legend markers
    QList<QLegendMarker *> markers = chart->legend()->markers();

    // Hide all but the OSM marker at index 1:
    markers[0]->setVisible(false);

    for (int i = 2; i < markers.size(); i++) {

        markers[i]->setVisible(false);
    }
}

void QtPlotting::set_axis_limits(QAbstractAxis *axis, double min_x, double max_x)
{
    axis->setMin(min_x);
    axis->setMax(max_x);
}

void QtPlotting::set_xaxis_limits(double min_x, double max_x)
{
    chart->axisX()->setRange(min_x, max_x);
}

void QtPlotting::set_yaxis_limits(double min_y, double max_y)
{
    chart->axisY()->setRange(min_y, max_y);
}

double QtPlotting::FindMaxForAxis(std::vector<double> data)
{
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

    double tick_spacing_x = FindTickSpacing(range_value, 3, 10);
    double max_limit_x = std::floor(max / tick_spacing_x) * tick_spacing_x + 0.5 * tick_spacing_x;

    return max_limit_x;
}

void QtPlotting::SavePlot()
{
    QPixmap p = chart_view->grab();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Images (*.png)"));

    if (!fileName.isEmpty())
        p.save(fileName, "PNG");

}
