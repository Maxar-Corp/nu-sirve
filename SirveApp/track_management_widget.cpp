#include "track_management_widget.h"

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
    QWidget * existing_track_control = findChild<QWidget*>(QString("TrackControl_%1").arg(id));
    if (existing_track_control != nullptr)
    {
        //If there's already a track control widget for this ID, just make sure the checkbox is checked
        QCheckBoxWithId * chk_box = existing_track_control->findChild<QCheckBoxWithId*>();
        chk_box->setChecked(true);
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
    QCheckBoxWithId *chk_should_display = new QCheckBoxWithId(id, "Display");
    chk_should_display->setChecked(true);
    QPushButtonWithId *delete_button = new QPushButtonWithId(id, "Delete");

    QComboBoxWithId *recolor_combobox = new QComboBoxWithId(id);
    recolor_combobox->addItems(ColorScheme::GetTrackColors());

    connect(chk_should_display, &QCheckBoxWithId::checked_with_id, this, &TrackManagementWidget::display_track);
    connect(chk_should_display, &QCheckBoxWithId::unchecked_with_id, this, &TrackManagementWidget::hide_track);
    connect(delete_button, &QPushButtonWithId::clicked_with_id, this, &TrackManagementWidget::delete_track);
    connect(recolor_combobox, &QComboBoxWithId::current_index_changed_with_id, this, &TrackManagementWidget::handle_track_color_choice);

    QHBoxLayout *control_layout = new QHBoxLayout(track_control);
    control_layout->addWidget(label);

    control_layout->addWidget(chk_should_display);
    control_layout->addWidget(recolor_combobox);
    control_layout->addWidget(delete_button);
    control_layout->addStretch(1);

    return track_control;
}

void TrackManagementWidget::handle_track_color_choice(int id, int index)
{
    QStringList color_options = ColorScheme::GetTrackColors();
    emit recolor_track(id, color_options[index]);
}