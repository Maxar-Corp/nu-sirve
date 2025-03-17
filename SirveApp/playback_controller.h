#ifndef PLAYBACK_CONTROLLER_H
#define PLAYBACK_CONTROLLER_H

#include <iostream>
#include <string>

#include <QTimer>

/**
 * \brief Controls playback by acting as a timer and managing the current frame number.
 */
class PlaybackController : public QObject
{
	Q_OBJECT

public:
	explicit PlaybackController(unsigned int number_frames);

	unsigned int GetCurrentFrameNumber() const;
	unsigned int GetMaxFrameNumber() const;
	void SetNumberOfFrames(unsigned int value);
	double GetFps() const;
	bool IsRunning() const;

signals:
	void frameSelected(unsigned int current_frame_number);

public slots:
	void IncreaseTimerInterval();
	void DecreaseTimerInterval();
	void StopTimer();
	void StartTimer();
	void UpdateTimer();
	void ReverseTimer();
	void SetCurrentFrameNumber(unsigned int value);

	void GotoPrevFrame();
	void GotoNextFrame();
	void CustomAdvanceFrame(unsigned int frame_amt);

private:
	static const std::vector<double> kSpeeds;

	int index_speed_ = 10;
	int timer_frequency_ = 1000 * 1 / kSpeeds[index_speed_];
	bool is_reverse_ = false;
	std::unique_ptr<QTimer> timer_;
	unsigned int current_frame_number_ = 0;
	unsigned int max_frame_number_ = 0;
};


#endif // PLAYBACK_CONTROLLER_H
