#include "abir_reader.h"

#include "qcoreapplication.h"
#include "qinputdialog.h"

static constexpr auto VERSION_NUMBER_OFFSET = 36;
static constexpr auto FRAME_SIZE_OFFSET = 16;
static constexpr auto MIN_VERSION_NUMBER = 1.0;
static constexpr auto MAX_VERSION_NUMBER = 20.0;
static constexpr auto FRAME_SIZE_INCREMENT = 32;
static constexpr auto FRAME_SIZE_INCREMENT_2_1 = 40;

ABIRFrames::ABIRFrames() :
    x_pixels(0), y_pixels(0), max_value(0), last_valid_frame(0)
{
}

bool ABIRReader::Open(const char* filename, double version_number)
{
    if (!BinaryReader::Open(filename))
    {
        return false;
    }

    if (version_number > 0.0)
    {
        file_version_ = version_number;
    }
    else
    {
        Seek(VERSION_NUMBER_OFFSET, SEEK_SET);
        file_version_ = Read<double>(true);
    }

    bool ok;
    double version_number_entered = QInputDialog::getDouble(nullptr, "Override File Version",
                                                            "Enter File Version to Use:",
                                                            version_number,
                                                            1, 4.2, 2, &ok);
    if (!ok)
    {
        // INFO << "User selected 'Cancel' for version. Import of video files canceled"
        return false;
    }

    file_version_ = version_number_entered;

    if (file_version_ < MIN_VERSION_NUMBER || file_version_ > MAX_VERSION_NUMBER)
    {
        return false;
    }

    if (file_version_ == 2.5)
    {
        file_version_ = 3.0;
    }

    return true;
}

