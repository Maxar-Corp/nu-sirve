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

class EngineeringPlot : public JKQTPlotter
{
    Q_OBJECT
public:

    EngineeringPlot(std::vector<Frame> const &osm_frames, std::vector<QString> params);
    ~EngineeringPlot();

    QChartView *chart_view;
    ColorScheme colors;
    JKQTPDatastore* ds;

    QString x_title, y_title, title;

    // Parameters to display subplot
    bool plot_all_data, plot_primary_only, plot_current_marker;
    double full_plot_xmin, full_plot_xmax, sub_plot_xmin, sub_plot_xmax;
    unsigned int index_sub_plot_xmin, index_sub_plot_xmax, index_zoom_min, index_zoom_max;

    std::vector<double> past_midnight, past_epoch;
    std::vector<double> sensor_i_fov_x, sensor_i_fov_y;
    std::vector<double> boresight_az, boresight_el;

    JKQTPXYLineGraph* graph;
    Enums::PlotType plotType;
    Enums::PlotUnit plotUnit;

    void copyStateFrom(const EngineeringPlot &other);

    // Correctly marked as const
    JKQTPDatastore* get_data_store() const {
        return ds;
    }

    // Returns a const reference to the column data
    const QVector<double>& getColumn(size_t index) const;

    //void AddSeries(QLineSeries *trackSeries);

    void AddSeriesWithColor(std::vector<double> x, std::vector<double> y, int track_id);
    bool HasGraphWithTitle(const QString& titleToFind);

    void PlotChart();
    void PlotSirveTracks();
    void UpdateManualPlottingTrackFrames(std::vector<ManualPlottingTrackFrame> frames, std::set<int> track_ids);
    void RecolorManualTrack(int track_id, QColor new_color);
    void RecolorOsmTrack(QColor new_color);

    void toggle_yaxis_log(bool input);
    void toggle_yaxis_scientific(bool input);
    void toggle_xaxis_fixed_pt(bool input);

    void set_xaxis_units(Enums::PlotUnit unit_choice);
    void set_plotting_track_frames(std::vector<PlottingTrackFrame> frames, int num_unique);
    std::vector<QString> get_params();

signals:
    void changeMotionStatus(bool status);
    void updatePlots();

public slots:

    void HandlePlayerButtonClick();
    void PlotCurrentFrameline(int frameline_x);
    void SetPlotTitle(QString input_title);

private:

    double fixed_max_y;
    size_t frameLineColumnX;
    std::vector<QString> my_params;
    std::map<int, QColor> manual_track_colors;
    std::vector<ManualPlottingTrackFrame> manual_track_frames;
    std::set<int> manual_track_ids;
    unsigned int num_frames;
    int number_of_tracks;
    std::vector<PlottingTrackFrame> track_frames;
    Enums::PlotUnit x_axis_units;
    QTabWidget* tabWidget;

    void DrawTitle();
    void InitializeFrameLine(double x_intercept);

    void PlotAzimuth(size_t plot_number_tracks);
    void PlotBoresightAzimuth();
    void PlotBoresightElevation();
    void PlotElevation(size_t plot_number_tracks);

    void PlotFovX();
    void PlotFovY();
    void PlotSirveQuantities(std::function<std::vector<double>(size_t)> get_x_func, std::function<std::vector<double>(size_t)> get_y_func, size_t plot_number_tracks, QString title);

    std::vector<double> get_individual_x_track(size_t i);
    std::vector<double> get_individual_y_track_irradiance(size_t i);
    std::vector<double> get_individual_y_track_azimuth(size_t i);
    std::vector<double> get_individual_y_track_elevation(size_t i);
    double get_max_x_axis_value();
    double get_single_x_axis_value(int x_index);
    std::vector<double> get_x_axis_values(unsigned int start_idx, unsigned int end_idx);
};

#endif
