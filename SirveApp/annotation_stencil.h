#pragma once
#ifndef ANNOTATION_TEMPLATE_H
#define ANNOTATION_TEMPLATE_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qlistwidget.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qmessagebox.h>

#include <qgridlayout.h>

#include "annotation_info.h"


class AnnotationStencil : public QWidget
{
    Q_OBJECT

public:
    explicit AnnotationStencil(QWidget *parent = nullptr);
    ~AnnotationStencil();
    void InitializeData(AnnotationInfo data);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool _drag_active;
    QPoint _drag_position;
	
    AnnotationInfo *current_data;

	int min_frame, max_frame;

	QList<QString> colors, sizes;
	QComboBox *cmb_colors, *cmb_size;
	QLineEdit *txt_annotation, *txt_x_loc, *txt_y_loc, *txt_frame_start, *txt_num_frames;

signals:

    void mouseMoved(const QPoint &location);
    void mouseReleased(const QPoint &location);

public slots:

    void AnnotationPositioned();

};


#endif
