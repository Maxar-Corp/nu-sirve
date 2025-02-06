#ifndef SINGLECHECKLIST_H
#define SINGLECHECKLIST_H

#include <QListWidget>
#include <QListWidgetItem>

class SingleCheckList : public QListWidget {
    Q_OBJECT

public:
    explicit SingleCheckList(QWidget *parent = nullptr);

private slots:
    void onItemChanged(QListWidgetItem *changedItem);
};

#endif // SINGLECHECKLIST_H
