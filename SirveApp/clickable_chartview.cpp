#include "clickable_chartview.h"

	
ClickableQChartView::ClickableQChartView(QChart *chart, QWidget *parent) :QChartView(chart) {

}

ClickableQChartView::~ClickableQChartView() {

}


void ClickableQChartView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		x0 = get_x_position(event);
	}
}

void ClickableQChartView::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		x1 = get_x_position(event);
		emit clickDrag(x0, x1);
	}
}

double ClickableQChartView::get_x_position(QMouseEvent * event)
{
	
	// get coordinates from clicking on chart

	// technique to get coordinates from stackoverflow:
	// https://stackoverflow.com/questions/44067831/get-mouse-coordinates-in-qchartviews-axis-system

	auto const widgetPos = event->localPos();
	auto const scenePos = mapToScene(QPoint(static_cast<int>(widgetPos.x()), static_cast<int>(widgetPos.y())));
	auto const chartItemPos = chart()->mapFromScene(scenePos);
	auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
	
	return valueGivenSeries.x();
}
