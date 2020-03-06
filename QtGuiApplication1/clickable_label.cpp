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

		// calculate height and width
		width = std::abs(origin.x() - edge.x());
		height = std::abs(origin.y() - edge.y());

		// set origin point
		pt0.setX(x0);
		pt0.setY(y0);

		// hide highlighted area
		rubber_band->hide();

		// if area is large enough then emit signal
		if(width > 10 && height > 10)
			emit highlighted_area(pt0, width, height);
	}
}
