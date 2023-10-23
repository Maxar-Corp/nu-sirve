#include "playback.h"

Playback::Playback(unsigned int number_frames)
{
	timer = new QTimer(this);
	timer->setInterval(1000);
	timer->setSingleShot(false);

	timer->stop();

	current_frame_number = 0;
	max_frame_number = number_frames - 1;
	is_reverse = false;

	// Speeds in frames per second
	speeds = {1/15.0, 0.10, 0.20, .25, 1/3.0, .5, 1, 2, 3, 4, 5, 10, 15, 20, 25, 30, 60};
	index_speed = 10;

	QObject::connect(timer, &QTimer::timeout, this, &Playback::timer_update);
}

Playback::~Playback() {

	delete timer;
}

void Playback::set_number_of_frames(unsigned int value)
{
	max_frame_number = value - 1;
	current_frame_number = 0;
}

unsigned int Playback::get_current_frame_number()
{
	return current_frame_number;
}

unsigned int Playback::get_max_frame_number()
{
	return max_frame_number;
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
	emit update_frame(current_frame_number);
}

void Playback::timer_update()
{
	if (is_reverse)
	{
		if (current_frame_number == 0) {
			current_frame_number = max_frame_number;
		}
		else {
			current_frame_number--;
		}
	}
	else {
		if (current_frame_number == max_frame_number){
			current_frame_number = 0;
		}
		else {
			current_frame_number++;
		}
	}

	emit update_frame(current_frame_number);
}

void Playback::set_current_frame_number(unsigned int value)
{
	if (value == current_frame_number)
		return;
	
	if (value > max_frame_number)
		current_frame_number = 0;
	else
		current_frame_number = value;

	emit update_frame(current_frame_number);
}

void Playback::prev_frame()
{
	timer->stop();
	if (current_frame_number == 0)
		current_frame_number = max_frame_number;
	else
		current_frame_number--;

	emit update_frame(current_frame_number);
}

void Playback::next_frame()
{
	
	timer->stop();
	
	if (current_frame_number == max_frame_number)
		current_frame_number = 0;
	else
		current_frame_number++;

	emit update_frame(current_frame_number);
}

void Playback::stop_timer() {

	timer->stop();
}

void Playback::reverse() {

	is_reverse = true;
	timer->start();

	emit update_frame(current_frame_number);
}

bool Playback::is_running()
{
	return timer->isActive();
}

void Playback::set_initial_speed_index(int index)
{
	
	if (index < speeds.size())
	{
		index_speed = index;

		timer_frequency = 1000 * 1 / speeds[index_speed];
		timer->stop();
		timer->setInterval(timer_frequency);
		timer->setSingleShot(false);
	}

}
