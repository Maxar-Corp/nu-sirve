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
#include "clickable_chartview.h"
#include "color_scheme.h"

#include <qfiledialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qdialog.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include <QtCharts/QChartView>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QList>
#include <QPointF>

#include <qgridlayout.h>


struct SelectedData {

	bool valid_data;
	double temperature_mean, temperature_std;
	int num_frames, initial_frame;
	double calculated_irradiance;

	QString color;

	QLineSeries *series;
};


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

	public slots:
		void point_selected(double x0, double x1);

private:

	
	ColorScheme colors;
	QList<QPointF> temperature;
	SelectedData user_selection1, user_selection2;

	QVBoxLayout* mainLayout;
	QChart* chart_temperature;
	Clickable_QChartView* chart_view_temperatures;
	QPushButton* btn_get_nuc_file, *btn_ok, *btn_cancel;
	QLabel* lbl_nuc_filename;
	FixedAspectRatioFrame* frame_plot;
	QRadioButton *radio_temperature1, *radio_temperature2;

	void initialize_gui();
	void import_nuc_file();
	void get_plotting_data(ABPNUC_Data &nuc_data);
	void create_temperature_plot(QList<QPointF> temperature);
	void show_user_selection(SelectedData &user_selection, double x0, double x1);
	void draw_axes();

	void ok();
	void close_window();
	void closeEvent(QCloseEvent* event);
};

#endif