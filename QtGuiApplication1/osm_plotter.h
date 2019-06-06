#pragma once

#ifndef OSMPLOTTER_H
#define OSMPLOTTER_H

#include <iostream>
#include <string>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include <QString>
#include <vector>
#include <math.h>

#include "color_scheme.h"

//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------

QT_CHARTS_USE_NAMESPACE

class OSMPlotter : public QChartView
{
    Q_OBJECT
    public:

        QChart *chart;
		QChartView *chart_view;
        ColorScheme colors;

		OSMPlotter(QWidget *parent = nullptr);
		~OSMPlotter();

		void Add_Scatter_Series(char* series_name, std::vector<double> x, std::vector<double> y,double marker_size = 5);
		void Add_Line_Series(const char* series_name, std::vector<unsigned int> x, std::vector<double> y);
		void Add_Attributes(char* chart_title, bool display_legend = false);

    private:

        unsigned int max_frames, min_frames ;
        double max_radiance, min_radiance;
        unsigned int max_num_ticks, min_num_ticks;

        //TODO add function for proper zooming of osm_plot
        //TODO add function for identifying frame numbers on osm_plot

		void Set_Max_Frames(unsigned int value);
		void Set_Max_Radiance(double value);

		double FindTickSpacing(double value);
		void SetAxes();

		void Remove_Series_Legend();

};

#endif // OSMPLOTTER_H
