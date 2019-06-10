#pragma once

#ifndef ENGINEERING_PLOT_H
#define ENGINEERING_PLOT_H

#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/qxyseries.h>
#include <QString>
#include <vector>
#include <math.h>
#include <armadillo>

#include "color_scheme.h"
#include "osm_reader.h"

QT_CHARTS_USE_NAMESPACE

class QtPlotting : public QWidget
{
	public:
		QChart *chart;
		QChartView *chart_view;
		ColorScheme colors;
		
		QtPlotting(QWidget *parent = nullptr);
		~QtPlotting();

		void add_series(QXYSeries *series, std::vector<double> x, std::vector<double> y, bool broken_data = false);
		void remove_series_legend();
		double find_tick_spacing(double value, int min_number_ticks, int max_number_ticks);
		void chart_options(double min_x, double max_x, double min_y, double max_y, QString x_label_title, QString y_label_title, QString title);
};


class Engineering_Plots : public QtPlotting 
{
	public:

		bool plot_all_data;
		int initial_index, last_index;

		Engineering_Plots(QWidget *parent = nullptr);
		~Engineering_Plots();

	public slots:
		void plot_irradiance_data(std::vector<Track_Irradiance> data);
		void plot_az_el_boresite_data(Plotting_Data data, bool plot_azimuth, bool plot_frames);

};

#endif