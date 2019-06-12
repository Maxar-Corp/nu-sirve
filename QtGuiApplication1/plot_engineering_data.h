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
#include <qfiledialog.h>
#include <QString>
#include <vector>
#include <math.h>
#include <armadillo>

#include "color_scheme.h"
#include "osm_reader.h"
#include "Data_Structures.h"

QT_CHARTS_USE_NAMESPACE


class NewChartView : public QChartView {

	public:
		NewChartView(QChart *chart);
		void mouseReleaseEvent(QMouseEvent *e);

		QChart *newchart;
};


class QtPlotting : public QWidget
{
	public:
		QChart *chart;
		NewChartView *chart_view;
		ColorScheme colors;
		
		QtPlotting(QWidget *parent = nullptr);
		~QtPlotting();

		void add_series(QXYSeries *series, std::vector<double> x, std::vector<double> y, bool broken_data = false);
		void remove_series_legend();
		double find_tick_spacing(double value, int min_number_ticks, int max_number_ticks);
		void chart_options(double min_x, double max_x, double min_y, double max_y, QString x_label_title, QString y_label_title, QString title);
		void set_axis_limits(QAbstractAxis *axis, double min_x, double max_x);
		void set_xaxis_limits(double min_x, double max_x, QString x_label_title, QString y_label_title, QString title);
		void set_yaxis_limits(double min_y, double max_y);
		double find_max_for_axis(std::vector<double>min_max_values);

		void save_plot();

};


class Engineering_Plots : public QtPlotting 
{
	public:

		bool plot_all_data;
		x_plot_variables x_axis_units;
		
		double full_plot_xmin, full_plot_xmax, sub_plot_xmin, sub_plot_xmax;
		int index_sub_plot_xmin, index_sub_plot_xmax;

		QString x_title, y_title, title;

		Engineering_Plots(QWidget *parent = nullptr);
		~Engineering_Plots();

		std::vector<Plotting_Frame_Data> engineering_data;
		std::vector<double>frame_numbers;
		std::vector<double>past_midnight;
		std::vector<double>past_epoch;
		

		void plot_azimuth();
		void plot_elevation();
		void plot_irradiance(int number_tracks);
		std::vector<double> find_min_max(std::vector<double>data);
		void get_xaxis_value(std::vector<double> &values);

	public slots:

		void toggle_subplot();

};

#endif