#include "plot_palette.h"
#include "plot_engineering_data.h"
#include "qcheckbox.h"
#include "qlineedit.h"
#include "qmenu.h"
#include "quantity.h"
#include "qwidgetaction.h"
#include <map>
#include <QHelpEvent>
#include <QMessageBox>
#include <QTabBar>
#include <QToolTip>

PlotPalette::PlotPalette(QWidget *parent) : QTabWidget(parent)
{
    quantities = {"Azimuth", "Boresight_Azimuth", "Boresight_Elevation", "Elevation", "FovX", "FovY", "Frames", "SumCounts", "Seconds_From_Epoch", "Seconds_Past_Midnight"};

    // Required to detect mouse passing over unused tab bar real estate, to display tooltip:
    setMouseTracking(true);

    // Enable custom context menu on the QTabBar
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), &QTabBar::customContextMenuRequested, this, &PlotPalette::HandleTabRightClicked);
}

void PlotPalette::AddPlotTab(EngineeringPlot *engineering_plot, std::vector<Quantity> quantities)
{
    QWidget *tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->addWidget(engineering_plot);

    QString quant_type_name = quantities.at(0).getName();
    QStringList parts = quant_type_name.split(' ',Qt::SkipEmptyParts);
    if (parts.size() >= 2)
    {
        quant_type_name = parts[1];
    }

    int plot_type_id = Enums::getPlotTypeIndexFromString(quant_type_name);
    Enums::PlotUnit plot_unit = quantities[0].getUnit();

    int palette_tab_id = this->tabBar()->count();

    // store the id and unit of the plot in a map for lookups later when managing plots
    tab_to_type[palette_tab_id] = plot_type_id;
    tab_to_unit[palette_tab_id] = plot_unit;

    this->QTabWidget::addTab(tab, engineering_plot->get_plot_title());

    engineering_plot_ref.push_back(engineering_plot);
}

void PlotPalette::AddPoppedTabIndex(int tab_index)
{
    popped_tabs.push_back(tab_index);
}

void PlotPalette::AddSyncedTabIndex(int tab_index)
{
    synced_tabs.push_back(tab_index);
}

void PlotPalette::DeleteAllTrackGraphs(int plot_id)
{
    engineering_plot_ref.at(plot_id)->DeleteAllTrackGraphs();
}

EngineeringPlot *PlotPalette::GetEngineeringPlotReference(int tab_id)
{
    return engineering_plot_ref.at(tab_id);
}

Enums::PlotType PlotPalette::GetPlotTypeByTabId(int tab_id)
{
    return Enums::getPlotTypeByIndex(tab_to_type[tab_id]);
}

Enums::PlotUnit PlotPalette::GetPlotUnitByTabId(int tab_id)
{
    return Enums::getPlotUnitByIndex(tab_to_unit[tab_id]);
}

int PlotPalette::GetLowestSyncedTabIndex()
{
    auto minIter = std::min_element(synced_tabs.begin(), synced_tabs.end());
    return *minIter;
}

bool PlotPalette::HasSyncedTabWithIndex(int tab_id)
{
    return std::find(synced_tabs.begin(), synced_tabs.end(), tab_id) != synced_tabs.end();
}

void PlotPalette::HandleDesignerParamsSelected(QString plotTitle, std::vector<Quantity> &quantities)
{
    emit paletteParamsSelected(plotTitle, quantities);
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

    connect(&contextMenu, &QMenu::aboutToShow, this, [=]() {
        syncCheckBox->setChecked(std::find(synced_tabs.begin(), synced_tabs.end(), tabIndex) != synced_tabs.end());  // Ensure it is checked when menu opens
    });

    connect(syncCheckBox, &QCheckBox::toggled, this, [tabIndex, syncCheckBox, this](bool checked) {
        OnCheckboxToggled(checked, tabIndex);
        if (!checked)
            syncCheckBox->setChecked(true);
    });

    contextMenu.addAction(syncAction);

    // What should be a simple use case for syncing plots?  How many can be synced at a given time?  What combos are allowed?
    // A set of plots are chosen for syncing, and one with the lowest tab index is designated as the leader.

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
    } else if (selectedAction == syncAction)
    {
    } else if (selectedAction == popoutPlotAction) {
        tabBar()->setTabVisible(tabIndex, false);

        std::vector<Quantity> quantities;
        quantities.push_back(Quantity(Enums::plotTypeToString(GetPlotTypeByTabId(tabIndex)), GetPlotUnitByTabId(tabIndex)));
        quantities.push_back(Quantity("Frames", Enums::PlotUnit::Undefined_PlotUnit)); // TODO: Implement analogous name/enum transfer here.

        emit popoutPlot(tabIndex, "title", quantities);
    } else if (selectedAction == editBanner) {
        emit editClassification(tabIndex, engineering_plot_ref.at(tabIndex)->getPlotter()->getPlotLabel());
    } else if (selectedAction == plotOptions1) {
        bool yAxisIsLogarithmic = engineering_plot_ref.at(tabIndex)->getPlotter()->getYAxis()->getLogAxis();
        qDebug() << "yAxisIsLogarithmic=" << yAxisIsLogarithmic;
        engineering_plot_ref.at(tabIndex)->getPlotter()->getYAxis()->setLogAxis(!yAxisIsLogarithmic);
        engineering_plot_ref.at(tabIndex)->getPlotter()->zoomToFit(true, true, false, false, 0.5, 0.5);
        // TODO: Refine this ^^^ so it zooms to where you left off.
    }
}

