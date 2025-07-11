#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "enums.h"
#include "plot_designer.h"

#include <qbuttongroup.h>
#include <QCheckBox>
#include <QPointer>
#include <QRadioButton>

#include "support/qthelpers.h"
#include "qcombobox.h"
#include "qlineedit.h"
#include "qlistwidget.h"
#include "quantity.h"
#include "single_check_list.h"

void populateComboBox(QComboBox* comboBox) {
    for (int i = 0; i < static_cast<int>(Enums::PlotUnit::Undefined_PlotUnit); ++i) {
        auto value = static_cast<Enums::PlotUnit>(i);
        comboBox->addItem(Enums::plotUnitToString(Enums::getPlotUnitByIndex(i)), QVariant::fromValue(value));
    }
}

PlotDesigner::PlotDesigner(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Plot Designer");

    plotTitle = new QLineEdit(this);
    plotTitle->setPlaceholderText("Give your plot a title...");

    unitsBox1 = new QComboBox(this);
    unitsBox1->setPlaceholderText("Choose Y-Axis Units...");

    unitsBox2 = new QComboBox(this);
    unitsBox2->setPlaceholderText("Choose X-Axis Units...");

    populateComboBox(unitsBox1);
    populateComboBox(unitsBox2);

    QLabel *plotYAxisLabel = new QLabel("Choose Y Axis Quantity & Units:");
    QLabel *plotXAxisLabel = new QLabel("Choose X Axis Quantity & Units:");

    // Create the list widgets
    listWidget1 = new SingleCheckList(this);
    listWidget1->setObjectName("listWidget1");
    listWidget2 = new SingleCheckList(this);
    listWidget2->setObjectName("listWidget2");

    // Create the buttons
    QPushButton *closeButton = new QPushButton("Create Tab", this);
    connect(closeButton, &QPushButton::clicked, this, &PlotDesigner::accept);

    // Connect the signal from SingleCheckList to this widget's slot
    connect(listWidget1, &SingleCheckList::itemChecked, this, &PlotDesigner::onSingleCheckItemSelected, Qt::UniqueConnection);
    connect(listWidget2, &SingleCheckList::itemChecked, this, &PlotDesigner::onSingleCheckItemSelected, Qt::UniqueConnection);



    // Layout the widgets
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(plotTitle);
    layout->addWidget(plotYAxisLabel);
    layout->addWidget(listWidget1);
    layout->addWidget(unitsBox1);
    layout->addWidget(plotXAxisLabel);
    layout->addWidget(listWidget2);
    layout->addWidget(unitsBox2);
    layout->addWidget(closeButton);

    setLayout(layout);
};

void PlotDesigner::AddCheckableItems(QListWidget *listWidget, const QStringList &items) {
    for (const QString &item : items) {
        QListWidgetItem *listItem = new QListWidgetItem(item, listWidget);
        listItem->setFlags(listItem->flags() | Qt::ItemIsUserCheckable);
        listItem->setCheckState(Qt::Unchecked); // Default to unchecked
    }
}

bool PlotDesigner::AnyItemChecked(QListWidget *listWidget)
{
    bool anyChecked = false;
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        if (item && item->checkState() == Qt::Checked) {
            anyChecked = true;
            break; // No need to check further
        }
    }
    return anyChecked;
}

void PlotDesigner::AddCheckableItemsByIndex(int index, QStringList items)
{
    if (index == 0)
        AddCheckableItems(listWidget1, items);
    else
        AddCheckableItems(listWidget2, items);
}

void PlotDesigner::SetDefaultUnits() {
    unitsBox1->setCurrentIndex(unitsBox1->count() - 1);
    unitsBox2->setCurrentIndex(unitsBox2->count() - 1);
}

void PlotDesigner::accept() {
    // Gather strings from the two list widgets
    std::vector<Quantity> quantity_pair;
    for (int i = 0; i < listWidget1->count(); ++i) {
        if (listWidget1->item(i)->checkState() == Qt::Checked)
            quantity_pair.push_back(Quantity(listWidget1->item(i)->text(), Enums::getPlotUnitByIndex(Enums::getPlotUnitIndexFromString(unitsBox1->currentText()))));
    }

    for (int i = 0; i < listWidget2->count(); ++i) {
        if (listWidget2->item(i)->checkState() == Qt::Checked)
            quantity_pair.push_back(Quantity(listWidget2->item(i)->text(), Enums::getPlotUnitByIndex(Enums::getPlotUnitIndexFromString(unitsBox2->currentText()))));
    }

    if (plotTitle->text().size() < 1) {
        QtHelpers::LaunchMessageBox(QString("Invalid title."), "Title must be at least one character.");
        return;
    }

    if (! AnyItemChecked(listWidget1)) {
        QtHelpers::LaunchMessageBox(QString("Invalid quantity type."), "You must select one quantity type for the Y axis.");
        return;
    }

    if (! AnyItemChecked(listWidget2)) {
        QtHelpers::LaunchMessageBox(QString("Invalid quantity type."), "You must select one quantity type for the X axis.");
        return;
    }

    emit designerParamsSelected(plotTitle->text(), quantity_pair);

    // Call the base class accept to close the dialog
    QDialog::accept();
}

void PlotDesigner::SetAxisUnit(QString checked_value, QComboBox *units_combo_box)
{
    QString combo_value = units_combo_box->currentText();

    QStringList radian_degree_values;
    radian_degree_values << "Azimuth" << "Elevation" << "Boresight_Azimuth" << "Boresight_Elevation";

    units_combo_box->clear();

    if (radian_degree_values.contains(checked_value))
    {
        units_combo_box->addItem("Degrees");
        units_combo_box->addItem("Radians");
        units_combo_box->setCurrentText("Degrees");
    }
    else if (checked_value == "FovX" || checked_value == "FovY")
    {
        units_combo_box->addItem("Microns");
        units_combo_box->setCurrentText("Microns");
    }
    else if (checked_value == "Sum_Counts")
    {
        units_combo_box->addItem("Counts");
        units_combo_box->setCurrentText("Counts");
    }
    else if (checked_value == "Peak_Irradiance" || checked_value == "Mean_Irradiance" || checked_value == "Sum_Irradiance")
    {
        units_combo_box->addItem("W_m2_str");
        units_combo_box->setCurrentText("W_m2_str");
    }
    else if (checked_value == "Frames")
    {
        units_combo_box->addItem("FrameNumber");
        units_combo_box->setCurrentText("FrameNumber");
    }
    else if (checked_value == "Seconds_From_Epoch" || checked_value == "Seconds_Past_Midnight")
    {
        units_combo_box->addItem("Seconds");
        units_combo_box->setCurrentText("Seconds");
    }else
    {
        populateComboBox(units_combo_box);
    }
}


void PlotDesigner::onSingleCheckItemSelected(QListWidgetItem *item)
{
    if (item) {
        QString selected_value = item->text();
        QString object_name = sender()->objectName();

        if (object_name == listWidget1->objectName())
        {
            SetAxisUnit(selected_value, unitsBox1);
        }
        else if (object_name == listWidget2->objectName())
        {
            SetAxisUnit(selected_value, unitsBox2);
        }
    }
}
