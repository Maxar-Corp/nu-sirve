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

	QPoint pt0;

signals:
	void click_drag(QPoint pt0, QPoint pt1);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

};

#endif
