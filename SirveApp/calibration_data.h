#pragma once
#ifndef CALIBRATION_DATA_H
#define CALIBRATION_DATA_H

#include <iostream>
#include <fstream>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <string>
#include <math.h>

#include <armadillo>

#include "logging.h"
#include "fixed_aspect_ratio_frame.h"
#include "abpnuc_reader.h"
#include "clickable_chartview.h"
#include "color_scheme.h"
#include "process_file.h"
#include "osm_reader.h"
#include "Data_Structures.h"
#include "support/qthelpers.h"

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
#include <qinputdialog.h>

#include <qgridlayout.h>

struct ImportFrames {

	int start_frame1, start_frame2, stop_frame1, stop_frame2;

	bool all_frames_found;
};


struct SelectedData {

	bool valid_data;
	double temperature_mean, temperature_std;
	int num_frames, initial_frame, id;
	double start_time, stop_time;
	double calculated_irradiance;

	QString color;
	QList<QPointF> points;
};


class CalibrationData {

public:
	CalibrationData();
	~CalibrationData();

	std::vector<double> measure_irradiance(int ul_row, int ul_col, int lr_row, int lr_col, arma::mat x, double frame_integration_time);
	void setup_model(arma::mat input_m, arma::mat input_b);
	bool set_calibration_details(QString path_to_nuc, QString path_to_image, SelectedData selection1, SelectedData selection2, double int_time);
	bool check_path(QString path);

	bool calibration_available;
	QString path_nuc, path_image;
	SelectedData user_selection1, user_selection2;
	double integration_time;

private:

	arma::mat m, b;
};


class CalibrationDialog : public QDialog
{
	Q_OBJECT


public:

	CalibrationDialog(CalibrationData & input_model, QWidget* parent = nullptr);
	~CalibrationDialog();

	CalibrationData model;

	public slots:
		void point_selected(double x0, double x1);

private:

	ColorScheme colors;
	QList<QPointF> temperature;
	std::vector<double>all_frame_times, vector_wavelength, vector_filter;

	SelectedData user_selection1, user_selection2;

	QString path_nuc, path_image;

	QVBoxLayout* mainLayout;
	QChart* chart_temperature;
	Clickable_QChartView* chart_view_temperatures;
	QPushButton* btn_get_nuc_file, *btn_ok, *btn_cancel;
	QLabel* lbl_nuc_filename;
	FixedAspectRatioFrame* frame_plot;
	QRadioButton *radio_temperature1, *radio_temperature2;
	QLineSeries* selection1, * selection2;

	Process_File file_processor;
	OSMReader osm_reader;
	std::vector<Frame> osm_frames;
	AbpFileMetadata abp_metadata;

	void initialize_gui();
	void get_new_nuc_file();
	void import_nuc_file();
	void get_plotting_data(ABPNUC_Data &nuc_data);
	void create_temperature_plot(QList<QPointF> temperature);
	
	void show_user_selection(SelectedData &user_selection, double x0, double x1);
	void draw_series(SelectedData& data);
	void update_user_selection_labels(SelectedData &data);
	void draw_axes();
	bool check_path(QString path);
	ImportFrames find_frames_in_osm();
	double calculate_black_body_radiance(double wavelength, double temperature);
	arma::mat average_multiple_frames(std::vector<std::vector<uint16_t>>& frames);

	bool check_configuration_values();
	bool check_filter_file(QString path);
	arma::vec get_total_filter_response();
	double trapezoidal_integration(arma::vec x, arma::vec y);


	arma::vec plank_equation(double temperature);

	void ok();
	void close_window();
	void closeEvent(QCloseEvent* event);
};

#endif