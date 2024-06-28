#pragma once
#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H
#include "video_details.h"
#include "tracks.h"

// #include <cmath>
// #include <vector>
// #include <set>
#include <armadillo>

#include <iostream>
#include <string>

#include <QWidget>
#include <QtWidgets>
#include <QTimer>

class  ImageProcessing : public QObject
{
	Q_OBJECT

public:

    ImageProcessing();
    ~ImageProcessing();
    int frameval;
    void UpdateProgressBar(unsigned int value);

	std::vector<std::vector<uint16_t>> DeinterlaceCrossCorrelation(VideoDetails & original);
    std::vector<std::vector<uint16_t>> CenterOnOSM(VideoDetails & original, int track_id, std::vector<TrackFrame> osmFrames, std::vector<std::vector<int>> & OSM_centered_offsets);
    std::vector<std::vector<uint16_t>> CenterOnManual(VideoDetails & original, int track_id, std::vector<TrackFrame> manualFrames, std::vector<std::vector<int>> & manual_centered_offsetsrogress);
    std::vector<std::vector<uint16_t>> CenterOnBrightest(VideoDetails & original, std::vector<std::vector<int>> & brightest_centered_offsets);
    std::vector<std::vector<uint16_t>> MedianFilterStandard(VideoDetails & original, int window_size);

signals:
     void SignalProgress(unsigned int frameval);

private:
	static arma::cx_mat xcorr2(arma::mat inFrame1, arma::mat inFrame2, int nRows, int nCols, int framei);	
};

#endif