void PlotPalette::OnCheckboxToggled(bool checked, int tabIndex)
{
    if (checked) // if checked (a moment ago)
    {
        AddSyncedTabIndex(tabIndex);
    }
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

        menu.addAction("Stop Syncing", [this]() {
            synced_tabs.clear();
            for (int i = 0; i< this->tabBar()->count(); i++)
            {
                GetEngineeringPlotReference(i)->getPlotter()->resetMasterSynchronization(JKQTBasePlotter::sdXAxis);
            }

            QMessageBox::information(nullptr, "Sync Reset", "Plot syncing has been reset.");
        });

        if (abir_data_loaded)
        {
            menu.addAction("Toggle Plot All Data", [this]() {
                emit toggleUseSubInterval();
            });
        }

        menu.exec(event->globalPosition().toPoint()); // Show menu at the cursor position
    } else {
        QWidget::mousePressEvent(event); // Pass the event to the base class
    }
}

void PlotPalette::mouseMoveEvent(QMouseEvent* event)
{
    auto tabBar = this->tabBar();
    int lastIndex = tabBar->count() - 1;
    if (lastIndex < 0)
        return;

    QPoint tabBarPos = tabBar->mapFrom(this, event->position().toPoint());
    QRect lastTabRect = tabBar->tabRect(lastIndex);

    // Check if the point is just to the right of the last tab:
    int tolerance = lastTabRect.width();
    bool isToRight = (tabBarPos.y() >= lastTabRect.top() &&
                      tabBarPos.y() <= lastTabRect.bottom() &&
                      tabBarPos.x() > this->width() - tolerance &&
                      tabBarPos.x() <= this->width());

    if (isToRight) {
        QString tooltipText = "Double right-click in right-side empty area to add new tab!";
        QToolTip::showText(event->globalPosition().toPoint(), tooltipText, this);
    }

    QWidget::mouseMoveEvent(event);
}

void PlotPalette::leaveEvent(QEvent* event)
{
    QToolTip::hideText();
    //QTabBar::leaveEvent(event);
}

void PlotPalette::PlotAllSirveTracks(int override_track_id)
{
    for (int plot_id = 0; plot_id < engineering_plot_ref.size(); plot_id++)
    {
        engineering_plot_ref.at(plot_id)->PlotSirveTracks(override_track_id);
    }
}

void PlotPalette::RecolorManualTrack(int plot_id, int track_id, QColor new_color)
{
    engineering_plot_ref.at(plot_id)->RecolorManualTrack(track_id, new_color);
}

void PlotPalette::RedrawPlot(int plot_id)
{
    engineering_plot_ref.at(plot_id)->redrawPlot();
}

void PlotPalette::RemovePoppedTabIndex(int tab_index)
{
    popped_tabs.erase(std::remove(popped_tabs.begin(), popped_tabs.end(), tab_index), popped_tabs.end());
}

void PlotPalette::RemoveSyncedTabIndex(int tab_index)
{
    synced_tabs.erase(std::remove(synced_tabs.begin(), synced_tabs.end(), tab_index), synced_tabs.end());
    GetEngineeringPlotReference(tab_index)->synchronizeToMaster(GetEngineeringPlotReference(GetLowestSyncedTabIndex()), JKQTBasePlotter::sdXAxis);
}

void PlotPalette::RouteFramelineUpdate(int frame)
{
    for (int tab_index : popped_tabs)
    {
        engineering_plot_ref.at(tab_index)->PlotCurrentFrameline(frame);
    }

    // Qt guarantees the value of currentIndex() is not an element of popped_tabs.
    engineering_plot_ref.at(currentIndex())->PlotCurrentFrameline(frame);
}

void PlotPalette::SetAbirDataLoaded(bool osm_data_loaded)
{
    this->abir_data_loaded = osm_data_loaded;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void PlotPalette::UpdatePlotLabel(int tab_id, const QString& label)
{
    engineering_plot_ref.at(tab_id)->getPlotter()->setPlotLabel(label);
    engineering_plot_ref.at(tab_id)->getPlotter()->redrawPlot();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void PlotPalette::UpdateManualPlottingTrackFrames(int plot_id, std::vector<ManualPlottingTrackFrame> frames, const std::set<int>& track_ids)
{
    engineering_plot_ref.at(plot_id)->UpdateManualPlottingTrackFrames(std::move(frames), track_ids);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void PlotPalette::UpdateAllManualPlottingTrackFrames(std::vector<ManualPlottingTrackFrame> frames, const std::set<int>& track_ids)
{
    for (int plot_id = 0; plot_id < engineering_plot_ref.size(); plot_id++)
    {
        engineering_plot_ref.at(plot_id)->UpdateManualPlottingTrackFrames(std::move(frames), track_ids);
    }
}
