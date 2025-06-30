#include "abpnuc_reader.h"

ABPNUCReader::~ABPNUCReader()
{
}

ABPNUCFrames ABPNUCReader::ReadFrames(ABPFileType file_type_)
{
    if (!IsOpen())
    {
        throw std::runtime_error("File is not open");
    }

    ABPNUCFrames frames;

    while (true)
    {
        ABPNUCFrame frame;

        // Load message header and break if empty and eliminate last message
        auto sec = Read<uint64_t>();
        auto nsec = Read<uint64_t>();
        auto tsize = Read<uint64_t>();

        // check that tsize is zero or empty
        if (tsize == 0)
        {
            break;
        }

        frame.seconds = static_cast<double>(sec) + static_cast<double>(nsec) * 1e-9;

        // load the OSM
        ReadArray(frame.guid);
        ReadArray(frame.source_guid);
        frame.sensorId = Read<uint16_t>();

        // skip 2 bytes here
        Seek(2, SEEK_CUR);

        frame.frameNumber = Read<int32_t>();
        frame.frameTime = Read<double>();
        frame.IR_TEMP_x100 = Read<uint32_t>();
        frame.TEC_Temperature_T0_x100 = Read<int32_t>();
        frame.TEC_Temperature_T1_x100 = Read<int32_t>();
        frame.TEC_Temperature_T2_x100 = Read<int32_t>();
        frame.TEC_Temperature_T3_x100 = Read<int32_t>();
        if (file_type_ == ABPFileType::ABP_D)
        {
            // frame.TEC_Temperature_T3_x100 = Read<int32_t>();
            frame.TEC_Temperature_T4_x100 = Read<int32_t>();
        }
        frame.Ambient = Read<float>();
        frame.Afocal1 = Read<float>();
        frame.Afocal2 = Read<float>();
        frame.IR_ATherm = Read<float>();
        frame.IR_Integration_Time_usec = Read<uint16_t>();

        // skip 2 bytes here
        Seek(2, SEEK_CUR);

        frame.Detector_Temperature = Read<double>();
        frame.NUC_Environment = Read<uint32_t>();
        frame.Measured_Det_Cal_Factor = Read<uint32_t>();
        if (file_type_ == ABPFileType::ABP_B)
        {
            frame.Scene_Mean_T2 = Read<int32_t>();
            frame.Scene_Mean_T1 = Read<int32_t>();
            frame.Scene_Mean_T3 = Read<int32_t>();
            frame.Scene_Mean = Read<int32_t>();
        }
        else if (file_type_ == ABPFileType::ABP_D)
        {
            frame.ImageMean = Read<double>();
            frame.numberOfTemps = Read<uint16_t>();
            frame.unusedShort = Read<uint16_t>();
            frame.TEC_Collected_T0_x100 = Read<uint32_t>();
            frame.TEC_Collected_T1_x100 = Read<uint32_t>();
            frame.TEC_Collected_T2_x100 = Read<uint32_t>();
            frame.TEC_Collected_T3_x100 = Read<uint32_t>();
            frame.TEC_Collected_T4_x100 = Read<uint32_t>();
            frame.TEC_Status_T0 = Read<uint16_t>();
            frame.TEC_Status_T1 = Read<uint16_t>();
            frame.TEC_Status_T2 = Read<uint16_t>();
            frame.TEC_Status_T3 = Read<uint16_t>();
            frame.TEC_Status_T4 = Read<uint16_t>();
        }

        frames.emplace_back(frame);
    }

    return frames;
}
