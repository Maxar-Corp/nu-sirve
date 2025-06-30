#ifndef ABPNUC_READER_H
#define ABPNUC_READER_H

#include "binary_reader.h"
#include "abir_reader.h"

struct ABPNUCFrame
{
    int frameNumber;
    uint32_t IR_TEMP_x100;
    uint32_t NUC_Environment;
    uint32_t Measured_Det_Cal_Factor;
    int32_t TEC_Temperature_T0_x100;
    int32_t TEC_Temperature_T1_x100;
    int32_t TEC_Temperature_T2_x100;
    int32_t TEC_Temperature_T3_x100;
    int32_t TEC_Temperature_T4_x100;
    uint32_t Scene_Mean_T1;
    uint32_t Scene_Mean_T2;
    uint32_t Scene_Mean_T3;
    uint32_t Scene_Mean;
    double frameTime;
    double seconds;
    float Ambient;
    float Afocal1;
    float Afocal2;
    float IR_ATherm;
    double IR_Integration_Time_usec; 
    double Detector_Temperature;
    uint32_t guid[5];
    uint32_t source_guid[5];
    uint16_t sensorId;


    double ImageMean;
    uint16_t numberOfTemps;
    uint16_t unusedShort;
    uint32_t TEC_Collected_T0_x100;
    uint32_t TEC_Collected_T1_x100;
    uint32_t TEC_Collected_T2_x100;
    uint32_t TEC_Collected_T3_x100;
    uint32_t TEC_Collected_T4_x100;
    uint16_t TEC_Status_T0;
    uint16_t TEC_Status_T1;
    uint16_t TEC_Status_T2;
    uint16_t TEC_Status_T3;
    uint16_t TEC_Status_T4;
};

using ABPNUCFrames = std::vector<ABPNUCFrame>;

class ABPNUCReader : public BinaryReader
{
public:
    ABPNUCReader() = default;
    ~ABPNUCReader() override;

    ABPNUCFrames ReadFrames(ABPFileType file_type_);

private:
    ABPFileType file_type_ = ABPFileType::UNKNOWN;
};

#endif // ABPNUC_READER_H
