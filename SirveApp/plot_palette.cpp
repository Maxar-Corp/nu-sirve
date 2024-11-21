#include "plot_palette.h"
#include "plot_engineering_data.h"


PlotPalette::PlotPalette(QWidget *parent)
    : QTabWidget(parent) {
}

void PlotPalette::addPlotTab(EngineeringPlot *engineering_plot, QString title) {

    QWidget *tab1 = new QWidget(this);
    QVBoxLayout *layout1 = new QVBoxLayout(tab1);
    layout1->addWidget(engineering_plot);

    // Add tabs to the QTabWidget
    this->QTabWidget::addTab(tab1, title);
}
