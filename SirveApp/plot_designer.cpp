#include <QApplication>
#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "plot_designer.h"
#include "qlineedit.h"
#include "qlistwidget.h"

PlotDesigner::PlotDesigner(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Plot Designer");

    // Create the editable textboxes with placeholder text
    QLineEdit *editBox1 = new QLineEdit(this);
    editBox1->setPlaceholderText("Enter Y-Axis Label...");

    QLineEdit *editBox2 = new QLineEdit(this);
    editBox2->setPlaceholderText("Enter X-Axis Label...");

    // Create the list widgets
    listWidget1 = new QListWidget(this);
    listWidget2 = new QListWidget(this);

    // Create the buttons
    QPushButton *closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &PlotDesigner::accept);

    // Layout the widgets
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(editBox1);
    layout->addWidget(listWidget1);
    layout->addWidget(editBox2);
    layout->addWidget(listWidget2);
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

void PlotDesigner::AddCheckableItemsByIndex(int index, QStringList items)
{
    if (index == 0)
        AddCheckableItems(listWidget1, items);
    else
        AddCheckableItems(listWidget2, items);
}

void PlotDesigner::accept() {
    // Gather strings from the two list widgets
    std::vector<QString> group1;
    for (int i = 0; i < listWidget1->count(); ++i) {
        if (listWidget1->item(i)->checkState() == Qt::Checked)
            group1.push_back(listWidget1->item(i)->text());
    }

    for (int i = 0; i < listWidget2->count(); ++i) {
        if (listWidget2->item(i)->checkState() == Qt::Checked)
            group1.push_back(listWidget2->item(i)->text());
    }

    qDebug() << "Sending the strings: " << group1.data();

    // Emit the signal
    emit designerParamsSelected(group1);

    // Call the base class accept to close the dialog
    QDialog::accept();
}
