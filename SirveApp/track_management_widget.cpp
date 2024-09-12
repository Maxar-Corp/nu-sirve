#include "track_management_widget.h"
#include "qdebug.h"

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
        track_colors[id] = QColor(255,0,0,255);
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
        track_colors[id] = QColor(255,0,0,255);
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
            track_colors[id] = QColor(255,0,0,255);
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

    HandleTrackColorSelection(id,0);
    connect(chk_should_display, &QCheckBoxWithId::checkedWithId, this, &TrackManagementWidget::HandleDisplayTrack);
    connect(chk_should_display, &QCheckBoxWithId::uncheckedWithId, this, &TrackManagementWidget::HandleHideTrack);
    connect(delete_button, &QPushButtonWithId::clickedWithId, this, &TrackManagementWidget::deleteTrack);
    connect(recolor_combobox, &QComboBoxWithId::currentIndexChangedWithId, this, &TrackManagementWidget::HandleTrackColorSelection);

    QLabel *lbl_track_desc = new QLabel("");
    lbl_track_desc->setObjectName("track_description");
    lbl_track_desc->setWordWrap(true);
    // lbl_track_desc->setStyleSheet("#track_description {background-color:#f5c87d;}");
    lbl_track_desc->setStyleSheet("#track_description {background-color:rgb(245,200,125,125);}");


    QVBoxLayout *control_layout = new QVBoxLayout(track_control);
    QHBoxLayout *top_box = new QHBoxLayout();
    top_box->addWidget(label);
    top_box->addWidget(chk_should_display);
    top_box->addWidget(lbl_track_desc);
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
    track_colors[id] = color_options[index];
    emit recolorTrack(id, color_options[index]);
}

void TrackManagementWidget::HandleDisplayTrack(int id)
{
    if (track_colors.empty()){
        track_colors[id] = QColor(255,0,0,255);
    }
    QWidget *tmp = this->findChild<QWidget*>(QString("TrackControl_%1").arg(id));
    tmp->findChild<QComboBoxWithId*>()->setEnabled(true);
    emit displayTrack(id, track_colors[id]);
}

void TrackManagementWidget::HandleHideTrack(int id)
{
    QWidget *tmp = this->findChild<QWidget*>(QString("TrackControl_%1").arg(id));
    tmp->findChild<QComboBoxWithId*>()->setEnabled(false);
    emit hideTrack(id);
}