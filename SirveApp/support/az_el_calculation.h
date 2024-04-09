#pragma once
#ifndef AZ_EL_CALCULATION_H
#define AZ_EL_CALCULATION_H

#include <armadillo>
#include <math.h>

#include "support/earth.h"

const int FOCAL_PLANE_ARRAY_WIDTH = 640;
const int FOCAL_PLANE_ARRAY_HEIGHT = 480;

const int OSM_CENTROID_SHIFT = 1;

namespace AzElCalculation
{
    std::vector<double> calculate(int x, int y, double sensor_lat, double sensor_long, std::vector<double> dcm, double ifov_x, double ifov_y, bool adjust_frame_ref);
};

#endif