#include "play_back.h"

Playback::Playback(int number_frames)
{
	timer = new QTimer(this);
	timer->setInterval(1000);
	timer->setSingleShot(false);

	timer->stop();
	max_counter = number_frames;
	is_reverse = false;

	counter = 0;

	// Speeds in frames per second
	speeds = {1/15.0, 0.10, 0.20, .25, 1/3.0, .5, 1, 2, 3, 4, 5, 10, 15, 20, 25, 30};
	index_speed = 6;

	QObject::connect(timer, &QTimer::timeout, this, &Playback::timer_update);
}

Playback::~Playback() {

	delete timer;
}

void Playback::set_number_of_frames(int value)
{
	max_counter = value - 1;
	counter = -1;
}

int Playback::get_counter()
{
	return counter;
}

void Playback::speed_timer() {

	if (index_speed + 1 >= speeds.size())
	{
		return;
	}

	index_speed++;
	timer_frequency = 1000 * 1 / speeds[index_speed];
	timer->stop();
	timer->setInterval(timer_frequency);
	timer->setSingleShot(false);
	timer->start();
}

double Playback::get_fps()
{
	return speeds[index_speed];	
}

void Playback::slow_timer() {

	if (index_speed - 1 < 0)
	{
		return;
	}

	index_speed--;
	timer_frequency = 1000 * 1 / speeds[index_speed];
	timer->stop();
	timer->setInterval(timer_frequency);
	timer->setSingleShot(false);
	timer->start();
}

void Playback::start_timer() {

	is_reverse = false;
	timer->start();
}

void Playback::timer_update()
{
	if (is_reverse)
		counter--;
	else
		counter++;

	if (counter < 0)
	{
		counter = max_counter;
	}

	if (counter > max_counter)
	{
		counter = 0;
	}	

	emit update_frame(counter);
}

void Playback::set_counter(int value)
{
	if (value > max_counter)
		counter = 0;
	else
		counter = value;

	emit update_frame(counter);
}

void Playback::prev_frame()
{
	timer->stop();
	counter--;

	if (counter < 0)
		counter = max_counter;

	emit update_frame(counter);

}

void Playback::next_frame()
{
	
	timer->stop();
	counter++;
		
	if (counter > max_counter)
		counter = 0;

	emit update_frame(counter);
	
}

void Playback::stop_timer() {

	timer->stop();
}

void Playback::reverse() {

	is_reverse = true;
	timer->start();
}

bool Playback::is_running()
{
	return timer->isActive();
}

void Playback::set_speed_index(int index)
{
	
	if (index < speeds.size())
	{
		index_speed = index;
	}

}
