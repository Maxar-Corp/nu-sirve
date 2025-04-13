#ifndef CALIBRATION_DATA_H
#define CALIBRATION_DATA_H

#include <armadillo>
#include <array>

#include <qdialog.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qradiobutton.h>

#include <QList>
#include <QPointF>
#include <QVBoxLayout>
#include <QtCharts/QLineSeries>

#include "abpnuc_reader.h"
#include "abir_reader.h"
#include "clickable_chartview.h"
#include "color_scheme.h"
#include "data_structures.h"
#include "osm_reader.h"
#include "process_file.h"

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
	CalibrationData() = default;

	std::array<double, 3> MeasureIrradiance(int ul_row, int ul_col, int lr_row, int lr_col, arma::mat x, double frame_integration_time) const;
	void setup_model(arma::mat input_m, arma::mat input_b);
	bool set_calibration_details(QString path_to_nuc, QString path_to_image, SelectedData selection1, SelectedData selection2, double int_time);
	bool check_path(QString path);

	bool calibration_available = false;
	QString path_nuc, path_image;
	SelectedData user_selection1, user_selection2;
	double integration_time = 0.0;

private:
	arma::mat m, b;
};

class CalibrationDialog : public QDialog
{
	Q_OBJECT
public:

	CalibrationDialog(CalibrationData & input_model, ABPFileType file_type);
	~CalibrationDialog();

	CalibrationData model;

	public slots:
        void PointSelected(double x0, double x1);

private:
    ABPFileType file_type;
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
    void PrepareAndPlotTemperature(const ABPNUCFrames &nuc_data);
    void CreateTemperaturePlot(QList<QPointF> temperature);
	
    void ShowUserSelection(SelectedData &user_selection, double x0, double x1);
    void DrawSeries(SelectedData& data);
    void UpdateUserSelectionLabels(SelectedData &data);
    void DrawAxes();

    ImportFrames FindOsmFrames();
    arma::mat AverageMultipleFrames(std::vector<std::vector<uint16_t>>& frames);

    bool CheckPath(QString path);
    bool CheckConfigurationValues();
    bool CheckFilterFile(QString path);

    arma::vec CalculateTotalFilterResponse();
    double TrapezoidalIntegration(arma::vec x, arma::vec Y);
    arma::vec CalculatePlanckEquation(double temperature);

    void verifyCalibrationValues();
    void closeWindow();
    void closeEvent(QCloseEvent* event);
};

#endif
