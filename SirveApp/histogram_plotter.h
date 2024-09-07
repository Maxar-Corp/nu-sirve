#ifndef HISTOGRAM_PLOTTER_H
#define HISTOGRAM_PLOTTER_H

#include <armadillo>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

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

#include "color_scheme.h"
#include "clickable_chartview.h"

QT_CHARTS_USE_NAMESPACE


class HistogramLinePlot : public QWidget //public QChartView
{

    Q_OBJECT
    public:
        HistogramLinePlot(QWidget *parent = nullptr);
        ~HistogramLinePlot();

		QChart *abs_chart, *rel_chart;
		QChartView *rel_chart_view;
		ClickableQChartView *abs_chart_view;

		QLabel *text;

		arma::vec bin_midpoints;

		QList<QPointF> CreateQPoints(arma::vec & bins, arma::uvec & values);

		void UpdateHistogramAbsPlot(arma::vec & values, double lift, double gain);
		void UpdateHistogramRelPlot(arma::vec & values);
	
		void RemoveHistogramPlots();

	signals:
        void clickDragHistogram(double x0, double x1);


	public slots:
		void AdjustColorCorrection(double x0, double x1);

    private:

		unsigned int counter, number_of_frames;

		QPen pen, pen_limits;
		ColorScheme colors;

		arma::vec CreateHistogramMidpoints();

		void PlotHistogram(QList<QPointF> & pts, double min, double max, double maximum_histogram_level, QChart *input_chart);
		void SetupHistogramPlot(QChart *input_chart);
};

#endif // HISTOGRAM_PLOTTER_H
