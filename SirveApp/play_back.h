#pragma once

#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <iostream>
#include <string>

#include <QWidget>
#include <QTimer>

class Playback : public QWidget
{
	Q_OBJECT
public:

	Playback(unsigned int number_frames);
	~Playback();
	void set_number_of_frames(unsigned int value);
	unsigned int get_current_frame_number();
	unsigned int get_max_frame_number();

	int timer_frequency, index_speed;
	QTimer  *timer;
	std::vector<double> speeds;
	bool is_reverse;
	double get_fps();
	bool is_running();
	void set_speed_index(int index);

public slots:
	void speed_timer();
	void slow_timer();
	void stop_timer();
	void start_timer();
	void timer_update();
	void set_current_frame_number(unsigned int value);
	void prev_frame();
	void next_frame();
	void reverse();
	
signals:
	void update_frame(unsigned int current_frame_number);

private:
	unsigned int current_frame_number;
	unsigned int max_frame_number;
};


#endif // PLAYBACK_H