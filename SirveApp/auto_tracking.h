#pragma once
#ifndef AUTO_TRACKING_H
#define AUTO_TRACKING_H

#include <armadillo>
#include <iostream>
#include <string>
#include <QWidget>
#include <QtWidgets>
#include <QTimer>
#include <QDialog>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/photo.hpp>
#include "support/qthelpers.h"
#include "tracks.h"
#include "video_details.h"
#include "processing_state.h"
#include "shared_scientific_functions.h"

using namespace std;
using namespace cv;

class  AutoTracking : public QObject
{
	Q_OBJECT

public:

    AutoTracking(); 
    ~AutoTracking();
    int N = 2;
    int nrows = 480;
    int ncols = 640;
    bool cancel_operation;
    void UpdateProgressBar(unsigned int value);
    arma::u64_mat SingleTracker(u_int track_id, double clamp_low, double clamp_high, int threshold, string prefilter, string tracktype, uint frame0, int start_frame, int stop_frame,\
     processingState & current_processing_state, VideoDetails & base_processing_state_details, QString new_track_file_name);   
     
signals:
     void signalProgress(unsigned int frameval);

public slots:
    void CancelOperation();

private:
    void FilterImage(string filter_type, cv::Mat & input_image, cv::Mat & output_image);
    void GetTrackFeatureData(string trackFeature, int threshold, cv::Mat frame_crop, cv::Point & frame_point, cv::Scalar frame_crop_mean, double & peak_counts, cv::Scalar & sum_counts,\
     cv::Scalar & sum_ROI_counts, uint & N_threshold_pixels,  uint & N_ROI_pixels);
    void GetPointXY(cv::Point input_point, cv::Rect ROI, u_int & centerX,  u_int & centerY);
    void GetProcessedFrameMatrix(int indx, double clampLow, double clampHigh, VideoDetails & original,  arma::vec & frame_vector, cv::Mat & frame_matrix, cv::Mat & processed_frame_matrix);
    void HandleInterruption(QMessageBox::StandardButton& response, u_int i, u_int indx, u_int num_frames, arma::u64_mat & output, cv::Rect & ROI, cv::Mat &filtered_frame_i_matrix_8bit_color_resize);
};

#endif
