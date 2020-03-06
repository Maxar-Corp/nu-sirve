#pragma once
#ifndef CLICKABLE_LABEL_H
#define CLICKABLE_LABEL_H

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
	void highlighted_area(QPoint pt0, int width, int height);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

};

/*
class ClickableLabel : public QLabel {
	Q_OBJECT

public:
	explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~ClickableLabel();

signals:
	void clicked();

protected:
	void mousePressEvent(QMouseEvent* event);
}
*/


#endif // CLICKABLE_LABEL_H