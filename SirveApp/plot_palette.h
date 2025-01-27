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
    EngineeringPlot *GetEngineeringPlotReference(int tab_id);
    Enums::PlotType GetPlotTypeByTabId(int tab_id);
    Enums::PlotUnit GetPlotUnitByTabId(int tab_id);
    int GetUnitTypeByTabId(int tab_id);
    void PlotSirveTracks(int plot_id, Enums::PlotType plot_type);
    void RecolorManualTrack(int plot_id, int track_id, QColor new_color);
    void UpdateManualPlottingTrackFrames(int plot_id, std::vector<ManualPlottingTrackFrame> frames, std::set<int> track_ids);
    void UpdatePlotLabel(int tab_id, QString label);
    void RouteFramelineUpdate(int frameline_x);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    void addTab();

    PlotDesigner *designer;
    QStringList quantities;
    std::map<int, int> tab_to_type;
    std::map<int, int> tab_to_unit;
    std::vector<EngineeringPlot*> engineering_plot_ref;

signals:
    void editClassification(int tab_index, QString current_value);
    void popoutPlot(int tab_index, QString plotTitle, std::vector<Quantity> &quantities);
    void popinPlot(int plotType);
    void paletteParamsSelected(QString plotTitle, std::vector<Quantity> &quantities);

private slots:
    void HandleTabRightClicked(const QPoint &pos);

public slots:
    void HandleDesignerParamsSelected(QString plotTitle, std::vector<Quantity> &quantities);

};

#endif // PLOT_PALETTE_H
