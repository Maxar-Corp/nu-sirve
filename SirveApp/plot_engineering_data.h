#pragma once

#include "color_scheme.h"
#include "qmetaobject.h"
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

#include "data_structures.h"
#include "new_chart_view.h"
#include "enums.h"
#include "tracks.h"

#include "jkqtplotter/jkqtplotter.h"
#include "jkqtplotter/graphs/jkqtplines.h"

enum XAxisPlotVariables{frames , seconds_past_midnight, seconds_from_epoch};

class EngineeringPlot : public JKQTPlotter
{
    Q_OBJECT
public:

    EngineeringPlot(std::vector<Frame> const &osm_frames, Enums::PlotType plot_type);
    ~EngineeringPlot();

    JKQTPDatastore* ds;

    QChartView *chart_view;
    ColorScheme colors;

    QString x_title, y_title, title;

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

    JKQTPXYLineGraph* graph;

    void copyStateFrom(const EngineeringPlot &other);

    // Correctly marked as const
    JKQTPDatastore* get_data_store() const {
        return ds;
    }

    // Returns a const reference to the column data
    const QVector<double>& getColumn(size_t index) const;

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

    ChartState chart_states[7];

    bool yAxisChanged = false;

    Enums::PlotType plotType;

signals:
    void changeMotionStatus(bool status);
    void updatePlots();

public slots:

    void ToggleSubplot();
    void PlotCurrentFrameline(int counter);
    void SetPlotTitle(QString input_title);
    void HandlePlayerButtonClick();

private:

    QTabWidget* tabWidget;

    int number_of_tracks;
    std::vector<PlottingTrackFrame> track_frames;
    std::set<int> manual_track_ids;
    std::vector<ManualPlottingTrackFrame> manual_track_frames;
    std::map<int, QColor> manual_track_colors;

    unsigned int num_frames;
    XAxisPlotVariables x_axis_units;

    double fixed_max_y;

    void CreateCurrentMarker(double x_intercept);
    void DrawTitle();
    void EstablishPlotLimits();

    void PlotAzimuth(size_t plot_number_tracks);
    void PlotElevation(size_t plot_number_tracks);

    void PlotSirveQuantity(std::function<std::vector<double>(size_t)> get_y_track_func, size_t plot_number_tracks, QString title);
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
