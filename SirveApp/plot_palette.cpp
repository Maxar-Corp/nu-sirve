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
    QCheckBox *syncCheckBox = new QCheckBox("Sync Plot", this);
    //syncCheckBox->setStyleSheet("QCheckBox::indicator { border: 1px solid gray; width: 15px; height: 15px; background-color: rgba(245,200,125,255); }");

    syncCheckBox->setStyleSheet(
        "QCheckBox {"
        "   font-size: 14px;"            // Set font size
        "   color: black;"               // Set text color
        "   font-weight: bold;"          // Set font weight
        "   background-color: rgba(120, 193, 218, 255);" // Set background Ubuntu blue
        "}"
        "QCheckBox::indicator {"
        "   width: 15px;"                // Set indicator width
        "   height: 15px;"               // Set indicator height
        "   background-color: white;"    // Set checkbox background
        "   border: 2px solid gray;"    // Set border around checkbox
        "}"
        "QCheckBox::indicator:checked {"
        "   background-color:rgb(246, 134, 86)"
        "}"
        "QCheckBox::indicator:unchecked {"
        "   background-color:rgb(200,200,200);"
        "}"
    );

    // Create a QWidgetAction and set the checkbox as the widget
    QWidgetAction *syncAction = new QWidgetAction(this);

    syncAction->setDefaultWidget(syncCheckBox);
    QAction *popoutPlotAction = contextMenu.addAction("Popout Plot");

    syncAction->setCheckable(true);
    syncAction->setChecked(false);
    contextMenu.addAction(syncAction);

    QAction *closeTabAction = contextMenu.addAction("Close Tab");

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

void PlotPalette::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        qDebug() << "Right-click detected at position:" << event->pos();
        // Respond to the right-click here
    } else {
        QWidget::mousePressEvent(event); // Pass the event to the base class
    }

    if (event->button() == Qt::RightButton) {
        QMenu menu(this);
        menu.addAction("Add Tab", [this]() {
            designer = new PlotDesigner(this);
            designer->exec();
        });
        ///menu.addAction("Option 2", []() { qDebug() << "Option 2 selected"; });
        menu.exec(event->globalPos()); // Show menu at the cursor position
    } else {
        QWidget::mousePressEvent(event); // Pass the event to the base class
    }
}
