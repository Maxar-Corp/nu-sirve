#include "clickable_chartview.h"

	
Clickable_QChartView::Clickable_QChartView(QChart *chart, QWidget *parent) :QChartView(chart) {

}

Clickable_QChartView::~Clickable_QChartView() {

}


void Clickable_QChartView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		x0 = get_x_position(event);
	}
}

void Clickable_QChartView::mouseReleaseEvent(QMouseEvent *event) 
{
	if (event->button() == Qt::LeftButton)
	{
		x1 = get_x_position(event);
		emit click_drag(x0, x1);
	}
}

double Clickable_QChartView::get_x_position(QMouseEvent * event)
{
	
	auto const widgetPos = event->localPos();
	auto const scenePos = mapToScene(QPoint(static_cast<int>(widgetPos.x()), static_cast<int>(widgetPos.y())));
	auto const chartItemPos = chart()->mapFromScene(scenePos);
	auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
	
	return valueGivenSeries.x();
}
