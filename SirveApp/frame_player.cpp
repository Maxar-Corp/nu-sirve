#include "frame_player.h"
#include "support/qthelpers.h"

FramePlayer::FramePlayer(unsigned int number_frames)
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

    connect(timer, &QTimer::timeout, this, &FramePlayer::UpdateTimer);
}

FramePlayer::~FramePlayer() {

	delete timer;
}

void FramePlayer::set_number_of_frames(unsigned int value)
{
	max_frame_number = value - 1;
	current_frame_number = 0;
}

unsigned int FramePlayer::get_current_frame_number()
{
	return current_frame_number;
}

unsigned int FramePlayer::get_max_frame_number()
{
	return max_frame_number;
}

void FramePlayer::IncreaseTimerInterval() {

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

double FramePlayer::get_fps()
{
	return speeds[index_speed];	
}

void FramePlayer::DecreaseTimerInterval() {

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

void FramePlayer::StartTimer() {

	is_reverse = false;
	timer->start();
    emit frameSelected(current_frame_number);
}

void FramePlayer::UpdateTimer()
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

    emit frameSelected(current_frame_number);
}

void FramePlayer::set_current_frame_number(unsigned int value)
{
	if (value == current_frame_number)
		return;
	
	if (value > max_frame_number)
		current_frame_number = 0;
	else
		current_frame_number = value;

    emit frameSelected(current_frame_number);
}

void FramePlayer::GotoPrevFrame()
{
	timer->stop();
	if (current_frame_number == 0)
		current_frame_number = max_frame_number;
	else
		current_frame_number--;

    emit frameSelected(current_frame_number);
}

void FramePlayer::GotoNextFrame(unsigned int frame_amt)
{
    if (frame_amt > 0 && frame_amt <= max_frame_number)
    {
        timer->stop();

        int new_frame_number = current_frame_number + frame_amt;

        if (new_frame_number > max_frame_number)
        {
            new_frame_number -= max_frame_number;
        }

        emit frameSelected(new_frame_number);
    }
    else
    {
        QtHelpers::LaunchMessageBox("Error", "The number of frames to advance is out of range.  Please adjust accordingly to a value between zero and the number of frames selected minus one.");
    }
}

void FramePlayer::StopTimer() {

	timer->stop();
}

void FramePlayer::ReverseTimer() {

	is_reverse = true;
	timer->start();

    emit frameSelected(current_frame_number);
}

bool FramePlayer::is_running()
{
	return timer->isActive();
}

void FramePlayer::set_initial_speed_index(int index)
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
