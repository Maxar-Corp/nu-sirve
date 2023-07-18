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
#include "color_correction.h"
#include "video_details.h"
#include "clickable_chartview.h"

QT_CHARTS_USE_NAMESPACE


class HistogramLine_Plot : public QWidget //public QChartView
{

    Q_OBJECT
    public:
		QChart *chart, *chart_full, *rel_chart;
		QChartView *rel_chart_view;
		Clickable_QChartView *chart_view, *chart_full_view;

		QLabel *text;
		unsigned int maximum_levels;

		HistogramLine_Plot(unsigned int max_levels, QWidget *parent = nullptr);
        ~HistogramLine_Plot();

		QList<QPointF> create_qpoints(arma::vec & bins, arma::uvec & values);
		arma::uvec create_histogram_data(arma::vec input);

		arma::vec create_histogram_midpoints(double start, double stop, double bin_size);
		arma::uvec create_histogram_data(arma::vec &values, arma::vec &bin_midpoints);
		void plot_absolute_histogram(arma::vec & values, double min, double max);
		void plot_relative_histogram(arma::vec & values);
	
		void plot_histogram(QList<QPointF> & pts, double min, double max, double maximum_histogram_level, QChart *input_chart);
		void  setup_histogram_plot(QChart *input_chart);
		void initialize_histogram_plot();
		void remove_histogram_plots();

	signals:
		void click_drag_histogram(double x0, double x1);


	public slots:
		void adjust_color_correction(double x0, double x1);

    private:

		//std::vector<uint8_t*> video_frames;

		unsigned int counter, number_of_bins, number_of_frames;

		QPen pen, pen_limits;
		ColorScheme colors;

};

#endif // HISTOGRAM_PLOTTER_H
