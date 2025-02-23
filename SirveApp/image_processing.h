#pragma once
#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <armadillo>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <QWidget>
#include <QtWidgets>
#include <QTimer>

#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/videoio.hpp>

#include "abir_reader.h"
#include "tracks.h"
#include "video_details.h"
#include "constants.h"

class  ImageProcessing : public QObject
{
	Q_OBJECT

public:

    ImageProcessing();
    ~ImageProcessing();

    double min_deinterlace_dist;
    double max_deinterlace_dist;
    int deinterlace_kernel_size;
    int frameval;
    bool cancel_operation;
    int nRows = SirveAppConstants::VideoDisplayHeight;
    int nRows2 = nRows/2;
    int nCols = SirveAppConstants::VideoDisplayWidth;

    void ReplacePixelsWithNeighbors(std::vector<std::vector<uint16_t>> & original_pixels, std::vector<unsigned int> bad_pixel_indeces, int width_pixels);
    void UpdateProgressBar(unsigned int value);

    arma::uvec FindDeadBadscalePixels(std::vector<std::vector<uint16_t>>& input_pixels);
    arma::uvec IdentifyBadPixelsMedian(double N, std::vector<std::vector<uint16_t>> & input_pixels);
    arma::uvec IdentifyBadPixelsMovingMedian(int half_window_length, double N, std::vector<std::vector<uint16_t>> & input_pixels);

    std::vector<std::vector<uint16_t>> FixedNoiseSuppression(QString image_path, QString path_video_file, int frame0, int start_frame, int end_frame, double version, VideoDetails & original);
    std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppressionByFrame(int start_frame, int num_of_averaging_frames_input, VideoDetails & original);
    std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppressionMatrix(int start_frame, int num_of_averaging_frames, VideoDetails & original);
    std::vector<std::vector<uint16_t>> AccumulatorNoiseSuppression(double weight, int offset, int NThresh, VideoDetails & original, bool hide_shadow_choice);
    std::vector<std::vector<uint16_t>> RPCPNoiseSuppression(VideoDetails & original);
	std::vector<std::vector<uint16_t>> DeinterlaceOpenCVPhaseCorrelation(VideoDetails & original);
    std::vector<uint16_t> DeinterlacePhaseCorrelationCurrent(int current_frame, std::vector<uint16_t> & current_frame_16bit);

    std::vector<std::vector<uint16_t>> CenterOnTracks(QString trackTypePriority, VideoDetails & original, int OSM_track_id, int manual_track_id, std::vector<TrackFrame> osmFrames,\
        std::vector<TrackFrame> manualFrames, boolean findAnyTrack, std::vector<std::vector<int>> & track_centered_offsets);

    std::vector<std::vector<uint16_t>> CenterOnBrightest(VideoDetails & original, std::vector<std::vector<int>> & brightest_centered_offsets);
    std::vector<std::vector<uint16_t>> FrameStacking(int number_of_frames, VideoDetails & original);
    std::vector<std::vector<uint16_t>> MedianFilterStandard(VideoDetails & original, int window_size);
    std::vector<std::vector<uint16_t>> CenterImageFromOffsets(VideoDetails & original, std::vector<std::vector<int>> track_centered_offsets);

signals:
     void signalProgress(unsigned int frameval);

public slots:
    void CancelOperation();

private:

    ABIRData abir_data;
    arma::mat disk_avg_kernel;

    static arma::mat apply_shrinkage_operator(arma::mat s, double tau);
    void remove_shadow(int nRows, int nCols, arma::vec & frame_vector, int NThresh);
    static arma::mat perform_thresholding(arma::mat X, double tau);
};

#endif
