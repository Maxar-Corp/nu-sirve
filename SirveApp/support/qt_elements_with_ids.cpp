#include "support/qt_elements_with_ids.h"

QComboBoxWithId::QComboBoxWithId(int id, QWidget *parent)
    : QComboBox(parent), id(id)
{
    connect(this, qOverload<int>(&QComboBox::currentIndexChanged), this, &QComboBoxWithId::HandleIndexChanged);
}

void QComboBoxWithId::HandleIndexChanged(int index)
{
    emit currentIndexChangedWithId(id, index);
}


QPushButtonWithId::QPushButtonWithId(int id, const QString& text, QWidget *parent)
    : QPushButton(text, parent), id(id)
{
    connect(this, &QPushButtonWithId::clicked, this, &QPushButtonWithId::HandleButtonClick);
}

void QPushButtonWithId::HandleButtonClick()
{
    emit clickedWithId(id);
}

QCheckBoxWithId::QCheckBoxWithId(int id, const QString& text, QWidget *parent)
    : QCheckBox(text, parent), id(id)
{
    connect(this, &QCheckBox::stateChanged, this, &QCheckBoxWithId::HandleStateChanged);
}

void QCheckBoxWithId::HandleStateChanged(int state)
{
    if (state == Qt::Checked)
    {
        emit checkedWithId(id);
    }
    else
    {
        emit uncheckedWithId(id);
    }
}
