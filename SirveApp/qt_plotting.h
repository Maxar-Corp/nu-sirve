#ifndef QT_PLOTTING_H
#define QT_PLOTTING_H

#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/qxyseries.h>
#include <qfiledialog.h>
#include <QPair>
#include <QString>
#include <vector>

#include "color_scheme.h"
#include "new_chart_view.h"

class QtPlotting : public QWidget
{
public:
    QChart *chart;
    NewChartView *chart_view;
    ColorScheme colors;
    QColor osm_track_color;

    QtPlotting();
    ~QtPlotting();

    QValueAxis *axis_x, *axis_y;
    QLogValueAxis *axis_ylog;
    QString x_title, y_title, title;

    bool yaxis_is_log, yaxis_is_scientific, xaxis_is_fixed_pt;

    void StartNewChart();
    void AddSeries(QXYSeries *series, std::vector<double> x, std::vector<double> y, bool broken_data = false);
    void AddSeriesWithColor(std::vector<double> x, std::vector<double> y, QColor color);
    void RemoveSeriesLegend();
    void DefineChartProperties(double min_x, double max_x, double min_y, double max_y);
    void SavePlot();

    double FindTickSpacing(double value, int min_number_ticks, int max_number_ticks);
    double FindMaxForAxis(std::vector<double> data);

    void set_axis_limits(QAbstractAxis *axis, double min_x, double max_x);
    void set_xaxis_limits(double min_x, double max_x);
    void set_yaxis_limits(double min_y, double max_y);
};

#endif // QT_PLOTTING_H
