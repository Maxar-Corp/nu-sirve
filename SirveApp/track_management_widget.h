#pragma once

#ifndef TRACK_MANAGEMENT_WIDGET_H
#define TRACK_MANAGEMENT_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "support/qt_elements_with_ids.h"

class TrackManagementWidget : public QWidget
{
    Q_OBJECT

public:
    TrackManagementWidget(QWidget *parent = nullptr);
    ~TrackManagementWidget();

    void add_track_control(int id);
    void remove_track_control(int id);

signals:
    void display_track(int id);
    void hide_track(int id);
    void delete_track(int id);
    void recolor_track(int id, QColor color);

private:
    QWidget* create_track_control(int id);
    void handle_track_color_choice(int id, int index);

    QVBoxLayout *layout;
};

#endif