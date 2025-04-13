#include "abpnuc_reader.h"

ABPNUCReader::~ABPNUCReader()
{
}

ABPNUCFrames ABPNUCReader::ReadFrames()
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
        ReadArray(frame.guid_source);
        frame.sensorId = Read<uint16_t>();

        // skip 2 bytes here
        Seek(2, SEEK_CUR);

        frame.frame_number = Read<int32_t>();
        frame.frame_time = Read<double>();
        frame.ir_temperature = Read<uint32_t>();
        frame.tec_temperature_x100 = Read<int32_t>();
        frame.tec_temperature_t1_x100 = Read<int32_t>();
        frame.tec_temperature_t2_x100 = Read<int32_t>();
        frame.tec_temperature_t3_x100 = Read<int32_t>();
        frame.ambient = Read<float>();
        frame.afocal1 = Read<float>();
        frame.afocal2 = Read<float>();
        frame.ir_atherm = Read<float>();
        frame.ir_integration_time_usec = Read<uint16_t>();

        // skip 2 bytes here
        Seek(2, SEEK_CUR);

        frame.detector_temperature = Read<double>();
        frame.nuc_environment = Read<uint32_t>();
        frame.measured_det_cal_factor = Read<uint32_t>();
        frame.scene_mean_t2 = Read<int32_t>();
        frame.scene_mean_t1 = Read<int32_t>();
        frame.scene_mean_t3 = Read<int32_t>();
        frame.scene_mean = Read<int32_t>();

        frames.emplace_back(frame);
    }

    return frames;
}
