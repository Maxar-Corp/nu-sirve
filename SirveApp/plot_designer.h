#ifndef PLOT_DESIGNER_H
#define PLOT_DESIGNER_H

#include "qcombobox.h"
#include "qlistwidget.h"
#include "quantity.h"
#include <QPointer>
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
    QPointer<QLabel> label1;
    QPointer<QLabel> label2;
    QPointer<QLineEdit> plotTitle;
    QPointer<QTextEdit> textEdit1;
    QPointer<QTextEdit> textEdit2;
    QPointer<QPushButton> closeButton;

    QPointer<QListWidget> listWidget1;
    QPointer<QListWidget> listWidget2;

    QPointer<QComboBox> unitsBox1;
    QPointer<QComboBox> unitsBox2;

    void AddCheckableItems(QListWidget *listWidget, const QStringList &items);
    bool AnyItemChecked(QListWidget *listWidget);
};

#endif // PLOT_DESIGNER_H
