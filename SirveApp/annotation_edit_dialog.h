#pragma once
#ifndef ANNOTATIONS_NEW_H
#define ANNOTATIONS_NEW_H

#include <qcombobox.h>
#include <qdialog.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qpushbutton.h>
#include "video_display.h"

#include "annotation_info.h"


class AnnotationEditDialog : public QDialog
{
	Q_OBJECT

public:
    AnnotationEditDialog(AnnotationInfo &data, QString btn_text, QWidget *parent = nullptr);
	~AnnotationEditDialog();

	int get_numeric_value(QString input);
    bool check_numeric_value(QString input);
    // VideoDisplay *current_video;
    QPushButton *btn_add, *btn_cancel;

signals:
    void annotationChanged();

public slots:

    void ColorChanged(const QString &text);
    void FontSizeChanged(const QString &text);
    void FrameStartStopChanged();
    void LocationChanged(QPoint location);
    void TextChanged();

private:
	
    AnnotationInfo *current_data;

	int min_frame, max_frame;

	QList<QString> colors, sizes;
	QComboBox *cmb_colors, *cmb_size;
    QLineEdit *txt_annotation, *txt_x_loc, *txt_y_loc, *txt_frame_start, *txt_frame_stop;

    QLabel *lbl_frame_start, *lbl_frame_stop, *lbl_color, *lbl_size, *lbl_message;

	QGridLayout *mainLayout;	

    void InitializeGui(QString btn_text);
	void DisplayError(QString msg);
    void AddDialog();
	void CloseWindow();
    void CloseEvent(QCloseEvent *event);
};


#endif
