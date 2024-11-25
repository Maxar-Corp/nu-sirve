#include "plot_palette.h"
#include "plot_engineering_data.h"
#include "qlineedit.h"
#include "qmenu.h"
#include "SirveApp.h"


PlotPalette::PlotPalette(QWidget *parent)
    : QTabWidget(parent) {

    // Enable custom context menu on the QTabBar
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), &QTabBar::customContextMenuRequested,
            this, &PlotPalette::HandleTabRightClicked);
}

void PlotPalette::addPlotTab(EngineeringPlot *engineering_plot, QString title) {

    QWidget *tab1 = new QWidget(this);
    QVBoxLayout *layout1 = new QVBoxLayout(tab1);
    layout1->addWidget(engineering_plot);

    // Add tabs to the QTabWidget
    this->QTabWidget::addTab(tab1, title);
}

void PlotPalette::HandleTabRightClicked(const QPoint &pos) {
    int tabIndex = tabBar()->tabAt(pos);
    if (tabIndex == -1)
        return; // Click was not on a tab

    // Create the context menu
    QMenu contextMenu;
    QAction *closeTabAction = contextMenu.addAction("Close Tab");
    QAction *popoutPlotAction = contextMenu.addAction("Popout Plot");

    // Show the menu at the cursor position
    QAction *selectedAction = contextMenu.exec(tabBar()->mapToGlobal(pos));
    if (!selectedAction)
        return;

    // Handle selected actions
    if (selectedAction == closeTabAction) {
        removeTab(tabIndex);
    } else if (selectedAction == popoutPlotAction) {
        tabBar()->setTabVisible(tabIndex, false);
        emit popoutPlot(tabIndex);
    }
}
