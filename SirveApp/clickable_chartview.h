#pragma once
#ifndef CLICKABLE_CHARTVIEW_H
#define CLICKABLE_CHARTVIEW_H

#include <iostream>
#include <string>
#include <QtCharts/QChartView>
#include <QMouseEvent> 

#include <QPointF>
//#include <qpoint.h>

QT_CHARTS_USE_NAMESPACE

class Clickable_QChartView : public QChartView
{
	Q_OBJECT
public:
	Clickable_QChartView(QChart *chart, QWidget *parent = nullptr);
	~Clickable_QChartView();

	double x0, x1;

signals:
	void click_drag(double x0, double x1);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

private:
	double get_x_position(QMouseEvent *event);
	double get_y_position(QMouseEvent *event);

};

#endif
