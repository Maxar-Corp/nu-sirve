#ifndef PLOT_DESIGNER_H
#define PLOT_DESIGNER_H

#include "qcombobox.h"
#include "qlistwidget.h"
#include "quantity.h"
#include <QWidget>
#include <QDialog>

class QLabel;
class QTextEdit;
class QPushButton;

class PlotDesigner : public QDialog {
    Q_OBJECT

public:
    explicit PlotDesigner(QWidget *parent = nullptr);

    void AddCheckableItemsByIndex(int index, QStringList items);
    void SetDefaultUnits();

signals:
    void designerParamsSelected(QString plotTitle, std::vector<Quantity> &quantities);

public slots:
    void accept() override;

private:
    QLabel *label1;
    QLabel *label2;
    QLineEdit *plotTitle;
    QTextEdit *textEdit1;
    QTextEdit *textEdit2;
    QPushButton *closeButton;

    QListWidget *listWidget1;
    QListWidget *listWidget2;

    QComboBox *unitsBox1;
    QComboBox *unitsBox2;

    void AddCheckableItems(QListWidget *listWidget, const QStringList &items);
    bool AnyItemChecked(QListWidget *listWidget);
};

#endif // PLOT_DESIGNER_H
