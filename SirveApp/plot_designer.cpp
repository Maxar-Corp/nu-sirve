#include <QApplication>
#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "enums.h"
#include "plot_designer.h"
#include "support/qthelpers.h"
#include "qcombobox.h"
#include "qlineedit.h"
#include "qlistwidget.h"
#include "quantity.h"
#include "single_check_list.h"

void populateComboBox(QComboBox* comboBox) {
    for (int i = 0; i <= static_cast<int>(Enums::PlotUnit::None); ++i) {
        Enums::PlotUnit value = static_cast<Enums::PlotUnit>(i);
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

    // Create the list widgets
    listWidget1 = new SingleCheckList(this);
    listWidget2 = new SingleCheckList(this);

    // Create the buttons
    QPushButton *closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &PlotDesigner::accept);

    // Layout the widgets
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(plotTitle);
    layout->addWidget(listWidget1);
    layout->addWidget(unitsBox1);
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
            quantity_pair.push_back(Quantity(listWidget1->item(i)->text(), Enums::getPlotUnitByIndex(unitsBox1->currentIndex())));
    }

    for (int i = 0; i < listWidget2->count(); ++i) {
        if (listWidget2->item(i)->checkState() == Qt::Checked)
            quantity_pair.push_back(Quantity(listWidget2->item(i)->text(), Enums::getPlotUnitByIndex(unitsBox2->currentIndex())));
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