ABIRFrames::Ptr ABIRReader::ReadFrames(uint32_t min_frame, uint32_t max_frame, bool header_only)
{
    if (!IsOpen())
    {
        throw std::runtime_error("File is not open.");
    }

    auto frames = std::make_unique<ABIRFrames>();

    std::vector<std::vector<uint16_t>> video_frames_16bit;

    Seek(FRAME_SIZE_OFFSET, SEEK_SET);

    auto frame_size = Read<uint64_t>();
    if (file_version_ >= 2.1)
    {
        frame_size += FRAME_SIZE_INCREMENT_2_1;
    }
    else
    {
        frame_size += FRAME_SIZE_INCREMENT;
    }

    auto file_size = FileSize();
    if (file_size == 0)
    {
        throw std::runtime_error("File size is zero.");
    }

    uint32_t num_frames = static_cast<uint32_t>(file_size / frame_size);

    // Ensure that minimum frame number is smaller than max frame number
    if (max_frame < min_frame)
    {
        auto tmp = max_frame;
        max_frame = min_frame;
        min_frame = tmp;
    }

    // Ensure reasonable upper and lower bounds
    max_frame = std::min(max_frame, num_frames);
    min_frame = std::max(min_frame, 1u);

    frames->ir_data.reserve(max_frame - min_frame + 1);

    // Initialize with the smallest possible integer
    auto max_val = std::numeric_limits<uint16_t>::min();
    uint16_t last_valid_frame = 0;
    auto frame_span = static_cast<double>(max_frame - min_frame);

    for (auto frame_index = min_frame; frame_index <= max_frame; ++frame_index)
    {
        ABIRFrameHeader header_data{};

        Seek(static_cast<int64_t>((frame_index - 1) * frame_size), SEEK_SET);

        auto temp_seconds = Read<uint64_t>();
        auto temp_nano_seconds = Read<uint64_t>();
        auto temp_size = Read<uint64_t>();

        if (temp_size == 0)
        {
            break; // break from empty frames
        }

        header_data.seconds = temp_seconds + static_cast<uint64_t>(static_cast<double>(temp_nano_seconds) * 10e-9);

        header_data.size = temp_size;
        header_data.image_size_double = Read<uint64_t>();

        if (file_version_ >= 2.1)
        {
            header_data.msg_type = Read<uint32_t>();
            header_data.msg_version = Read<float>();
        }

        ReadArray(header_data.guid);
        ReadArray(header_data.guid_source);

        // Skipped section only relevant for versions less than or equal to 3.0

        header_data.sensor_id = Read<uint16_t>();
        Seek(2, SEEK_CUR);
        header_data.frame_number = Read<uint32_t>();

        header_data.frame_time = Read<double>();
        header_data.image_x_size = Read<uint16_t>();
        header_data.image_y_size = Read<uint16_t>();

        header_data.pixel_depth = Read<uint16_t>();
        header_data.bits_per_pixel = Read<uint16_t>();

        // Skipped section only relevant for versions less than or equal to 3.0

        if (file_version_ >= 3.1)
        {
            header_data.int_time = Read<float>();
        }

        header_data.sensor_fov = Read<uint32_t>();

        if (file_version_ >= 2.1)
        {
            header_data.intended_fov = Read<uint32_t>();
        }

        if (file_version_ >= 2)
        {
            header_data.focus = Read<uint16_t>();
        }

        // Skipped check for seek when file version is < 2.1
        Seek(2, SEEK_CUR);

        header_data.alpha = Read<double>();
        header_data.beta = Read<double>();

        if (file_version_ >= 5)
        {
            header_data.g_roll = Read<double>();
        }

        header_data.alpha_dot = Read<double>();
        header_data.beta_dot = Read<double>();

        if (file_version_ >= 5)
        {
            header_data.g_roll_rate = Read<double>();
        }

        if (file_version_ >= 3)
        {
            if (file_version_ == 3.0)
            {
                header_data.p_heading = Read<double>();
                header_data.p_pitch = Read<double>();
                header_data.p_roll = Read<double>();
            }

            ReadArray(header_data.p_lla);

            if (file_version_ == 3.0)
            {
                header_data.p_alt_gps = Read<double>();
            }

            if (file_version_ >= 3.1)
            {
                ReadArray(header_data.p_ypr);
            }

            if (file_version_ >= 4.1)
            {
                ReadArray(header_data.p_ypr_dot);
            }

            ReadArray(header_data.p_vel_xyz);

            if (file_version_ == 3.0)
            {
                header_data.p_heading_mag = Read<double>();
                header_data.p_heading_ref = Read<uint32_t>();
            }

            // Skipped reading in 3 values of type double which is applicable to file
            // versions < 4.0
        }

        if (file_version_ >= 4.0)
        { // Read object data
            header_data.one.frame_time = Read<double>();
            header_data.two.frame_time = Read<double>();

            header_data.one.alpha = Read<double>();
            header_data.one.beta = Read<double>();
            header_data.one.alpha_dot = Read<double>();
            header_data.one.beta_dot = Read<double>();

            header_data.two.alpha = Read<double>();
            header_data.two.beta = Read<double>();
            header_data.two.alpha_dot = Read<double>();
            header_data.two.beta_dot = Read<double>();

            header_data.one.imc_az = Read<double>();
            header_data.one.imc_el = Read<double>();

            header_data.two.imc_az = Read<double>();
            header_data.two.imc_el = Read<double>();

            ReadArray(header_data.one.ars_ypr);
            ReadArray(header_data.two.ars_ypr);

            header_data.one.imu_count = Read<uint32_t>();
            ReadArray(header_data.one.imu_data);

            header_data.two.imu_count = Read<uint32_t>();
            ReadArray(header_data.two.imu_data);

            if (file_version_ >= 4.2)
            {
                ReadArray(header_data.one.imu_sum);
                ReadArray(header_data.two.imu_sum);
            }

            ReadArray(header_data.one.p_lla);
            ReadArray(header_data.one.p_ypr);

            if (file_version_ >= 4.1)
            {
                ReadArray(header_data.one.p_ypr_dot);
            }
            ReadArray(header_data.one.p_vel);

            ReadArray(header_data.two.p_lla);
            ReadArray(header_data.two.p_ypr);

            if (file_version_ >= 4.1)
            {
                ReadArray(header_data.two.p_ypr_dot);
            }
            ReadArray(header_data.two.p_vel);

            ReadArray(header_data.one.fpa_ypr);
            ReadArray(header_data.two.fpa_ypr);

            header_data.fpa_gain = Read<float>();

            Seek(4, SEEK_CUR);

            if (file_version_ >= 4.1)
            {
                header_data.temp_k = Read<float>();
                header_data.pressure = Read<float>();
                header_data.relative_humidity = Read<float>();
            }
        }

        header_data.image_origin = Read<uint32_t>();
        header_data.image_size = Read<uint32_t>();

        if (header_data.image_size > static_cast<uint64_t>(20e6))
        {
            // WARN << "ABIR Load: Image size exceeds max allowable. Check version
            // type.";
            Close();
            frames->x_pixels = frames->ir_data[0].image_x_size;
            frames->y_pixels = frames->ir_data[0].image_y_size;

            for (const auto& row : video_frames_16bit)
            {
                max_val = std::max(max_val, *std::max_element(row.begin(), row.end()));
            }

            frames->max_value = max_val;
            frames->video_frames_16bit = std::move(video_frames_16bit);

            return frames;
        }

        auto image_data = std::make_unique<uint16_t[]>(header_data.image_size);

        if (!header_only)
        {
            Read(image_data.get(), static_cast<uint32_t>(header_data.image_size));
        }

        video_frames_16bit.emplace_back(image_data.get(), image_data.get() + header_data.image_size);
        image_data.reset();

        frames->ir_data.emplace_back(header_data);

        last_valid_frame = static_cast<uint16_t>(frame_index);

        auto progress = frame_index / (frame_span * 0.01);
        QCoreApplication::processEvents();

        emit advanceFrame(static_cast<int>(std::round(progress)));
    }

    Close();

    frames->x_pixels = frames->ir_data[0].image_x_size;
    frames->y_pixels = frames->ir_data[0].image_y_size;

    for (const auto& row : video_frames_16bit)
    {
        max_val = std::max(max_val, *std::max_element(row.begin(), row.end()));
    }

    frames->max_value = max_val;
    frames->last_valid_frame = last_valid_frame;

    frames->video_frames_16bit = std::move(video_frames_16bit);

    return frames;
}
