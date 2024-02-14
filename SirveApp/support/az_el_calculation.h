#pragma once
#ifndef AZ_EL_CALCULATION_H
#define AZ_EL_CALCULATION_H

#include <armadillo>
#include <math.h>

#include "support/earth.h"

namespace AzElCalculation
{
    std::vector<double> calculate(int x, int y, double sensor_lat, double sensor_long, std::vector<double> dcm, double ifov_x, double ifov_y);
};

#endif