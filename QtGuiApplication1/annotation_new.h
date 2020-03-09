#pragma once
#ifndef ANNOTATIONS_NEW_H
#define ANNOTATIONS_NEW_H

#include "logging.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qlistwidget.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmessagebox.h>

#include <qgridlayout.h>

#include "Data_Structures.h"


class NewAnnotation : public QDialog
{
	Q_OBJECT

public:
	NewAnnotation(annotation_info &data, QWidget *parent = nullptr);
	~NewAnnotation();

	int get_numeric_value(QString input);
	bool check_numeric_value(QString input);

public slots:

	void text_changed();
	void frame_start_changed();
	void number_of_frames_changed();
	void x_location_changed();
	void y_location_changed();
	void color_changed(const QString &text);
	void font_size_changed(const QString &text);

private:
	
	annotation_info *current_data;

	int min_frame, max_frame;

	QList<QString> colors, sizes;
	QComboBox *cmb_colors, *cmb_size;
	QLineEdit *txt_annotation, *txt_x_loc, *txt_y_loc, *txt_frame_start, *txt_num_frames;
	QPushButton *btn_add, *btn_cancel;

	QLabel *lbl_frame_start, *lbl_num_frames, *lbl_color, *lbl_size, *lbl_x, *lbl_y;

	QGridLayout *mainLayout;
		

	void initialize_gui();
	void display_error(QString msg);
	void add();
	void close_window();
	void closeEvent(QCloseEvent *event);

};


#endif