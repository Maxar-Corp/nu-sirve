#include "support/qt_elements_with_ids.h"

QPushButtonWithId::QPushButtonWithId(int id, const QString& text, QWidget *parent)
    : QPushButton(text, parent), id(id)
{
    connect(this, &QPushButtonWithId::clicked, this, &QPushButtonWithId::handle_button_click);
}

void QPushButtonWithId::handle_button_click()
{
    emit clicked_with_id(id);
}

QCheckBoxWithId::QCheckBoxWithId(int id, const QString& text, QWidget *parent)
    : QCheckBox(text, parent), id(id)
{
    connect(this, &QCheckBox::stateChanged, this, &QCheckBoxWithId::handle_state_changed);
}

void QCheckBoxWithId::handle_state_changed(int state)
{
    if (state == Qt::Checked)
    {
        emit checked_with_id(id);
    }
    else
    {
        emit unchecked_with_id(id);
    }
}