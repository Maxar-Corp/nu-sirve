#ifndef SHARED_TRACKING_FUNCTIONS_H
#define SHARED_TRACKING_FUNCTIONS_H


#include "video_display.h"
#include <armadillo>
#include "calibration_data.h"

namespace SharedTrackingFunctions
{
    std::array<double, 3> CalculateIrradiance(int indx, cv::Rect boundingBox, const VideoDetails & base_processing_state_details, double frame_integration_time, const CalibrationData&
                                              calibration_model);
    double GetAdjustedCounts(int indx, cv::Rect boundingBox, const VideoDetails & base_processing_state_details);
    void FindTargetExtent(int i, double & clamp_low_coeff, double & clamp_high_coeff, cv::Mat & frame, int threshold, int bbox_buffer_pixels, cv::Mat & frame_crop_threshold, cv::Rect & ROI, cv::Rect & bbox, arma::mat & offsets_matrix, cv::Rect & bbox_uncentered, int & extent_window_x, int & extent_window_y);
    void GetFrameRepresentations(uint & indx, double clamp_low_coeff, double clamp_high_coeff, const VideoDetails & current_processing_state_details, const VideoDetails & base_processing_state_details, cv::Mat & frame, std::string & prefilter, cv::Mat & display_frame, cv::Mat & raw_display_frame, cv::Mat & clean_display_frame, cv::Mat & raw_frame);
    void FilterImage(std::string & prefilter, cv::Mat & display_frame, cv::Mat & clean_display_frame);
    void GetTrackPointData(std::string & trackFeature, cv::Mat & frame_crop, cv::Mat & raw_frame_bbox, cv::Mat & frame_bbox_threshold, cv::Point & frame_point, double & peak_counts, double & mean_counts, cv::Scalar & sum_counts, uint32_t & number_pixels);
    void GetPointXY(cv::Point input_point, cv::Rect ROI, int & centerX,  int & centerY);
    void CheckROI(cv::Rect & ROI, bool & valid_ROI);
    void CreateOffsetMatrix(int start_frame, int stop_frame, const ProcessingState & state_details, arma::mat & offsets_matrix);
};

#endif