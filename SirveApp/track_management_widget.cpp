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

void TrackManagementWidget::RemoveTrackControl(int id)
{
    QWidget * track_control = findChild<QWidget*>(QString("TrackControl_%1").arg(id));
    if (track_control != nullptr)
    {
        delete(track_control);
    }
}

void TrackManagementWidget::AddTrackControl(int id)
{
    QWidget * existing_track_control = findChild<QWidget*>(QString("TrackControl_%1").arg(id));
    if (existing_track_control != nullptr)
    {
        //If there's already a track control widget for this ID, just make sure the checkbox is checked
        QCheckBoxWithId * chk_box = existing_track_control->findChild<QCheckBoxWithId*>();
        chk_box->setChecked(true);
        return;
    }

    QWidget *track_control = CreateTrackControl(id);

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

QWidget* TrackManagementWidget::CreateTrackControl(int id)
{
    QWidget *track_control = new QWidget();
    track_control->setObjectName(QString("TrackControl_%1").arg(id));

    QLabel *label = new QLabel("Track ID: " + QString::number(id));
    QCheckBoxWithId *chk_should_display = new QCheckBoxWithId(id, "Display");
    chk_should_display->setChecked(true);
    QPushButtonWithId *delete_button = new QPushButtonWithId(id, "Delete");

    QComboBoxWithId *recolor_combobox = new QComboBoxWithId(id);
    recolor_combobox->addItems(ColorScheme::get_track_colors());

    connect(chk_should_display, &QCheckBoxWithId::checkedWithId, this, &TrackManagementWidget::displayTrack);
    connect(chk_should_display, &QCheckBoxWithId::uncheckedWithId, this, &TrackManagementWidget::hideTrack);
    connect(delete_button, &QPushButtonWithId::clickedWithId, this, &TrackManagementWidget::deleteTrack);
    connect(recolor_combobox, &QComboBoxWithId::currentIndexChangedWithId, this, &TrackManagementWidget::HandleTrackColorSelection);

    QVBoxLayout *control_layout = new QVBoxLayout(track_control);
    QHBoxLayout *top_box = new QHBoxLayout();
    top_box->addWidget(label);
    top_box->addWidget(chk_should_display);
    top_box->addStretch(1);

    QHBoxLayout *bottom_box = new QHBoxLayout();
    bottom_box->addWidget(recolor_combobox);
    bottom_box->addWidget(delete_button);
    bottom_box->addStretch(1);

    control_layout->addLayout(top_box);
    control_layout->addLayout(bottom_box);

    return track_control;
}

void TrackManagementWidget::HandleTrackColorSelection(int id, int index)
{
    QStringList color_options = ColorScheme::get_track_colors();
    emit recolorTrack(id, color_options[index]);
}
