#include "abpnuc_reader.h"

ABPNUC_Data::ABPNUC_Data(char* path)
{

    read_status = 0;
    full_file_path = path;

    read_apbnuc_file();

}

ABPNUC_Data::~ABPNUC_Data()
{
}

void ABPNUC_Data::read_apbnuc_file()
{
    // Outline for file read function provided by John Albritton

    INFO << "ABPNUC Load: Getting ABPNUC data";

    errno_t err = fopen_s(&fp, full_file_path, "rb");

    // if error in reading file, set read status to zero 
    if (err != 0) {
        read_status = 0;
        return;
    }

    number_of_frames = 0;
    while (true)
    {

        ABPNUC_Frame temp;

        // Load message header, break if emptyand eliminate last message
        uint64_t sec = ReadValue<uint64_t>();
        uint64_t nsec = ReadValue<uint64_t>();
        uint64_t tsize = ReadValue<uint64_t>();

        // check that tsize is zero or empty 
        if (tsize == 0)
            break;

        temp.seconds = sec + nsec * 1e-9;
        // msgHdr(n).size = tsize;
        // currPosition = ftell(fid);

        // Load the OSM
        std::vector<uint32_t> guid = ReadMultipleValuesIntoVector<uint32_t>(5);
        std::vector<uint32_t> source_guid = ReadMultipleValuesIntoVector<uint32_t>(5);
        uint16_t sensorId = ReadValue<uint16_t>();

        // skip 2 bytes here
        int buffer = ReadValue<int16_t>();
        temp.frame_number = ReadValue<uint32_t>();
        temp.frame_time = ReadValue<double>();
        temp.ir_temperature = ReadValue<uint32_t>();
        temp.tec_temperature_x100 = ReadValue<uint32_t>();
        temp.tec_temperature_t1_x100 = ReadValue<uint32_t>();
        temp.tec_temperature_t2_x100 = ReadValue<uint32_t>();
        temp.tec_temperature_t3_x100 = ReadValue<uint32_t>();
        temp.ambient = ReadValue<float>();
        temp.afocal1 = ReadValue<float>();
        temp.afocal2 = ReadValue<float>();
        temp.ir_atherm = ReadValue<float>();
        temp.ir_integration_time_usec = ReadValue<uint16_t>();

        // skip 2 bytes here
        buffer = ReadValue<int16_t>();
        temp.detector_temperature = ReadValue<double>();
        temp.nuc_environment = ReadValue<uint32_t>();
        temp.measured_det_cal_factor = ReadValue<uint32_t>();
        temp.scene_mean_t2 = ReadValue<uint32_t>();
        temp.scene_mean_t1 = ReadValue<uint32_t>();
        temp.scene_mean_t3 = ReadValue<uint32_t>();
        temp.scene_mean = ReadValue<uint32_t>();

        // add new abpnuc frame and add to frame counter
        data.push_back(temp);
        number_of_frames = number_of_frames + 1;
    }

    fclose(fp);
    read_status = 1;

    INFO << "ABPNUC Load: Data found for " << std::to_string(number_of_frames) << " frame(s)";

}
