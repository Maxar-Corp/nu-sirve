#pragma once

#ifndef TRACK_MANAGEMENT_WIDGET_H
#define TRACK_MANAGEMENT_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

class TrackControlButton : public QPushButton
{
    Q_OBJECT

public:
    TrackControlButton(int id, const QString& text, QWidget *parent = nullptr);

signals:
    void clicked_with_id(int id);

private slots:
    void handle_button_click();

private:
    int track_id;
};

class TrackCheckbox : public QCheckBox
{
    Q_OBJECT

public:
    TrackCheckbox(int id, const QString& text, QWidget *parent = nullptr);

signals:
    void checked_with_id(int id);
    void unchecked_with_id(int id);

private slots:
    void handle_state_changed(int state);

private:
    int track_id;
};

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

private:
    QWidget* create_track_control(int id);

    QVBoxLayout *layout;
};

#endif