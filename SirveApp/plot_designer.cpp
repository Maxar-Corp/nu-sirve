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
    editBox1->setPlaceholderText("Enter Axis 1 Label...");

    QLineEdit *editBox2 = new QLineEdit(this);
    editBox2->setPlaceholderText("Enter Axis 2 Label...");

    // Create the list widgets
    QListWidget *listWidget1 = new QListWidget(this);
    QListWidget *listWidget2 = new QListWidget(this);

    // Populate the first list with checkable items
    AddCheckableItems(listWidget1, { "Option 1", "Option 2", "Option 3" });

    // Populate the second list with checkable items
    AddCheckableItems(listWidget2, { "Option X", "Option Y", "Option Z" });

    // Create the buttons
    QPushButton *closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

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
