#pragma once
#ifndef ENHANCED_LABEL_H
#define ENHANCED_LABEL_H

#include <QLabel>
#include <iostream>
#include <qrubberband.h>
#include <qpoint.h>
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

signals:
	void clicked(QPoint pt);
	void right_clicked(QPoint pt);
	void highlighted_area(QRect area);
	void double_clicked(QPoint pt);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
};


#endif // ENHANCED_LABEL_H