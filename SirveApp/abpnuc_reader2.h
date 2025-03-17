#ifndef ABPNUC_READER2_H
#define ABPNUC_READER2_H

#include "binary_reader.h"

struct ABPNUCFrame2
{
    int frame_number;
    uint32_t ir_temperature;
    uint32_t nuc_environment;
    uint32_t measured_det_cal_factor;
    int32_t tec_temperature_x100;
    int32_t tec_temperature_t1_x100;
    int32_t tec_temperature_t2_x100;
    int32_t tec_temperature_t3_x100;
    int32_t scene_mean_t1;
    int32_t scene_mean_t2;
    int32_t scene_mean_t3;
    int32_t scene_mean;
    double frame_time;
    double seconds;
    double ambient;
    double afocal1;
    double afocal2;
    double ir_atherm;
    double ir_integration_time_usec;
    double detector_temperature;

    // new fields
    uint32_t guid[5];
    uint32_t guid_source[5];
    uint16_t sensorId;
};

using ABPNUCFrames = std::vector<ABPNUCFrame2>;

class ABPNUCReader : public BinaryReader
{
public:
    ABPNUCReader() = default;
    ~ABPNUCReader() override;

    ABPNUCFrames ReadFrames();
};

#endif // ABPNUC_READER2_H
