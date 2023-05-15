
#pragma once

#ifndef COLORPLOTTER_H
#define COLORPLOTTER_H

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

#include <vector>
#include <math.h>

#include "color_scheme.h"
#include "color_correction.h"

//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------

QT_CHARTS_USE_NAMESPACE

class ColorPlotter : public QChartView
{
	Q_OBJECT
public:

	Lift_Gamma_Gain color_correction;
	std::vector<double> x_points;
	
	QLineSeries *original_series, *updated_series;

	QPen original_pen, updated_pen;
	
	QChart *chart;
	QChartView *chart_view;
	ColorScheme colors;

	ColorPlotter(QWidget *parent = nullptr);
	~ColorPlotter();
	void update_chart_properties();

public slots:
	void update_color_chart();
	void update_color_correction(double lift, double gamma, double gain);
	
};

#endif // COLORPLOTTER_H
