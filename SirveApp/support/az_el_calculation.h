#pragma once
#ifndef AZ_EL_CALCULATION_H
#define AZ_EL_CALCULATION_H

#include <armadillo>
#include <math.h>

#include "support/earth.h"

// referenced by AzElCalculation::calculate()
const int FOCAL_PLANE_ARRAY_WIDTH = 640;
const int FOCAL_PLANE_ARRAY_HEIGHT = 480;

namespace AzElCalculation
{
    std::vector<double> calculate(int x, int y, double sensor_lat, double sensor_long, std::vector<double> dcm, double ifov_x, double ifov_y, bool adjust_frame_ref, bool adjust_base, bool adjust_rounding_err_az, bool adjust_rounding_err_el);
};

#endif