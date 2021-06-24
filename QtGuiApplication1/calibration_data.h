#pragma once
#ifndef CALIBRATION_DATA_H
#define CALIBRATION_DATA_H

#include <iostream>
#include <string>
#include <math.h>

#include <armadillo>

#include "logging.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qdialog.h>
#include <qcombobox.h>
#include <qmessagebox.h>

#include <qgridlayout.h>

class CalibrationData {

public:
	CalibrationData();

	double measure_irradiance(int ul_row, int ul_col, int lr_row, int lr_col);

private:
	bool calibration_available; 
	arma::mat m, b;
};


class CalibrationDialog : public QDialog
{
	Q_OBJECT


public:

	CalibrationDialog(QWidget* parent = nullptr);
	~CalibrationDialog();

private:

	QPushButton* btn_ok, * btn_cancel;
	QLabel* lbl_display;

	QGridLayout* mainLayout;

	void initialize_gui();
	void ok();
	void close_window();
	void closeEvent(QCloseEvent* event);
};


#endif