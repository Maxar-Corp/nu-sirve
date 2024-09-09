#pragma once

#ifndef TRACK_MANAGEMENT_WIDGET_H
#define TRACK_MANAGEMENT_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "color_scheme.h"
#include "support/qt_elements_with_ids.h"

class TrackManagementWidget : public QWidget
{
    Q_OBJECT

public:
    TrackManagementWidget(QWidget *parent = nullptr);
    ~TrackManagementWidget();
    std::map<int,QColor> track_colors;
    void AddTrackControl(int id);
    void RemoveTrackControl(int id);

signals:
    void displayTrack(int id, QColor color);
    void hideTrack(int id);
    void deleteTrack(int id);
    void recolorTrack(int id, QColor color);
    void recolorLegend(int id, QColor color);

private:
    QWidget* CreateTrackControl(int id);
    void HandleTrackColorSelection(int id, int index);
    void HandleDisplayTrack(int id);

    QVBoxLayout *layout;
};

#endif
