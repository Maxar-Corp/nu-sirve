#pragma once
#ifndef AUTO_TRACKING_H
#define AUTO_TRACKING_H
#include "video_details.h"
#include "tracks.h"
#include <armadillo>
#include <iostream>
#include <string>
#include <QWidget>
#include <QtWidgets>
#include <QTimer>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

class  AutoTracking : public QObject
{
	Q_OBJECT

public:

    AutoTracking();
    ~AutoTracking();


    bool cancel_operation;
    void UpdateProgressBar(unsigned int value);

    // TrackDetails SingleTracker(int start_frame, int stop_frame, VideoDetails & original);

    void SingleTracker(int frame0, int start_frame, int stop_frame, VideoDetails original);

signals:
     void SignalProgress(unsigned int frameval);

public slots:
    void CancelOperation();

private:


};

#endif