#pragma once
#ifndef CUSTOM_INPUT_DIALOG_H
#define CUSTOM_INPUT_DIALOG_H

#include "logging.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qdialog.h>
#include <qcombobox.h>
#include <qmessagebox.h>

#include <qgridlayout.h>

#include "Data_Structures.h"
#include "video_class.h"


class CustomInputDialog : public QDialog
{
	Q_OBJECT

public:
	CustomInputDialog(QVector<QString> combo_box_options, QString dialog_title, QString combo_label, int * index_combo, Video * input_video, QWidget *parent = nullptr);
	~CustomInputDialog();



public slots:

	void combo_box_changed(const QString &text);

private:

	int *current_index;
	QList<QString> options;
	QComboBox *cmb_options;
	QPushButton *btn_ok, *btn_cancel;
	QLabel *lbl_display;

	QGridLayout *mainLayout;

	Video *current_video;

	void initialize_gui(QVector<QString> combo_box_options, QString dialog_title, QString combo_label);
	void ok();
	void close_window();
	void closeEvent(QCloseEvent *event);
};




#endif