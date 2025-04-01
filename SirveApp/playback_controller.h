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
	explicit PlaybackController(QObject* parent = nullptr, uint32_t number_frames = 1);

	uint32_t GetCurrentFrameNumber() const;
	uint32_t GetMaxFrameNumber() const;
	void SetNumberOfFrames(uint32_t value);
	double GetFps() const;
	bool IsRunning() const;

signals:
	void frameSelected(uint32_t current_frame_number);

public slots:
	void IncreaseTimerInterval();
	void DecreaseTimerInterval();
	void StopTimer();
	void StartTimer();
	void UpdateTimer();
	void ReverseTimer();
	void SetCurrentFrameNumber(uint32_t value);

	void GotoPrevFrame();
	void GotoNextFrame();
	void CustomAdvanceFrame(uint32_t frame_amt);

private:
	static const std::vector<double> kSpeeds;

	int index_speed_ = 10;
	int timer_frequency_ = 1000 * 1 / kSpeeds[index_speed_];
	bool is_reverse_ = false;
	std::unique_ptr<QTimer> timer_;
	uint32_t current_frame_number_ = 0;
	uint32_t max_frame_number_ = 0;
};


#endif // PLAYBACK_CONTROLLER_H
