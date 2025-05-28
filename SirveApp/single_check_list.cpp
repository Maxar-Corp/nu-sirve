#include "single_check_list.h"

SingleCheckList::SingleCheckList(QWidget *parent)
    : QListWidget(parent) {
    connect(this, &QListWidget::itemChanged, this, &SingleCheckList::onItemChanged);
    connect(this, &QListWidget::itemClicked, this, &SingleCheckList::onItemClicked);

}

void SingleCheckList::onItemChanged(QListWidgetItem *changedItem) {
    if (changedItem->checkState() == Qt::Checked) {
        // Uncheck all other items
        for (int i = 0; i < count(); ++i) {
            QListWidgetItem *item = this->item(i);
            if (item != changedItem && item->checkState() == Qt::Checked) {
                item->setCheckState(Qt::Unchecked);
            }
        }
    }
}

void SingleCheckList::onItemClicked(QListWidgetItem *changedItem) {
    if (changedItem->checkState() == Qt::Unchecked) {
        changedItem->setCheckState(Qt::Checked);
        // Uncheck all other items
        for (int i = 0; i < count(); ++i) {
            QListWidgetItem *item = this->item(i);
            if (item != changedItem && item->checkState() == Qt::Checked) {
                item->setCheckState(Qt::Unchecked);
            }
        }
    }
}
