#ifndef PLOT_PALETTE_H
#define PLOT_PALETTE_H

#include "plot_designer.h"
#include "plot_engineering_data.h"
#include "quantity.h"
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QList>

class PlotPalette : public QTabWidget {
    Q_OBJECT

public:
    explicit PlotPalette(QWidget *parent = nullptr);

    void AddPlotTab(EngineeringPlot *engineering_plot, std::vector<Quantity> quantities);
    void AddPoppedTabIndex(int tab_index);
    void AddSyncedTabIndex(int tab_index);
    void DeleteAllTrackGraphs(int plot_id);

    EngineeringPlot *GetEngineeringPlotReference(int tab_id);
    Enums::PlotType GetPlotTypeByTabId(int tab_id);
    Enums::PlotUnit GetPlotUnitByTabId(int tab_id);

    int GetLowestSyncedTabIndex();
    bool HasSyncedTabWithIndex(int tab_id);
    void PlotAllSirveTracks(int override_track_id);
    void RecolorManualTrack(int plot_id, int track_id, QColor new_color);
    void RedrawPlot(int plot_id);
    void RemovePoppedTabIndex(int tab_index);
    void RemoveSyncedTabIndex(int tab_index);
    void RouteFramelineUpdate(int frameline_x);
    void SetAbirDataLoaded(bool abir_data_loaded);
    void UpdateManualPlottingTrackFrames(int plot_id, std::vector<ManualPlottingTrackFrame> frames, const std::set<int>& track_ids);
    void UpdatePlotLabel(int tab_id, const QString& label);
    void UpdateAllManualPlottingTrackFrames(const std::vector<ManualPlottingTrackFrame>& frames, const std::set<int>& track_ids);

protected:
    void mouseDoubleClickEvent (QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    bool abir_data_loaded = false;
    PlotDesigner *designer;
    std::vector<EngineeringPlot*> engineering_plot_ref;
    std::vector<int> popped_tabs;
    std::vector<int> synced_tabs;
    QStringList quantities;
    std::map<int, int> tab_to_type;
    std::map<int, int> tab_to_unit;
    QMap<int, QMenu*> tab_menus;

signals:
    void editClassification(int tab_index, QString current_value);
    void paletteParamsSelected(QString plotTitle, std::vector<Quantity> &quantities);
    void plotFocusChanged(int tab_index);

    void popinPlot(int plotType);
    void popoutPlot(int tab_index, QString plotTitle, std::vector<Quantity> &quantities);
    void toggleUseSubInterval();

public slots:
    void HandleDesignerParamsSelected(QString plotTitle, std::vector<Quantity> &quantities);

private slots:
    void HandleTabRightClicked(const QPoint &pos);
    void OnCheckboxToggled(bool checked, int tabIndex);
};

#endif // PLOT_PALETTE_H
