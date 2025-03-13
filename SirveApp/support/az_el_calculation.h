#pragma once
#ifndef AZ_EL_CALCULATION_H
#define AZ_EL_CALCULATION_H

#include <armadillo>
#include "constants.h"

const int FOCAL_PLANE_ARRAY_WIDTH = SirveAppConstants::VideoDisplayWidth;
const int FOCAL_PLANE_ARRAY_HEIGHT = SirveAppConstants::VideoDisplayHeight;

namespace AzElCalculation
{
    std::vector<double> calculate(int x, int y, double sensor_lat, double sensor_long, std::vector<double> dcm, double ifov_x, double ifov_y, bool adjust_frame_ref);
};

#endif
