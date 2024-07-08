#pragma once

#ifndef FRAME_PLAYER_H
#define FRAME_PLAYER_H

#include <iostream>
#include <string>

#include <QWidget>
#include <QTimer>

class FramePlayer : public QWidget
{
	Q_OBJECT
public:

    FramePlayer(unsigned int number_frames);
    ~FramePlayer();

    void set_current_frame_number(unsigned int value);
	void set_number_of_frames(unsigned int value);
	unsigned int get_current_frame_number();
	unsigned int get_max_frame_number();

	int timer_frequency, index_speed;
	QTimer  *timer;
	std::vector<double> speeds;
	bool is_reverse;
	double get_fps();
	bool is_running();
	void set_initial_speed_index(int index);

signals:
    void frameSelected(unsigned int current_frame_number);

public slots:
    void IncreaseTimerInterval();
    void DecreaseTimerInterval();
    void StopTimer();
    void StartTimer();
    void UpdateTimer();
    void ReverseTimer();

    void GotoPrevFrame();
    void GotoNextFrame(int frame_amt);

private:
	unsigned int current_frame_number;
	unsigned int max_frame_number;
};


#endif // PLAYBACK_H
