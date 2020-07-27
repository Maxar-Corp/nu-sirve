#include "clickable_chartview.h"

	
Clickable_QChartView::Clickable_QChartView(QChart *chart, QWidget *parent) :QChartView(chart) {

}

Clickable_QChartView::~Clickable_QChartView() {

}


void Clickable_QChartView::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		pt0 = event->pos();
	}
}

void Clickable_QChartView::mouseReleaseEvent(QMouseEvent *event) 
{
	if (event->button() == Qt::LeftButton)
	{
		QPoint pt1 = event->pos();
		emit click_drag(pt0, pt1);
	}
}
