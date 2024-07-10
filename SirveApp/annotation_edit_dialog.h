#pragma once
#ifndef ANNOTATIONS_NEW_H
#define ANNOTATIONS_NEW_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qlistwidget.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmessagebox.h>

#include <qgridlayout.h>

#include "annotation_info.h"


class AnnotationEditDialog : public QDialog
{
	Q_OBJECT

public:
    AnnotationEditDialog(AnnotationInfo &data, QWidget *parent = nullptr);
	~AnnotationEditDialog();

	int get_numeric_value(QString input);
	bool check_numeric_value(QString input);

signals:
    void annotationChanged();

public slots:

    void TextChanged();
    void FrameStartChanged();
    void NumberOfFramesChanged();
    void XLocationChanged();
    void YLocationChanged();
    void ColorChanged(const QString &text);
    void FontSizeChanged(const QString &text);

private:
	
    AnnotationInfo *current_data;

	int min_frame, max_frame;

	QList<QString> colors, sizes;
	QComboBox *cmb_colors, *cmb_size;
	QLineEdit *txt_annotation, *txt_x_loc, *txt_y_loc, *txt_frame_start, *txt_num_frames;
	QPushButton *btn_add, *btn_cancel;

    QLabel *lbl_frame_start, *lbl_num_frames, *lbl_color, *lbl_size;

	QGridLayout *mainLayout;	

	void InitializeGui();
	void DisplayError(QString msg);
    void AddDialog();
	void CloseWindow();
    void CloseEvent(QCloseEvent *event);
};


#endif
