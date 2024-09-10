#ifndef CHART_METADATA_H
#define CHART_METADATA_H

#include <stdexcept>
#include <unordered_map>

// Define an enumerated type
enum ChartType {
    IRRADIANCE,
    AZIMUTH,
    ELEVATION,
    IFOV_X,
    IFOV_Y,
    BORESIGHT_AZIMUTH,
    BORESIGHT_ELEVATION
};

struct ChartBoundingBox {
    float axis_x_min;
    float axis_x_max;
    float axis_y_min;
    float axis_y_max;
};

#endif // CHART_METADATA_H
