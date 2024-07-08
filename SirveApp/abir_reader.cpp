#include "abir_reader.h"

ABIRData::ABIRData()
{
}

ABIRData::~ABIRData()
{
}

ABIRDataResult ABIRData::GetFrames(const char* file_path, unsigned int min_frame, unsigned int max_frame, double version_number, bool header_only)
{
    ABIRDataResult data_result;
    data_result.had_error = false;

    int check_value = FileSetup(file_path, version_number);
	if (check_value < 0) {
        data_result.had_error = true;
		return data_result;
	}

    std::vector<unsigned int> valid_frames { min_frame, max_frame };

	std::vector<std::vector<uint16_t>>video_frames_16bit;

	fopen_s(&fp, full_file_path, "rb");

    fseek(fp, 16, SEEK_SET);
    uint64_t frame_size = ReadValue<uint64_t>();

    if (file_version >= 2.1)
        frame_size += 40;
    else
        frame_size += 32;

    struct _stat64 info;
    _stat64(full_file_path, &info);
    uint64_t file_size = info.st_size;

    uint32_t number_frames_file = file_size / frame_size;

    //Check that minimum frame number is smaller than max frame number
    if (valid_frames[0] > valid_frames[1]) {
        uint32_t temp = valid_frames[1];
        valid_frames[1] = valid_frames[0];
        valid_frames[0] = temp;
    }

    if (valid_frames[1] > number_frames_file)
        valid_frames[1] = number_frames_file;

	if (valid_frames[0] < 1)
		valid_frames[0] = 1;

    ir_data.reserve(valid_frames[1] - valid_frames[0] + 1);

    for (unsigned int frame_index = valid_frames[0]; frame_index <= valid_frames[1]; frame_index++)
    {
        ABIR_Header header_data;

		size_t seek_rtn = _fseeki64(fp, (frame_index - 1) * frame_size, SEEK_SET);

        uint64_t temp_seconds = ReadValue<uint64_t>();
        uint64_t temp_nano_seconds = ReadValue<uint64_t>();
        uint64_t temp_size = ReadValue<uint64_t>();

        // Break from empty frames
		if (temp_size == 0) {
			break;
		}

        header_data.seconds = temp_seconds + temp_nano_seconds * 10e-9;
        header_data.size = temp_size;
        header_data.image_size_double = ReadValue<uint64_t>();

        if (file_version >= 2.1)
        {
            header_data.msg_type = ReadValue<uint32_t>();
            header_data.msg_version = ReadValue<float>();
        }

        header_data.guid = ReadMultipleValuesIntoVector<uint32_t>(5);
        header_data.guid_source = ReadMultipleValuesIntoVector<uint32_t>(5);

        // Skipped section only relevant for versions less than or equal to 3.0

        header_data.sensor_id = ReadValue<uint16_t>();
        fseek(fp, 2, SEEK_CUR);
        header_data.frame_number = ReadValue<uint32_t>();

        header_data.frame_time = ReadValue<double>();
        header_data.image_x_size = ReadValue<uint16_t>();
        header_data.image_y_size = ReadValue<uint16_t>();

        header_data.pixel_depth = ReadValue<uint16_t>();
        header_data.bits_per_pixel = ReadValue<uint16_t>();

        // Skipped section only relevant for versions less than or equal to 3.0

        if (file_version >= 3.1) {
            header_data.int_time = ReadValue<float>();
        }

        header_data.sensor_fov = ReadValue<uint32_t>();

        if (file_version >= 2.1) {
            header_data.intended_fov = ReadValue<uint32_t>();
        }

        if (file_version >= 2) {
            header_data.focus = ReadValue<uint16_t>();
        }

        // Skipped check for fseek when file version is < 2.1
        fseek(fp, 2, SEEK_CUR);

        header_data.alpha = ReadValue<double>();
        header_data.beta = ReadValue<double>();

        if (file_version >= 5) {
            header_data.g_roll = ReadValue<double>();
        }

        header_data.alpha_dot = ReadValue<double>();
        header_data.beta_dot = ReadValue<double>();

        if (file_version >= 5) {
            header_data.g_roll_rate = ReadValue<double>();
        }

        if (file_version >= 3) {

            if (file_version == 3) {
                header_data.p_heading = ReadValue<double>();
                header_data.p_pitch = ReadValue<double>();
                header_data.p_roll = ReadValue<double>();
            }

            header_data.p_lla = ReadMultipleValuesIntoVector<double>(3);

            if (file_version == 3)
                header_data.p_alt_gps = ReadValue<double>();

            if (file_version >= 3.1) {
                header_data.p_ypr = ReadMultipleValuesIntoVector<double>(3);

                if (file_version >= 4.1)
                    header_data.p_ypr_dot = ReadMultipleValuesIntoVector<double>(3);
            }

            header_data.p_vel_xyz = ReadMultipleValuesIntoVector<double>(3);

            if (file_version == 3) {
                header_data.p_heading_mag = ReadValue<double>();
                header_data.p_heading_ref = ReadValue<uint32_t>();
            }

            // Skipped reading in 3 values of type double which is applicable to file versions < 4.0
        }

        if (file_version >= 4.0) {

            header_data.one.frame_time = ReadValue<double>();
            header_data.two.frame_time = ReadValue<double>();

            header_data.one.alpha = ReadValue<double>();
            header_data.one.beta = ReadValue<double>();
            header_data.one.alpha_dot = ReadValue<double>();
            header_data.one.beta_dot = ReadValue<double>();

            header_data.two.alpha = ReadValue<double>();
            header_data.two.beta = ReadValue<double>();
            header_data.two.alpha_dot = ReadValue<double>();
            header_data.two.beta_dot = ReadValue<double>();

            header_data.one.imc_az = ReadValue<double>();
            header_data.one.imc_el = ReadValue<double>();

            header_data.two.imc_az = ReadValue<double>();
            header_data.two.imc_el = ReadValue<double>();

            header_data.one.ars_ypr = ReadMultipleValuesIntoVector<double>(3);
            header_data.two.ars_ypr = ReadMultipleValuesIntoVector<double>(3);

            header_data.one.imu_count = ReadValue<uint32_t>();
            for (size_t i = 0; i < 8; i++)
            {
                std::vector<float> temp1 = ReadMultipleValuesIntoVector<float>(3);
                std::vector<float> temp2 = ReadMultipleValuesIntoVector<float>(3);

                header_data.one.imu_angle.push_back(temp1);
                header_data.one.imu_vel.push_back(temp2);
            }

            header_data.two.imu_count = ReadValue<uint32_t>();
            for (size_t i = 0; i < 8; i++)
            {
                std::vector<float> temp1 = ReadMultipleValuesIntoVector<float>(3);
                std::vector<float> temp2 = ReadMultipleValuesIntoVector<float>(3);

                header_data.two.imu_angle.push_back(temp1);
                header_data.two.imu_vel.push_back(temp2);
            }

            if (file_version >= 4.2) {
                header_data.one.imu_sum = ReadMultipleValuesIntoVector<double>(3);
                header_data.two.imu_sum = ReadMultipleValuesIntoVector<double>(3);
            }

            header_data.one.p_lla = ReadMultipleValuesIntoVector<double>(3);
            header_data.one.p_ypr = ReadMultipleValuesIntoVector<double>(3);
            if (file_version >= 4.1)
                header_data.one.p_ypr_dot = ReadMultipleValuesIntoVector<double>(3);
            header_data.one.p_vel = ReadMultipleValuesIntoVector<double>(3);

            header_data.two.p_lla = ReadMultipleValuesIntoVector<double>(3);
            header_data.two.p_ypr = ReadMultipleValuesIntoVector<double>(3);
            if (file_version >= 4.1)
                header_data.two.p_ypr_dot = ReadMultipleValuesIntoVector<double>(3);
            header_data.two.p_vel = ReadMultipleValuesIntoVector<double>(3);

            header_data.one.fpa_ypr = ReadMultipleValuesIntoVector<double>(3);
            header_data.two.fpa_ypr = ReadMultipleValuesIntoVector<double>(3);

            header_data.fpa_gain = ReadValue<float>();

            fseek(fp, 4, SEEK_CUR);

            if (file_version >= 4.1) {
                header_data.temp_k = ReadValue<float>();
                header_data.pressure = ReadValue<float>();
                header_data.relative_humidity = ReadValue<float>();
            }

        }

        header_data.image_origin = ReadValue<uint32_t>();
        header_data.image_size = ReadValue<uint32_t>();

        if (header_data.image_size > 20e6) {
            //WARN << "ABIR Load: Image size exceeds max allowable. Check version type.";
            fclose(fp);

            data_result.x_pixels = ir_data[0].header.image_x_size;
            data_result.y_pixels = ir_data[0].header.image_y_size;
            data_result.video_frames_16bit = video_frames_16bit;
            return data_result;
        }

        //uint16_t total_range = 65535 / 4;

		uint16_t *raw_16bit_data = new uint16_t[header_data.image_size];
		

        if (! header_only)	{
			ReadMultipleValues(raw_16bit_data, false, header_data.image_size);
        }

        ABIR_Frame temp_frame;
        temp_frame.header = header_data;
        
		std::vector<uint16_t> values(raw_16bit_data, raw_16bit_data + header_data.image_size);
		video_frames_16bit.push_back(values);
		delete[] raw_16bit_data;

        ir_data.push_back(temp_frame);
    }

    fclose(fp);

    data_result.x_pixels = ir_data[0].header.image_x_size;
    data_result.y_pixels = ir_data[0].header.image_y_size;
    data_result.video_frames_16bit = video_frames_16bit;
    return data_result;
}

int ABIRData::FileSetup(const char* file_path, double version_number)
{
    errno_t err = fopen_s(&fp, file_path, "rb");

    if (err != 0) {
        return err;
    }

	if (version_number > 0) {
        //INFO << "ABIR Load: File version is being overridden to " << version_number;
        file_version = version_number;
	}
    else {
        file_version = GetVersionNumberFromFile();
        if (file_version < 0) {
            return -1;
        }
    }

    full_file_path = file_path;

    fclose(fp);

    return err;
}

double ABIRData::GetVersionNumberFromFile()
{
    fseek(fp, 36, SEEK_SET);
    double version_number = ReadValue<double>(true);  //TODO matlab code has extra code for manipulating version number. double check this is correct

	bool ok;
	double version_number_entered = QInputDialog::getDouble(nullptr, "Override File Version", "Enter File Version to Use:", version_number, 1, 4.2, 2, &ok);

	version_number = version_number_entered;

	if (!ok) {
		//INFO << "User selected 'Cancel' for version. Import of video files canceled";
		return -1;
	}

	if (version_number < 1 || version_number > 20) {
		return -1;
	}

    if (version_number == 2.5)
        version_number = 3.0;

    return version_number;
}
