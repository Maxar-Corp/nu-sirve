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
#include "shared_tracking_functions.h"
#include "constants.h"
#include "abir_reader.h"
#include "calibration_data.h"

using namespace std;
using namespace cv;

class  AutoTracking : public QObject
{
	Q_OBJECT

public:

    AutoTracking() = default;
    int image_scale_factor = 2;
    int step_success_coefficient = 3;
    int nrows = SirveAppConstants::VideoDisplayHeight;
    int ncols = SirveAppConstants::VideoDisplayWidth;
    int SirveApp_x, SirveApp_y, Display_res_x, Display_res_y;
    int ROI_window_x, ROI_window_y, tracking_window_x, tracking_window_y, raw_window_x, raw_window_y, extent_window_x, extent_window_y;
    bool cancel_operation = false;
    void UpdateProgressBar(unsigned int value);
    cv::Mat raw_display_frame;
    arma::s32_mat SingleTracker(QSize screenResolution, QPoint appPos, u_int track_id, double clamp_low_coeff, double clamp_high_coeff, int threshold, int
                                bbox_buffer_pixels, string prefilter, string trackFeature, uint frame0, uint start_frame, uint stop_frame, const ProcessingState&
                                current_processing_state, const VideoDetails& base_processing_state_details, const std::vector<ABIRFrameHeader>& input_frame_header, const
                                CalibrationData& calibration_model);
    void SetCalibrationModel(CalibrationData input);

signals:
     void signalProgress(unsigned int frameval);

public slots:
    void CancelOperation();

private:
    CalibrationData model;

    void InitializeTracking(
        bool isRestart,
        u_int i,
        u_int indx,
        u_int frame0,
        double clamp_low_coeff,
        double clamp_high_coeff,
        const VideoDetails& current_processing_state_details,
        const VideoDetails& base_processing_state_details,
        string prefilter,
        cv::Mat & display_frame,
        cv::Mat & raw_display_frame,
        cv::Mat & clean_display_frame,
        cv::Rect & ROI,
        bool &valid_ROI,
        cv::Mat & frame,
        cv::Mat & frame_crop,
        cv::Mat & raw_frame,
        Ptr<Tracker> tracker,
        string & choice,
        arma::running_stat<double> & stats
    );
 
    void TrackingStep(
        int & i,
        uint & indx,
        uint & track_id,
        uint & frame0,
        double & clamp_low_coeff,
        double & clamp_high_coeff,
        const ProcessingState& current_processing_state,
        const VideoDetails& base_processing_state_details,
        const std::vector<ABIRFrameHeader>& input_frame_header,
        string & prefilter,
        Ptr<Tracker> & tracker,
        string & trackFeature,
        cv::Mat & display_frame,
        cv::Mat & raw_display_frame,
        cv::Mat & clean_display_frame,
        int & threshold,
        int & bbox_buffer_pixels,
        cv::Rect & ROI,
        cv::Mat & frame,
        cv::Mat & raw_frame,
        cv::Mat & raw_frame_bbox,
        cv::Point & frame_point,
        arma::running_stat<double> & stats,
        bool & step_success,
        double & S,
        double & peak_counts,
        double & mean_counts,
        cv::Scalar & sum_counts,
        uint & number_pixels,
        arma::mat & offsets_matrix,
        arma::s32_mat & output,
        double & adjusted_integrated_counts_old,
        const CalibrationData& calibration_model
    );

    void GetROI(string window_name, cv::Rect & ROI, cv::Mat & filtered_frame_8bit_color_resize);
};

#endif
