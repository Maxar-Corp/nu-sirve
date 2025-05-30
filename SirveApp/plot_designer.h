#ifndef PLOT_DESIGNER_H
#define PLOT_DESIGNER_H

#include "qcombobox.h"
#include "qlistwidget.h"
#include "quantity.h"
#include <QPointer>
#include <QDialog>

#include "single_check_list.h"

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


private slots:
    void onSingleCheckItemSelected(QListWidgetItem *item);

private:
    QPointer<QLabel> label1;
    QPointer<QLabel> label2;
    QPointer<QLineEdit> plotTitle;
    QPointer<QTextEdit> textEdit1;
    QPointer<QTextEdit> textEdit2;
    QPointer<QPushButton> closeButton;

    QPointer<SingleCheckList> listWidget1;
    QPointer<SingleCheckList> listWidget2;

    QPointer<QComboBox> unitsBox1;
    QPointer<QComboBox> unitsBox2;

    void AddCheckableItems(QListWidget *listWidget, const QStringList &items);
    bool AnyItemChecked(QListWidget *listWidget);
    void SetAxisUnit(QString checked_value, QComboBox *units_combo_box);
};

#endif // PLOT_DESIGNER_H
