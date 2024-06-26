#pragma once
#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H
#include "video_details.h"
#include "tracks.h"

#include <cmath>
#include <vector>
#include <set>
#include <armadillo>
#include <QProgressDialog>


class ImageProcessing
{
private:
	static arma::cx_mat xcorr2(arma::mat inFrame1, arma::mat inFrame2, int nRows, int nCols, int framei);	

public:
	std::vector<std::vector<uint16_t>> DeinterlaceCrossCorrelation(VideoDetails & original, QProgressDialog & progress);
    std::vector<std::vector<uint16_t>> CenterOnOSM(VideoDetails & original, int track_id, std::vector<TrackFrame> osmFrames, std::vector<std::vector<int>> & OSM_centered_offsets, QProgressDialog & progress);
    std::vector<std::vector<uint16_t>> CenterOnManual(VideoDetails & original, int track_id, std::vector<TrackFrame> manualFrames, std::vector<std::vector<int>> & manual_centered_offsets, QProgressDialog & progress);
    std::vector<std::vector<uint16_t>> MedianFilterStandard(VideoDetails & original, int window_size, QProgressDialog & progress);
};


#endif