#pragma once
#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H
#include "video_details.h"
#include "tracks.h"
#include "abir_reader.h"
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

    std::vector<std::vector<uint16_t>> FixedNoiseSuppression(QString image_path, QString path_video_file, int start_frame, int end_frame, double version, VideoDetails & original);

    std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppressionByFrame(int start_frame, int num_of_averaging_frames_input, int NThresh, VideoDetails & original, QString & hide_shadow_choice);

    // std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppressionMatrix(int start_frame, int number_of_frames, VideoDetails & original,  QString & hide_shadow_choice);

    std::vector<std::vector<uint16_t>> RPCPNoiseSuppression(VideoDetails & original);

	std::vector<std::vector<uint16_t>> DeinterlaceCrossCorrelation(std::vector<Frame> osm_frames,VideoDetails & original);

    std::vector<uint16_t> DeinterlaceCrossCorrelationCurrent(int current_frame,  int nRows, int nCols, std::vector<uint16_t> & current_frame_16bit);

    std::vector<std::vector<uint16_t>> CenterOnTracks(QString trackTypePriority, VideoDetails & original, int track_id, std::vector<TrackFrame> osmFrames, std::vector<TrackFrame> manualFrames, boolean findAnyTrack, std::vector<std::vector<int>> & track_centered_offsets);

    std::vector<std::vector<uint16_t>> CenterOnBrightest(VideoDetails & original, std::vector<std::vector<int>> & brightest_centered_offsets);

    std::vector<std::vector<uint16_t>> FrameStacking(int number_of_frames, VideoDetails & original);

    std::vector<std::vector<uint16_t>> MedianFilterStandard(VideoDetails & original, int window_size);


signals:
     void SignalProgress(unsigned int frameval);

private:
    arma::mat disk_avg_kernel;

    ABIRData abir_data;

    void remove_shadow(int nRows, int nCols, arma::vec & frame_vector, arma::mat window_data, int NThresh, int num_of_averaging_frames);

    static arma::cx_mat xcorr2(arma::mat inFrame1, arma::mat inFrame2, int nRows, int nCols);

    static arma::mat thresholding(arma::mat X, double tau);	

    static arma::mat shrink(arma::mat s, double tau);

};

#endif