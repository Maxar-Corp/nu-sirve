#pragma once
#ifndef SHARED SCIENTIFIC_FUNCTIONS_H
#define SHARED SCIENTIFIC_FUNCTIONS_H

#include "video_display.h"
#include <armadillo>

namespace IrradianceCountsCalc
{
    double ComputeIrradiance(int indx, int height2, int width2, int x, int y, VideoDetails & base_processing_state_details);
};

#endif