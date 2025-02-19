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
#include "enums.h"
#include "tracks.h"
#include "quantity.h"

#include "jkqtplotter/jkqtplotter.h"
#include "jkqtplotter/graphs/jkqtplines.h"

using FuncType = std::function<std::vector<double>(size_t)>;

class EngineeringPlot : public JKQTPlotter
{
    Q_OBJECT
public:

    EngineeringPlot(std::vector<Frame> *osm_frames, QString plotTitle, std::vector<Quantity> quantities);
    ~EngineeringPlot();

    void AddTransparentLabel();

    QChartView *chart_view;
    ColorScheme colors;
    JKQTPDatastore* ds;

    QString x_title, y_title, plotTitle;

    // Parameters to display subplot
    bool plot_all_data, plot_primary_only, plot_current_marker;
    double full_plot_xmin, full_plot_xmax, sub_plot_xmin, sub_plot_xmax;
    unsigned int index_sub_plot_xmin, index_sub_plot_xmax, index_zoom_min, index_zoom_max;

    std::vector<double> past_midnight, past_epoch;
    std::vector<double> sensor_i_fov_x, sensor_i_fov_y;
    std::vector<double> boresight_az, boresight_el;

    JKQTPXYLineGraph* graph;
    Enums::PlotType plotYType;
    Enums::PlotType plotXType;

    void copyStateFrom(EngineeringPlot &other);

    const QVector<double>& getColumn(size_t index) const;

    void AddSeriesWithColor(std::vector<double> x, std::vector<double> y, int track_id);
    void DefineFullPlotInterval();
    void DefinePlotSubInterval(int min, int max);
    void DeleteGraphIfExists(const QString& titleToFind);

    void PlotChart();
    void PlotSirveTracks();
    void UpdateManualPlottingTrackFrames(std::vector<ManualPlottingTrackFrame> frames, std::set<int> track_ids);
    void RecolorManualTrack(int track_id, QColor new_color);
    void RecolorOsmTrack(QColor new_color);
    void SetPlotterXAxisMinMax(int min, int max);
    void ToggleUseSubInterval();

    JKQTPDatastore* get_data_store() const {
        return ds;
    }

    std::vector<Quantity> get_my_quantities() const {
        return my_quantities;
    }

    std::vector<Quantity> get_params();
    QString get_plot_title();
    int get_palette_tab_index();
    int get_subinterval_min();
    int get_subinterval_max();

    void set_plotting_track_frames(std::vector<PlottingTrackFrame> frames, int num_unique);
    void set_palette_index();

    bool get_use_subinterval();
    void set_use_subinterval(bool use_subinterval);

    FuncType DeriveFunctionPointers(Enums::PlotType type);

signals:
    void changeMotionStatus(bool status);
    void updatePlots();

public slots:

    void HandlePlayerButtonClick();
    void PlotCurrentFrameline(int frameline_x);
    void SetPlotClassification(QString input_title);

public Q_SLOTS:
    void ToggleFrameLine();

private:

    QAction* actToggleFrameLine;
    double fixed_max_y;
    size_t frameLineColumnX;
    std::vector<Quantity> my_quantities;
    std::map<int, QColor> manual_track_colors;
    std::vector<ManualPlottingTrackFrame> manual_track_frames;
    std::set<int> manual_track_ids;
    unsigned int num_frames;
    int number_of_tracks;
    std::vector<Frame> *osm_frames_ref;
    int palette_tab_index;
    QString plot_classification;
    bool show_frame_line;
    QTabWidget* tabWidget;
    bool use_subinterval = false;
    std::vector<PlottingTrackFrame> track_frames;
    Enums::PlotType x_axis_units;

    void EditPlotText();
    void InitializeFrameLine(double x_intercept);
    void PlotSirveQuantities(std::function<std::vector<double>(size_t)> get_x_func, std::function<std::vector<double>(size_t)> get_y_func, size_t plot_number_tracks, QString title);

    QAction* get_action_toggle_frameline() const;
    std::vector<double> get_individual_x_track(size_t i);
    std::vector<double> get_individual_y_track_irradiance(size_t i);
    std::vector<double> get_individual_y_track_azimuth(size_t i);
    std::vector<double> get_individual_y_track_boresight_azimuth(size_t i);
    std::vector<double> get_individual_y_track_boresight_elevation(size_t i);
    std::vector<double> get_individual_y_track_elevation(size_t i);
    std::vector<double> get_individual_y_track_fov_x(size_t i);
    std::vector<double> get_individual_y_track_fov_y(size_t i);
    double get_max_x_axis_value();
    double get_single_x_axis_value(int x_index);
    std::vector<double> get_x_axis_values(unsigned int start_idx, unsigned int end_idx);
};

#endif
