#include "track_management_widget.h"

TrackControlButton::TrackControlButton(int id, const QString& text, QWidget *parent)
    : QPushButton(text, parent), track_id(id)
{
    connect(this, &TrackControlButton::clicked, this, &TrackControlButton::handle_button_click);
}

void TrackControlButton::handle_button_click()
{
    emit clicked_with_id(track_id);
}

TrackCheckbox::TrackCheckbox(int id, const QString& text, QWidget *parent)
    : QCheckBox(text, parent), track_id(id)
{
    connect(this, &QCheckBox::stateChanged, this, &TrackCheckbox::handle_state_changed);
}

void TrackCheckbox::handle_state_changed(int state)
{
    if (state == Qt::Checked)
    {
        emit checked_with_id(track_id);
    }
    else
    {
        emit unchecked_with_id(track_id);
    }
}

TrackManagementWidget::TrackManagementWidget(QWidget *parent)
    : QWidget(parent)
{
    layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    layout->setSpacing(0);
}

TrackManagementWidget::~TrackManagementWidget()
{
}

void TrackManagementWidget::remove_track_control(int id)
{
    QWidget * track_control = findChild<QWidget*>(QString("TrackControl_%1").arg(id));
    if (track_control != nullptr)
    {
        delete(track_control);
    }
}

void TrackManagementWidget::add_track_control(int id)
{
    if (findChild<QWidget*>(QString("TrackControl_%1").arg(id)) != nullptr)
    {
        //If there's already a track control widget for this ID, no changes are needed
        return;
    }

    QWidget *track_control = create_track_control(id);

    // Maintain ascending order by track ID using the ID we hid in the track control widget's objectName property
    for (int i = 0; i < layout->count(); ++i) {
        QString track_control_name = layout->itemAt(i)->widget()->objectName();
        int existing_id = track_control_name.split("_").last().toInt();
        if (existing_id > id)
        {
            layout->insertWidget(i, track_control);
            return;
        }
    }

    layout->addWidget(track_control);
}

QWidget* TrackManagementWidget::create_track_control(int id)
{
    QWidget *track_control = new QWidget();
    track_control->setObjectName(QString("TrackControl_%1").arg(id));

    QLabel *label = new QLabel("Track ID: " + QString::number(id));
    TrackCheckbox *chk_should_display = new TrackCheckbox(id, "Display");
    chk_should_display->setChecked(true);
    TrackControlButton *delete_button = new TrackControlButton(id, "Delete");

    connect(chk_should_display, &TrackCheckbox::checked_with_id, this, &TrackManagementWidget::display_track);
    connect(chk_should_display, &TrackCheckbox::unchecked_with_id, this, &TrackManagementWidget::hide_track);
    connect(delete_button, &TrackControlButton::clicked_with_id, this, &TrackManagementWidget::delete_track);

    QHBoxLayout *control_layout = new QHBoxLayout(track_control);
    control_layout->addWidget(label);

    control_layout->addWidget(chk_should_display);
    control_layout->addWidget(delete_button);
    control_layout->addStretch(1);

    return track_control;
}