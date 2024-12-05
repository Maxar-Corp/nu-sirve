#ifndef PLOT_PALETTE_H
#define PLOT_PALETTE_H

#include "plot_designer.h"
#include "plot_engineering_data.h"
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QList>

class PlotPalette : public QTabWidget {
    Q_OBJECT

public:
    explicit PlotPalette(QWidget *parent = nullptr);
    void addPlotTab(EngineeringPlot *engineering_plot, std::vector<QString> params);

    EngineeringPlot* get_plot(int index);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    void addTab();

    PlotDesigner *designer;
    QList<EngineeringPlot*> plotPointers;
    QStringList quantities;

signals:
    void popoutPlot(std::vector<QString> params);
    void popinPlot(int plotType);
    void paletteParamsSelected(const std::vector<QString> &strings);

private slots:
    void HandleTabRightClicked(const QPoint &pos);

public slots:
    void HandleDesignerParamsSelected(const std::vector<QString> &strings);

};

#endif // PLOT_PALETTE_H
