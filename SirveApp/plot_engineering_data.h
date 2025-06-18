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
#include <QDebug>
#include <QPair>
#include <QString>
#include <vector>

#include <armadillo>

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

    EngineeringPlot(std::vector<Frame> const &osm_frames, QString plotTitle, std::vector<Quantity> quantities);
    ~EngineeringPlot();

    using JKQTPlotter::zoomIn; // Keep access to the base class version

    QChartView *chart_view;
    ColorScheme colors;
    JKQTPDatastore* ds;

    QString x_title, y_title, plotTitle;

    std::vector<double> past_midnight, past_epoch;
    std::vector<double> sensor_i_fov_x, sensor_i_fov_y;
    std::vector<double> boresight_az, boresight_el;

    Enums::PlotType plotYType = Enums::PlotType::Undefined_PlotType;
    Enums::PlotType plotXType = Enums::PlotType::Undefined_PlotType;

    void copyStateFrom(EngineeringPlot &other);

    const QVector<double>& getColumn(size_t index) const;

    void AddGraph(int track_id, size_t &columnX, size_t&columnY);
    void AddTrack(std::vector<double> x, std::vector<double> y, int track_id, size_t &columnX, size_t &columnY);
    void DefineFullPlotInterval();
    void DefinePlotSubInterval(int min, int max);
    void DeleteAllTrackGraphs();
    void DeleteGraphIfExists(const QString &titleToFind);
    std::vector<size_t>& DeleteTrack(int track_id);
    FuncType DeriveFunctionPointers(Enums::PlotType type);

    void DisableDataScopeButton(bool value);

    void PlotChart();
    void PlotCurrentFrameline(int frameline_x);
    void PlotSirveTracks(int override_track_id);

    void RecolorManualTrack(int track_id, QColor new_color);
    void RecolorOsmTrack(QColor new_color);
    void RecordYAxisMinMax();
    void ReplaceTrack(std::vector<double> x, std::vector<double> y, int track_id);
    void RestoreTrackGraphs(std::vector<size_t> &new_column_indexes);

    void SetPlotterXAxisMinMax(int min, int max);
    void SetPlotterYAxisMinMax(int min, int max);
    void SetupSubRange(int min, int max);

    void UpdateManualPlottingTrackFrames(std::vector<ManualPlottingTrackFrame> frames, const std::set<int>& track_ids);

    JKQTPDatastore* get_data_store() const {
        return ds;
    }

    std::vector<Quantity> get_my_quantities() const {
        return my_quantities;
    }

    std::vector<Quantity> get_params();
    QString get_plot_title() const;
    int get_palette_tab_index();
    int get_subinterval_min() const;
    int get_subinterval_max() const;

    int get_index_full_scope_xmin() const;
    int get_index_full_scope_xmax() const;
    int get_index_zoom_min();
    int get_index_zoom_max();
    double get_max_x_axis_value();
    double get_min_x_axis_value();
    bool get_plot_primary_only();
    bool get_show_full_scope();
    double get_single_x_axis_value(int x_index);
    Enums::PlotUnit get_quantity_unit_by_axis(int axis_index);

    std::vector<double> get_x_axis_values(unsigned int start_idx, unsigned int end_idx);

    void set_plot_primary_only(bool value);
    void set_plotting_track_frames(std::vector<PlottingTrackFrame> frames, int num_unique);
    void set_pre_image(double, double);
    void set_sub_plot_xmin(int value);
    void set_sub_plot_xmax(int value);

    void set_show_full_scope(bool use_subinterval);
    void set_data_scope_icon(QString type);
    void set_graph_style_icon(QString type);

    void print_ds(JKQTPDatastore *_ds);

signals:
    void changeMotionStatus(bool status);
    void frameNumberChanged(uint32_t frame_number);
    void updatePlots();

public slots:

    void HandlePlayerButtonClick();
    void SetPlotClassification(QString input_title);

public Q_SLOTS:
    void DoCustomZoomIn();
    void RotateGraphStyle();
    void ToggleDataScope();
    void ToggleFrameLine();
    void ToggleGraphTickSymbol();

private slots:
    void onJPContextActionTriggered(const QString& actionName);

private:

    // Parameters to display subplot
    bool plot_all_data, plot_primary_only;
    int index_full_scope_xmin, index_full_scope_xmax;
    int partial_scope_original_min_x = 0, partial_scope_original_max_x;

    QAction* actToggleFrameLine;
    QAction* actToggleDataScope;
    QAction* actRotateGraphStyle;
    double fixed_max_y, sub_max_y;
    size_t frameLineColumnX;
    JKQTPXYLineGraph* graph;
    int graph_style = 0;
    std::vector<Quantity> my_quantities;
    std::map<int, QColor> manual_track_colors;
    std::vector<ManualPlottingTrackFrame> manual_track_frames;
    std::set<int> manual_track_ids;
    unsigned int num_frames;
    int number_of_tracks;
    const std::vector<Frame> *osm_frames_ref;
    int palette_tab_index;
    QString plot_classification;
    bool show_frame_line;
    double snap_x = 0;
    QTabWidget* tabWidget;

    std::vector<PlottingTrackFrame> track_frames;
    Enums::PlotType x_axis_units = Enums::PlotType::Undefined_PlotType;

    void EditPlotText();
    QToolButton *FindToolButtonForAction(QToolBar *toolbar, QAction *action);
    void InitializeFrameLine(double x_intercept);
    void LookupTrackColumnIndexes(int track_id, size_t &columnX, size_t &columnY);
    void PlotSirveQuantities(std::function<std::vector<double>(size_t)> get_x_func, std::function<std::vector<double>(size_t)> get_y_func, size_t plot_number_tracks, QString title);
    bool TrackExists(int track_id) const;

    QAction* get_action_rotate_graphstyle() const;
    QAction* get_action_toggle_frameline() const;
    QAction* get_action_toggle_datascope() const;

    std::vector<double> get_individual_x_track(size_t i);
    std::vector<double> get_individual_y_track_irradiance(size_t i);
    std::vector<double> get_individual_y_track_sum_relative_counts(size_t i);
    std::vector<double> get_individual_y_track_azimuth(size_t i);
    std::vector<double> get_individual_y_track_boresight_azimuth(size_t i);
    std::vector<double> get_individual_y_track_boresight_elevation(size_t i);
    std::vector<double> get_individual_y_track_elevation(size_t i);
    std::vector<double> get_individual_y_track_fov_x(size_t i);
    std::vector<double> get_individual_y_track_fov_y(size_t i);

    std::vector<double> x_osm_values;
    std::vector<double> y_osm_values;

protected:
    void mousePressEvent(QMouseEvent *event);
};

#endif
