#pragma once

#ifndef ENGINEERING_PLOT_H
#define ENGINEERING_PLOT_H

#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/qxyseries.h>
#include <qfiledialog.h>
#include <QString>
#include <vector>
#include <math.h>
#include <armadillo>

#include "color_scheme.h"
#include "Data_Structures.h"
#include <armadillo>

QT_CHARTS_USE_NAMESPACE


class NewChartView : public QChartView {

	Q_OBJECT
	public:
		NewChartView(QChart *chart);
		void mouseReleaseEvent(QMouseEvent *e);
		void apply_nice_numbers();

		QChart *newchart;
		bool chart_zoomed;

	signals:
		void zoom_changed(bool active_zoom);
		
};


class QtPlotting : public QWidget
{
	public:
		QChart *chart;
		NewChartView *chart_view;
		ColorScheme colors;
		
		QtPlotting(QWidget *parent = nullptr);
		~QtPlotting();

		QValueAxis *axis_x, *axis_y;
		QLogValueAxis *axis_ylog;

		bool yaxis_is_log, yaxis_is_scientific;

		void start_new_chart();
		void add_series(QXYSeries *series, std::vector<double> x, std::vector<double> y, bool broken_data = false);
		void remove_series_legend();
		double find_tick_spacing(double value, int min_number_ticks, int max_number_ticks);
		void chart_options(double min_x, double max_x, double min_y, double max_y, QString x_label_title, QString y_label_title);
		void set_axis_limits(QAbstractAxis *axis, double min_x, double max_x);
		void set_xaxis_limits(double min_x, double max_x, QString x_label_title, QString y_label_title, QString title);
		void set_yaxis_limits(double min_y, double max_y);
		double find_max_for_axis(std::vector<double>min_max_values);

		void save_plot();

};

enum x_plot_variables{frames , seconds_past_midnight, seconds_from_epoch};

class Engineering_Plots : public QtPlotting 
{
	Q_OBJECT 
	public:
		// Parameters to display subplot
		bool plot_all_data, plot_primary_only, plot_current_marker, chart_is_zoomed;
		double full_plot_xmin, full_plot_xmax, sub_plot_xmin, sub_plot_xmax;
		unsigned int index_sub_plot_xmin, index_sub_plot_xmax, index_zoom_min, index_zoom_max, current_chart_id;
		std::vector<double>past_midnight,past_epoch;

		// plot axes titles
		QString x_title, y_title, title;
		QXYSeries *current_frame_marker;

		Engineering_Plots(int number_of_frames, QWidget *parent = nullptr);
		~Engineering_Plots();

		int num_frames;
		std::vector<Plotting_Frame_Data> engineering_data;
		std::vector<Track_Irradiance> track_irradiance_data;

		void plot_azimuth();
		void plot_elevation();
		void plot_irradiance();

		std::vector<double> get_individual_x_track(int i);
		void establish_plot_limits();

		std::vector<double> find_min_max(std::vector<double>data);
		void get_xaxis_value(std::vector<double> &values);
		void create_current_marker();
		void reset_current_marker();
		void toggle_yaxis_log(bool input);
		void toggle_yaxis_scientific(bool input);
		void draw_title();

		void set_xaxis_units(x_plot_variables unit_choice);

	public slots:

		void toggle_subplot();
		void plot_current_step(int counter);
		void set_plot_title(QString input_title);
		void set_zoom_limits(bool active_zoom);

	private:
		x_plot_variables x_axis_units;
		std::vector<double> frame_indeces;
};

#endif