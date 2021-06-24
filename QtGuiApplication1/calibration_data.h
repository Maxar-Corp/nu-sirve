#pragma once
#ifndef CALIBRATION_DATA_H
#define CALIBRATION_DATA_H

#include <iostream>
#include <string>
#include <math.h>

#include <armadillo>

#include "logging.h"
#include "fixed_aspect_ratio_frame.h"
#include "abpnuc_reader.h"

#include <qfiledialog.h>
#include <qlineedit.h>
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

	QVBoxLayout* mainLayout;

	QPushButton* btn_get_nuc_file, *btn_select_temp1, *btn_select_temp2, * btn_ok, * btn_cancel;
	QLabel* lbl_nuc_filename, *lbl_temp1, *lbl_temp2;
	FixedAspectRatioFrame* frame_plot;
	QLineEdit* txt_additional_frames;

	void initialize_gui();
	void import_nuc_file();

	void ok();
	void close_window();
	void closeEvent(QCloseEvent* event);
};


#endif