#pragma once
#ifndef CUSTOM_INPUT_DIALOG_H
#define CUSTOM_INPUT_DIALOG_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qdialog.h>
#include <qcombobox.h>
#include <qmessagebox.h>

#include <qgridlayout.h>

#include "data_structures.h"


class CustomInputDialog : public QDialog
{
	Q_OBJECT
	

public:

	QComboBox *cmb_options;

	CustomInputDialog(QVector<QString> combo_box_options, QString dialog_title, QString combo_label, int index_combo, QWidget *parent = nullptr);
	~CustomInputDialog();

private:
	
	QPushButton *btn_ok, *btn_cancel;
	QLabel *lbl_display;

	QGridLayout *mainLayout;

	void InitializeGui(QVector<QString> combo_box_options, QString dialog_title, QString combo_label);

    void verifyCustomInputValues();
    void closeWindow();
	void closeEvent(QCloseEvent *event);
};




#endif
