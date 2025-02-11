#pragma once
#ifndef SHARED_TRACKING_FUNCTIONS_H
#define SHARED_TRACKING_FUNCTIONS_H

#include "video_display.h"
#include <armadillo>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/photo.hpp>

namespace SharedTrackingFunctions
{
    double GetAdjustedCounts(int indx, cv::Rect boundingBox, VideoDetails & base_processing_state_details);
    void FindTargetExtent(cv::Mat &  display_image, int threshold, cv::Rect & ROI, cv::Rect & bbox);
};

#endif