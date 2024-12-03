#ifndef PLOT_PALETTE_H
#define PLOT_PALETTE_H

#include "plot_designer.h"
#include "plot_engineering_data.h"
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

class PlotPalette : public QTabWidget {
    Q_OBJECT

public:
    explicit PlotPalette(QWidget *parent = nullptr);
    void addPlotTab(EngineeringPlot *engineering_plot, QString title);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    void addTab();

    PlotDesigner *designer;

signals:
    void popoutPlot(int plotType);
    void popinPlot(int plotType);

private slots:
    void HandleTabRightClicked(const QPoint &pos);
};

#endif // PLOT_PALETTE_H
