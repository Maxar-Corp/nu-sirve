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

// Define a custom hash function for the enum (only needed for unordered_map)
namespace std {
template <>
struct hash<ChartType> {
    std::size_t operator()(const ChartType& f) const {
        return std::hash<int>()(static_cast<int>(f));
    }
};
}

// Define a class that contains a lookup table
class ChartRanges {
public:
    ChartRanges() {
        ranges[IRRADIANCE];
        ranges[AZIMUTH];
        ranges[ELEVATION];
        ranges[IFOV_X];
        ranges[IFOV_Y];
        ranges[BORESIGHT_AZIMUTH];
        ranges[BORESIGHT_ELEVATION];
    }

    ChartBoundingBox getValue(ChartType key) const
    {
        auto it = ranges.find(key);
        if (it != ranges.end()) {
            return it->second;
        } else {
            throw std::out_of_range("Key not found in lookup table.");
        }
    }

    void setValue(ChartType key, ChartBoundingBox value)
    {
        ranges[key] = value;
    }

private:
    std::unordered_map<ChartType, ChartBoundingBox> ranges;
};

#endif // CHART_METADATA_H
