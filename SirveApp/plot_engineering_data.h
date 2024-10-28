#pragma once

#ifndef ENGINEERING_PLOT_H
#define ENGINEERING_PLOT_H

#include <armadillo>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLegendMarker>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/qxyseries.h>
#include <qfiledialog.h>
#include <QPair>
#include <QString>
#include <vector>

#include "color_scheme.h"
#include "data_structures.h"
#include "new_chart_view.h"
#include "tracks.h"

class QtPlotting : public QWidget
{
public:
    QChart *chart;
    NewChartView *chart_view;
    ColorScheme colors;
    QColor osm_track_color;

    QtPlotting();
    ~QtPlotting();

    QValueAxis *axis_x, *axis_y;
    QLogValueAxis *axis_ylog;
    QString x_title, y_title, title;

    bool yaxis_is_log, yaxis_is_scientific, xaxis_is_fixed_pt;

    void StartNewChart();
    void AddSeries(QXYSeries *series, std::vector<double> x, std::vector<double> y, bool broken_data = false);
    void AddSeriesWithColor(std::vector<double> x, std::vector<double> y, QColor color);
    void RemoveSeriesLegend();
    void DefineChartProperties(double min_x, double max_x, double min_y, double max_y);
    void SavePlot();

    double FindTickSpacing(double value, int min_number_ticks, int max_number_ticks);
    double FindMaxForAxis(std::vector<double> data);

    void set_axis_limits(QAbstractAxis *axis, double min_x, double max_x);
    void set_xaxis_limits(double min_x, double max_x);
    void set_yaxis_limits(double min_y, double max_y);
};

enum XAxisPlotVariables{frames , seconds_past_midnight, seconds_from_epoch};

class EngineeringPlots : public QtPlotting
{
    Q_OBJECT
public:
    EngineeringPlots(std::vector<Frame> const &osm_frames);
    ~EngineeringPlots();

    // Parameters to display subplot
    bool plot_all_data, plot_primary_only, plot_current_marker;
    double full_plot_xmin, full_plot_xmax, sub_plot_xmin, sub_plot_xmax;
    unsigned int index_sub_plot_xmin, index_sub_plot_xmax, index_zoom_min, index_zoom_max, current_unit_id, current_chart_id;

    std::vector<double> past_midnight, past_epoch;
    std::vector<double> sensor_i_fov_x, sensor_i_fov_y;
    std::vector<double> boresight_az, boresight_el;

    // plot axes titles
    QXYSeries *current_frame_marker;

    std::vector<PlottingFrameData> engineering_data;

    void SetXAxisChartId(int xaxis_chart_id);
    void SetYAxisChartId(int yaxis_chart_id);

    void InitializeIntervals(std::vector<Frame> const &osm_frames);
    void PlotChart(bool yAxisChangedLocal);
    void UpdateManualPlottingTrackFrames(std::vector<ManualPlottingTrackFrame> frames, std::set<int> track_ids);
    void RecolorManualTrack(int track_id, QColor new_color);
    void RecolorOsmTrack(QColor new_color);

    void toggle_yaxis_log(bool input);
    void toggle_yaxis_scientific(bool input);
    void toggle_xaxis_fixed_pt(bool input);

    void set_xaxis_units(XAxisPlotVariables unit_choice);
    void set_plotting_track_frames(std::vector<PlottingTrackFrame> frames, int num_unique);

    QPair<qreal, qreal> chart_x_intervals[3]; // popupalate these later

    float chart_y_maxes[7] = {-1, 360.0, 90.0, 750.0, 750.0, 360.0, 90.0}; // the -1 entry for irradiance gets populated later...
    ChartState chart_states[7];

    bool yAxisChanged = false;

signals:
    void changeMotionStatus(bool status);
    void updatePlots();

public slots:

    void ToggleSubplot();
    void PlotCurrentStep(int counter);
    void SetPlotTitle(QString input_title);
    void HandlePlayerButtonClick();

private:

    int number_of_tracks;
    std::vector<PlottingTrackFrame> track_frames;
    std::set<int> manual_track_ids;
    std::vector<ManualPlottingTrackFrame> manual_track_frames;
    std::map<int, QColor> manual_track_colors;

    unsigned int num_frames;
    XAxisPlotVariables x_axis_units;

    float fixed_max_y;

    void CreateCurrentMarker();
    void DrawTitle();
    void EstablishPlotLimits();

    void PlotAzimuth(size_t plot_number_tracks);
    void PlotElevation(size_t plot_number_tracks);
    void PlotIrradiance(size_t plot_number_tracks);
    void PlotFovX();
    void PlotFovY();
    void PlotBoresightAzimuth();
    void PlotBoresightElevation();

    std::vector<double> get_individual_x_track(size_t i);
    std::vector<double> get_individual_y_track_irradiance(size_t i);
    std::vector<double> get_individual_y_track_azimuth(size_t i);
    std::vector<double> get_individual_y_track_elevation(size_t i);
    std::vector<double> get_x_axis_values(unsigned int start_idx, unsigned int end_idx);
    double get_single_x_axis_value(int x_index);
    double get_max_x_axis_value();
};

#endif
