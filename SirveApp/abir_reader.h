#ifndef ABIR_READER_H
#define ABIR_READER_H

#include "binary_reader.h"

enum class ABPFileType
{
    ABP_B,
    ABP_D,
};

#pragma pack(push, 1)
///////////////////////////////////////////////////////////////////////////////
struct IMUData
{
    float imu_angle[3];
    float imu_vel[3];
};
#pragma pack(pop)

///////////////////////////////////////////////////////////////////////////////
struct ABIRFrameObject
{
    double frame_time;
    double alpha;
    double beta;
    double alpha_dot;
    double beta_dot;
    double imc_az;
    double imc_el;
    double ars_ypr[3];
    double p_lla[3];
    double p_ypr[3];
    double p_vel[3];
    double fpa_ypr[3];

    uint32_t imu_count;
    IMUData imu_data[8];

    // VERSION >= 4.1
    double p_ypr_dot[3];
    // END VERSION >= 4.1

    // VERSION >= 4.2
    double imu_sum[3];
    // END VERSION >= 4.2
};

///////////////////////////////////////////////////////////////////////////////
struct ABIRFrameHeader
{
    uint64_t seconds{};
    uint64_t nano_seconds{};
    uint64_t size{};
    uint64_t image_size{};
    uint64_t image_size_double{};

    uint16_t sensor_id{};
    uint32_t frame_number{};
    double frame_time{};
    uint32_t image_origin{};
    uint16_t image_x_size{};
    uint16_t image_y_size{};
    uint16_t pixel_depth{};
    uint16_t bits_per_pixel{};

    uint32_t sensor_fov{};
    double alpha{};
    double beta{};
    double alpha_dot{};
    double beta_dot{};

    // VERSION >=2
    uint16_t focus{};
    // END VERSION >=2

    // VERSION >= 2.1
    uint32_t msg_type{};
    float msg_version{};
    uint32_t guid[5]{};
    uint32_t guid_source[5]{};
    uint32_t intended_fov{};
    // END VERSION >=2.1

    // VERSION == 3.0
    double p_heading{};
    double p_pitch{};
    double p_roll{};
    double p_alt_gps{};
    double p_heading_mag{};
    uint32_t p_heading_ref{};
    // END VERSION == 3

    // VERSION >= 3.0
    double p_lla[3]{};
    double p_vel_xyz[3]{};
    // END VERSION >= 3.0

    // VERSION >= 3.1
    float int_time{};
    double p_ypr[3]{};
    // END VERSION >= 3.1

    // VERSION >= 4.0
    float fpa_gain{};
    ABIRFrameObject one{};
    ABIRFrameObject two{};
    // END VERSION >= 4.0

    // VERSIONS >= 4.1
    float temp_k{};
    float pressure{};
    float relative_humidity{};
    float cooling{};    // for MTS-D Data
    double p_ypr_dot[3]{};
    // END VERSIONS >= 4.1

    // VERSION >= 5
    double g_roll{};
    double g_roll_rate{};
    // END VERSION >= 5
};

///////////////////////////////////////////////////////////////////////////////
struct ABIRFrames
{
    ABIRFrames();
    ABIRFrames(const ABIRFrames& other) = default;
    ABIRFrames(ABIRFrames&& other) = default;
    ~ABIRFrames() = default;

    using Ptr = std::unique_ptr<ABIRFrames>;

    ABIRFrames& operator=(const ABIRFrames& other) = default;
    ABIRFrames& operator=(ABIRFrames&& other) = default;

    std::vector<std::vector<uint16_t>> video_frames_16bit;
    std::vector<ABIRFrameHeader> ir_data;
    int x_pixels;
    int y_pixels;
    int max_value;
    int last_valid_frame;
};

///////////////////////////////////////////////////////////////////////////////
class ABIRReader : public BinaryReader
{
    Q_OBJECT

public:
    ABIRReader() = default;
    ~ABIRReader() override = default;

    bool Open(const char* filename, ABPFileType file_type);
    ABIRFrames::Ptr ReadFrames(uint32_t min_frame, uint32_t max_frame, bool header_only = false);

private:
signals:
    void AdvanceFrame(int);

private:
    double file_version_{};
    ABPFileType file_type_{ABPFileType::ABP_B};
};

#endif // ABIR_READER_H
