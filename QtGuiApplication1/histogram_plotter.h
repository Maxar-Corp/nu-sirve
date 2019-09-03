#ifndef HISTOGRAM_PLOTTER_H
#define HISTOGRAM_PLOTTER_H

#include <iostream>
#include <string>
#include <QtCharts/QChartView>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
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
        
		QChart *chart;
		QChartView *chart_view;

		QLabel *text;
		unsigned int maximum_levels;

		HistogramLine_Plot(unsigned int max_levels, QWidget *parent = nullptr);
        ~HistogramLine_Plot();

		QList<QPointF> create_qpoints();
		QList<QPointF> create_qpoints(arma::uvec values);

	public slots:
		void update_histogram_chart();
		void update_specific_histogram(unsigned int frame_number);
		void update_color_correction(double lift, double gamma, double gain);
		void receive_video_data(video_details &new_input);

    private:

		//std::vector<uint8_t*> video_frames;
		std::vector<std::vector<unsigned int>> histogram_data;
		std::vector<std::vector<uint16_t>> video_frames;

		unsigned int counter, number_of_bins, number_of_frames;

		QPen pen;
		ColorScheme colors;

};

#endif // HISTOGRAM_PLOTTER_H
