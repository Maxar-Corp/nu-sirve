#pragma once
#ifndef ENHANCED_LABEL_H
#define ENHANCED_LABEL_H

#include <iostream>
#include <qrubberband.h>
#include <qpoint.h>
#include <QLabel>
#include <QMouseEvent> 


class EnhancedLabel : public QLabel
{
	Q_OBJECT
public:
	EnhancedLabel(QWidget *parent = nullptr);
	~EnhancedLabel();

	QRubberBand *rubber_band;
	QPoint origin, edge, pt0;
	int height, width;
	bool right_button_clicked;

	void disable();
	void enable();

signals:
    void areaHighlighted(QRect area);
    void doubleClicked(QPoint pt);
	void clicked(QPoint pt);
    void rightClicked(QPoint pt);
	void hoverPoint(QPoint pt);
	void cursorInImage(bool status);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);
	void hoverEnter(QHoverEvent * event);
	void hoverLeave(QHoverEvent * event);
	void hoverMove(QHoverEvent * event);
	bool event(QEvent * e);

private:
	bool enabled;
};


#endif // ENHANCED_LABEL_H
