#pragma once
#ifndef AZ_EL_CALCULATION_H
#define AZ_EL_CALCULATION_H

#include <armadillo>
#include "constants.h"


namespace AzElCalculation
{
    std::vector<double> calculate(int nRows, int nCols, int x, int y, double sensor_lat, double sensor_long, std::vector<double> dcm, double ifov_x, double ifov_y, bool adjust_frame_ref);
    std::vector<int> calculateXY(int nRows, int nCols, double az, double el, double sensor_lat, double sensor_long, std::vector<double> dcm, double ifov_x, double ifov_y);
};

#endif
