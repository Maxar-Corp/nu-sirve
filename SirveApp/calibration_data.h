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

	std::vector<double> MeasureIrradiance(int ul_row, int ul_col, int lr_row, int lr_col, arma::mat x, double frame_integration_time);
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
    ClickableQChartView* chart_view_temperatures;
	QPushButton* btn_get_nuc_file, *btn_ok, *btn_cancel;
	QLabel* lbl_nuc_filename;
	QFrame* frame_plot;
	QRadioButton *radio_temperature1, *radio_temperature2;
	QLineSeries* selection1, * selection2;

	ProcessFile file_processor;
	OSMReader osm_reader;
	std::vector<Frame> osm_frames;
	AbpFileMetadata abp_metadata;

    void InitializeGui();
    void ResetOrImportNucFile();
    void ImportNucFile();
    void PrepareAndPlotTemperature(ABPNUCData &nuc_data);
    void CreateTemperaturePlot(QList<QPointF> temperature);
	
    void ShowUserSelection(SelectedData &user_selection, double x0, double x1);
    void DrawSeries(SelectedData& data);
    void UpdateUserSelectionLabels(SelectedData &data);
    void DrawAxes();

    ImportFrames FindOsmFrames();
    double CalculateBlackBodyRadiance(double wavelength, double temperature);
    arma::mat AverageMultipleFrames(std::vector<std::vector<uint16_t>>& frames);

    bool CheckPath(QString path);
    bool CheckConfigurationValues();
    bool CheckFilterFile(QString path);

    arma::vec CalculateTotalFilterResponse();
    double CalculateTrapezoidalArea(arma::vec x, arma::vec y);
    arma::vec CalculatePlankEquation(double temperature);

    void verifyCalibrationValues();
    void closeWindow();
    void closeEvent(QCloseEvent* event);
};

#endif
