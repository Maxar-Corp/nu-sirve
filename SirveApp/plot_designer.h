#ifndef PLOT_DESIGNER_H
#define PLOT_DESIGNER_H

#include "qlistwidget.h"
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

signals:
    void designerParamsSelected(const std::vector<QString> &strings);

public slots:
    void accept() override;

private:
    QLabel *label1;       // Label for the first group
    QLabel *label2;       // Label for the second group
    QTextEdit *textEdit1; // Multiline box for the first group
    QTextEdit *textEdit2; // Multiline box for the second group
    QPushButton *closeButton; // Close button

    QListWidget *listWidget1;
    QListWidget *listWidget2;

    void AddCheckableItems(QListWidget *listWidget, const QStringList &items);
};

#endif // PLOT_DESIGNER_H
