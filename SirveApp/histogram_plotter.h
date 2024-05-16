#ifndef HISTOGRAM_PLOTTER_H
#define HISTOGRAM_PLOTTER_H

#include <iostream>
#include <string>
#include <QtCharts/QChartView>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/qboxset.h>
#include <QtCharts/qboxplotseries.h>
#include <QtCharts/qscatterseries.h>
#include <QtCharts/qbarcategoryaxis.h>
#include <QString>
#include <QList>
#include <QPointF>
#include <qlabel.h>
#include <armadillo>

#include <vector>
#include <math.h>

#include "color_scheme.h"
#include "clickable_chartview.h"

QT_CHARTS_USE_NAMESPACE


class HistogramLinePlot : public QWidget //public QChartView
{

    Q_OBJECT
    public:
		QChart *abs_chart, *rel_chart;
		QChartView *rel_chart_view;
		ClickableQChartView *abs_chart_view;

		QLabel *text;

		arma::vec bin_midpoints;

        HistogramLinePlot(QWidget *parent = nullptr);
        ~HistogramLinePlot();

		QList<QPointF> create_qpoints(arma::vec & bins, arma::uvec & values);

		void update_histogram_abs_plot(arma::vec & values, double lift, double gain);
		void update_histogram_rel_plot(arma::vec & values);
	
		void remove_histogram_plots();

	signals:
		void click_drag_histogram(double x0, double x1);


	public slots:
		void adjust_color_correction(double x0, double x1);

    private:

		//std::vector<uint8_t*> video_frames;

		unsigned int counter, number_of_frames;

		QPen pen, pen_limits;
		ColorScheme colors;

		arma::vec create_histogram_midpoints();

		void plot_histogram(QList<QPointF> & pts, double min, double max, double maximum_histogram_level, QChart *input_chart);
		void setup_histogram_plot(QChart *input_chart);
};

#endif // HISTOGRAM_PLOTTER_H
