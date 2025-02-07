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
#include "constants.h"

using namespace std;
using namespace cv;

class  AutoTracking : public QObject
{
	Q_OBJECT

public:

    AutoTracking(); 
    ~AutoTracking();
    int N = 2;
    int nrows = SirveAppConstants::VideoDisplayHeight;
    int ncols = SirveAppConstants::VideoDisplayWidth;
    bool cancel_operation;
    void UpdateProgressBar(unsigned int value);
    arma::u64_mat SingleTracker(u_int track_id, double clamp_low_coeff, double clamp_high_coeff, int threshold, string prefilter, string tracktype, uint frame0, int start_frame, int stop_frame, processingState & current_processing_state, VideoDetails & base_processing_state_details, QString new_track_file_name);   
     
signals:
     void signalProgress(unsigned int frameval);

public slots:
    void CancelOperation();

private:
    void GetFrameRepresentations(int indx, double clampLow, double clampHigh, VideoDetails & state_details, VideoDetails & base_details, cv::Mat & frame, cv::Mat & processed_frame, cv::Mat & base_frame);
    void FilterImage(string filter_type, cv::Mat & input_image, cv::Mat & output_image);
    void GetTrackFeatureData(string trackFeature, int threshold, cv::Mat & frame_crop, cv::Mat & base_frame_crop, cv::Point & frame_point, double & peak_counts, cv::Scalar & sum_counts, cv::Scalar & sum_ROI_counts, uint & N_threshold_pixels, uint & N_ROI_pixels);
    void GetPointXY(cv::Point input_point, cv::Rect ROI, u_int & centerX,  u_int & centerY);
    void InitializeTracking(bool isrestart, u_int i, u_int indx, u_int num_frames, arma::u64_mat & output, cv::Rect & ROI, cv::Mat & filtered_frame_8bit_color, cv::Mat & frame, cv::Mat & frame_crop, cv::Mat & base_frame, cv::Mat & base_frame_crop, Ptr<Tracker> tracker);
    void GetValidROI(string window_name, cv::Rect & ROI, cv::Mat & filtered_frame_8bit_color_resize, arma::u64_mat & output);
};

#endif
