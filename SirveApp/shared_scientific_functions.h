#pragma once
#ifndef SHARED SCIENTIFIC_FUNCTIONS_H
#define SHARED SCIENTIFIC_FUNCTIONS_H

#include "video_display.h"
#include <armadillo>

namespace IrradianceCountsCalc
{
    double ComputeIrradiance(int indx, cv::Rect boundingBox, VideoDetails & base_processing_state_details);
};

#endif