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

	Playback(int number_frames);
	~Playback();
	void set_number_of_frames(int value);
	int get_counter();

	int timer_frequency, index_speed;
	QTimer  *timer;
	std::vector<double> speeds;
	bool is_reverse;

public slots:
	void speed_timer();
	void slow_timer();
	void stop_timer();
	void start_timer();
	void timer_update();
	void set_counter(int value);
	void prev_frame();
	void next_frame();
	void reverse();

signals:
	void update_frame(int counter);

private:

	int counter;
	unsigned int max_counter;
};


#endif // PLAYBACK_H