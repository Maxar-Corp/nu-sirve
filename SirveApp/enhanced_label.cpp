#include "enhanced_label.h"
#include <QDebug>

EnhancedLabel::EnhancedLabel(QWidget *parent) :QLabel(parent)
{
	rubber_band = NULL;
	enabled = true;
	setAttribute(Qt::WA_Hover);
}

EnhancedLabel::~EnhancedLabel()
{
}

void EnhancedLabel::disable()
{
	enabled = false;
}

void EnhancedLabel::enable()
{
	enabled = true;
}

void EnhancedLabel::mousePressEvent(QMouseEvent *event)
{
	if (!enabled)
	{
		return;
	}

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
		emit rightClicked(origin);
	}
}

void EnhancedLabel::mouseMoveEvent(QMouseEvent *event)
{
	if (!enabled)
	{
		return;
	}

	// resize rectangle when user is dragging mouse
	rubber_band->setGeometry(QRect(origin, event->pos()).normalized());
}

void EnhancedLabel::mouseReleaseEvent(QMouseEvent *event)
{
	if (!enabled)
	{
		return;
	}

	if (right_button_clicked == false) {
		
		// find rectangle edge point
		edge = event->pos();

		QRect area = QRect(origin, edge).normalized();

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
			emit areaHighlighted(area);
	}
}

bool EnhancedLabel::event(QEvent * e)
{
    switch(e->type())
    {
    case QEvent::HoverEnter:
        hoverEnter(static_cast<QHoverEvent*>(e));
        return true;
        break;
    case QEvent::HoverLeave:
        hoverLeave(static_cast<QHoverEvent*>(e));
        return true;
        break;
    case QEvent::HoverMove:
        hoverMove(static_cast<QHoverEvent*>(e));
        return true;
        break;
    default:
        break;
    }
    return QWidget::event(e);
}

void EnhancedLabel::enterEvent(QEvent * e)
{
    // qDebug() << Q_FUNC_INFO << e->type();
	emit cursorInImage(true);
}

void EnhancedLabel::leaveEvent(QEvent * e)
{
    // qDebug() << Q_FUNC_INFO << e->type();
	emit cursorInImage(false);
}

void EnhancedLabel::hoverEnter(QHoverEvent * event)
{
    // qDebug() << Q_FUNC_INFO << event->type();
	emit cursorInImage(true);
}

void EnhancedLabel::hoverLeave(QHoverEvent * event)
{
    // qDebug() << Q_FUNC_INFO << event->type();
	emit cursorInImage(false);
}

void EnhancedLabel::hoverMove(QHoverEvent * event)
{
	emit hoverPoint(event->pos());
}