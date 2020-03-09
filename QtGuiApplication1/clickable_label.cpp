#include "clickable_label.h"


EnhancedLabel::EnhancedLabel(QWidget *parent) :QLabel(parent)
{
	rubber_band = NULL;
}

EnhancedLabel::~EnhancedLabel()
{
}

void EnhancedLabel::mousePressEvent(QMouseEvent *event)
{
	
	origin = event->pos();

	// if left button click then draw rectangle
	if (event->button() == Qt::LeftButton)
	{
		
		if (!rubber_band)
			rubber_band = new QRubberBand(QRubberBand::Rectangle, this);

		rubber_band->setGeometry(QRect(origin, QSize()));
		rubber_band->show();

		right_button_clicked = false;
		emit clicked(origin);
	}
	
	// if right button click then ...
	if (event->button() == Qt::RightButton)
	{
		right_button_clicked = true;
		emit right_clicked(origin);
	}

}

void EnhancedLabel::mouseMoveEvent(QMouseEvent *event)
{
	// resize rectangle when user is dragging mouse
	rubber_band->setGeometry(QRect(origin, event->pos()).normalized());
}

void EnhancedLabel::mouseReleaseEvent(QMouseEvent *event)
{
	
	


	if (right_button_clicked == false) {
		
		// find rectangle edge point
		edge = event->pos();

		QRect area = QRect(origin, edge).normalized();

		/*
		// calculate point nearest orgin as "origin"
		int x0, y0;
		if (origin.x() < edge.x())
			x0 = origin.x();
		else
			x0 = edge.x();

		if (origin.y() < edge.y())
			y0 = origin.y();
		else
			y0 = edge.y();

		// if x is below zero ...
		if (x0 < 0)
			x0 = 0;

		// if y is below zero ...
		if (y0 < 0)
			y0 = 0;


		// calculate height and width
		width = std::abs(origin.x() - edge.x());
		height = std::abs(origin.y() - edge.y());

		// set origin point
		pt0.setX(x0);
		pt0.setY(y0);
		*/


		// if initial x-point is less than zero, reduce width and set x to zero
		if (area.x() < 0) {
			
			area.setWidth(area.width() + area.x());
			area.setX(0);
			
		}

		// if initial y-point is less than zero, reduce height and set y to zero
		if (area.y() < 0) {
			area.setHeight(area.height() + area.y());
			area.setY(0);
		}

		// hide highlighted area
		rubber_band->hide();

		// if area is large enough then emit signal
		if(area.width() > 10 && area.height() > 10)
			emit highlighted_area(area);
	}
}

void EnhancedLabel::mouseDoubleClickEvent(QMouseEvent * event)
{
	emit double_clicked(origin);
}
