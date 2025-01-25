#include "plot_palette.h"
#include "plot_engineering_data.h"
#include "qcheckbox.h"
#include "qlineedit.h"
#include "qmenu.h"
#include "quantity.h"
#include "qwidgetaction.h"

#include <map>


PlotPalette::PlotPalette(QWidget *parent) : QTabWidget(parent)
{
    quantities = {"Azimuth", "Boresight_Azimuth", "Boresight_Elevation", "Elevation", "Frames", "Irradiance", "Seconds_From_Epoch", "Seconds_Past_Midnight"};

    // Enable custom context menu on the QTabBar
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), &QTabBar::customContextMenuRequested,
            this, &PlotPalette::HandleTabRightClicked);
}

EngineeringPlot *PlotPalette::GetEngineeringPlotReference(int tab_id)
{
    return engineering_plot_ref.at(tab_id);
}

void PlotPalette::UpdatePlotLabel(int tab_id, QString label)
{
    engineering_plot_ref.at(tab_id)->getPlotter()->setPlotLabel(label);
    engineering_plot_ref.at(tab_id)->getPlotter()->redrawPlot();
}

void PlotPalette::AddPlotTab(EngineeringPlot *engineering_plot, std::vector<Quantity> quantities)
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->addWidget(engineering_plot);

    int plot_type_id = Enums::getPlotTypeIndexFromString(quantities[0].getName());
    Enums::PlotUnit plot_unit = quantities[0].getUnit();

    qDebug() << "plot_unit=" << Enums::plotUnitToString(plot_unit);

    int palette_tab_id = this->tabBar()->count();

    // store the id and unit of the plot in a map for lookups later when managing plots
    tab_to_type[palette_tab_id] = plot_type_id;
    tab_to_unit[palette_tab_id] = plot_unit;

    this->QTabWidget::addTab(tab, quantities[0].getName());

    engineering_plot_ref.push_back(engineering_plot);
}

Enums::PlotType PlotPalette::GetPlotTypeByTabId(int tab_id)
{
    return Enums::getPlotTypeByIndex(tab_to_type[tab_id]);
}

Enums::PlotUnit PlotPalette::GetPlotUnitByTabId(int tab_id)
{
    return Enums::getPlotUnitByIndex(tab_to_unit[tab_id]);
}

void PlotPalette::HandleTabRightClicked(const QPoint &pos)
{
    int tabIndex = tabBar()->tabAt(pos);
    if (tabIndex == -1)
        return; // Click was not on a tab

    // Create the context menu
    QMenu contextMenu;
    QCheckBox *syncCheckBox = new QCheckBox("Sync Plot", this);

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

    // What should be a simple use case for syncing plots?  How many can be synced at a given time?  What combos are allowed?

    QAction *closeTabAction = contextMenu.addAction("Hide Tab");
    QAction *editBanner = contextMenu.addAction("Edit Banner");
    QMenu *plotOptions = contextMenu.addMenu("Plot Options ");
    QAction *plotOptions1 = plotOptions->addAction("Linear/Log");

    // Show the menu at the cursor position
    QAction *selectedAction = contextMenu.exec(tabBar()->mapToGlobal(pos));
    if (!selectedAction)
        return;

    // Handle selected actions
    if (selectedAction == closeTabAction) {
        setTabVisible(tabIndex,false);
    } else if (selectedAction == popoutPlotAction) {
        tabBar()->setTabVisible(tabIndex, false);

        qDebug() << "tabIndex = " << tabIndex;
        qDebug() << "PlotUnitByTabId = " << GetPlotUnitByTabId(tabIndex);
        qDebug() << "PlotTypeByTabId = " << GetPlotTypeByTabId(tabIndex);

        std::vector<Quantity> quantities;
        quantities.push_back(Quantity(Enums::plotTypeToString(GetPlotTypeByTabId(tabIndex)), GetPlotUnitByTabId(tabIndex)));
        quantities.push_back(Quantity("Frames", Enums::PlotUnit::None)); // TODO: Implement analogous name/enum transfer here.

        emit popoutPlot(tabIndex, quantities);
    } else if (selectedAction == editBanner) {
        emit editClassification(tabIndex, engineering_plot_ref.at(tabIndex)->getPlotter()->getPlotLabel());
    } else if (selectedAction == plotOptions1) {
        bool yAxisIsLogarithmic = engineering_plot_ref.at(tabIndex)->getPlotter()->getYAxis()->getLogAxis();
        qDebug() << "yAxisIsLogarithmic=" << yAxisIsLogarithmic;
        engineering_plot_ref.at(tabIndex)->getPlotter()->getYAxis()->setLogAxis(!yAxisIsLogarithmic);
        double xScale = engineering_plot_ref.at(tabIndex)->getPlotter()->getAxisAspectRatio();
        engineering_plot_ref.at(tabIndex)->getPlotter()->zoomToFit(true, true, false, false, 0.5, 0.5);
        // TODO: Refine this ^^^ so it zooms to where you left off.
    }
}

void PlotPalette::HandleDesignerParamsSelected(std::vector<Quantity> &quantities)
{
    emit paletteParamsSelected(quantities);
}

void PlotPalette::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QMenu menu(this);
        menu.addAction("Add Tab", [this]() {
            designer = new PlotDesigner(this);
            designer->AddCheckableItemsByIndex(0, quantities);
            designer->AddCheckableItemsByIndex(1, quantities);

            designer->SetDefaultUnits();

            connect(designer, &PlotDesigner::designerParamsSelected, this, &PlotPalette::HandleDesignerParamsSelected);

            designer->exec();
        });
        menu.exec(event->globalPosition().toPoint()); // Show menu at the cursor position
    } else {
        QWidget::mousePressEvent(event); // Pass the event to the base class
    }
}
