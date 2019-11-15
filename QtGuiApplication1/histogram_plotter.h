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
#include "color_correction_lgg.h"
#include "video_container.h"

QT_CHARTS_USE_NAMESPACE

class HistogramLine_Plot : public QWidget //public QChartView
{

    Q_OBJECT
    public:
        
		Lift_Gamma_Gain color_correction;
        
		QChart *chart, *rel_chart;
		QChartView *chart_view, *rel_chart_view;

		QLabel *text;
		unsigned int maximum_levels;

		HistogramLine_Plot(unsigned int max_levels, QWidget *parent = nullptr);
        ~HistogramLine_Plot();

		QList<QPointF> create_qpoints();
		QList<QPointF> create_qpoints(arma::vec & bins, arma::uvec & values);
		arma::uvec create_histogram_data(arma::vec input);

		arma::vec create_histogram_midpoints(double start, double stop, double bin_size);
		arma::uvec create_histogram_data(arma::vec &values, arma::vec &bin_midpoints);
		void plot_absolute_histogram(arma::vec & values, double min, double max);
		void plot_relative_histogram(arma::vec & values);
	
		void plot_histogram(QList<QPointF> & pts);
		void plot_histogram(QList<QPointF> & pts, double min, double max, double maximum_histogram_level);

	public slots:
		void update_histogram_chart();
		void update_specific_histogram(unsigned int frame_number);
		void update_color_correction(double lift, double gamma, double gain);
		void receive_video_data(video_details &new_input);
		void toggle_enhanced_dynamic_range(bool input);

    private:

		//std::vector<uint8_t*> video_frames;
		std::vector<std::vector<unsigned int>> histogram_data;
		std::vector<std::vector<uint16_t>> video_frames;

		unsigned int counter, number_of_bins, number_of_frames;

		QPen pen, pen_limits;
		ColorScheme colors;

};

#endif // HISTOGRAM_PLOTTER_H
