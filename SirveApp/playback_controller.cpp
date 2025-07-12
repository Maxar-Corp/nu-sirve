#include "playback_controller.h"
#include "support/qthelpers.h"

const std::vector<double> PlaybackController::kSpeeds =
	{1 / 15.0, 0.10, 0.20, .25, 1 / 3.0, .5, 1, 2, 3, 4, 5, 10, 15, 20, 25, 30, 60};

PlaybackController::PlaybackController(QObject* parent, uint32_t number_frames) : QObject(parent)
{
	timer_ = std::make_unique<QTimer>(this);
	timer_->setInterval(1000);
	timer_->setSingleShot(false);
	timer_->stop();

	max_frame_number_ = number_frames > 0 ? number_frames - 1 : 0;
	is_reverse_ = false;

	// Speeds in frames per second
	index_speed_ = 10;

	connect(timer_.get(), &QTimer::timeout, this, &PlaybackController::UpdateTimer);
}

void PlaybackController::SetNumberOfFrames(uint32_t value)
{
	max_frame_number_ = value - 1;
	current_frame_number_ = 0;
}

uint32_t PlaybackController::GetCurrentFrameNumber() const
{
	return current_frame_number_;
}

uint32_t PlaybackController::GetMaxFrameNumber() const
{
	return max_frame_number_;
}

void PlaybackController::IncreaseTimerInterval()
{
	if (index_speed_ + 1 >= kSpeeds.size()) {
		return;
	}

	index_speed_++;
	timer_frequency_ = 1000 * 1 / kSpeeds[index_speed_];
	bool isPlaying = timer_->isActive();
	timer_->stop();
	timer_->setInterval(timer_frequency_);
	timer_->setSingleShot(false);
	if (isPlaying){
		timer_->start();
	}
}

double PlaybackController::GetFps() const
{
	return kSpeeds[index_speed_];
}

void PlaybackController::DecreaseTimerInterval()
{
	if (index_speed_ < 1) {
		return;
	}

	index_speed_--;
	timer_frequency_ = 1000 * 1 / kSpeeds[index_speed_];
	bool isPlaying = timer_->isActive();
	timer_->stop();
	timer_->setInterval(timer_frequency_);
	timer_->setSingleShot(false);
	if (isPlaying){
		timer_->start();
	}
}

void PlaybackController::StartTimer()
{
	is_reverse_ = false;
	timer_->start();
	emit frameSelected(current_frame_number_);
}

void PlaybackController::UpdateTimer()
{
	if (is_reverse_) {
		if (current_frame_number_ == 0) {
			current_frame_number_ = max_frame_number_;
		} else {
			current_frame_number_--;
		}
	} else {
		if (current_frame_number_ == max_frame_number_) {
			current_frame_number_ = 0;
		} else {
			current_frame_number_++;
		}
	}

	emit frameSelected(current_frame_number_);
}

void PlaybackController::SetCurrentFrameNumber(uint32_t value)
{
	if (value == current_frame_number_)
		return;

	if (value > max_frame_number_)
		current_frame_number_ = 0;
	else
		current_frame_number_ = value;

	emit frameSelected(current_frame_number_);
}

void PlaybackController::GotoPrevFrame()
{
	timer_->stop();
	if (current_frame_number_ == 0)
		current_frame_number_ = max_frame_number_;
	else
		current_frame_number_--;

	emit frameSelected(current_frame_number_);
}

void PlaybackController::GotoNextFrame()
{
	timer_->stop();

	if (current_frame_number_ == max_frame_number_)
		current_frame_number_ = 0;
	else
		current_frame_number_++;

	emit frameSelected(current_frame_number_);
}

void PlaybackController::CustomAdvanceFrame(uint32_t frame_amt)
{
	if (frame_amt > 0 && frame_amt <= max_frame_number_) {
		timer_->stop();

		uint32_t new_frame_number = current_frame_number_ + frame_amt;

		if (new_frame_number > max_frame_number_) {
			new_frame_number -= max_frame_number_;
		}

		emit frameSelected(new_frame_number);
	}
}

void PlaybackController::StopTimer()
{
	timer_->stop();
}

void PlaybackController::ReverseTimer()
{
	is_reverse_ = true;
	timer_->start();

	emit frameSelected(current_frame_number_);
}

bool PlaybackController::IsRunning() const
{
	return timer_->isActive();
}