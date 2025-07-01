
#include <armadillo>
#include <numeric>

#include "location_input.h"
#include "osm_reader.h"
#include "support/az_el_calculation.h"
#include "support/jtime.h"

static constexpr auto MAX_NUMBER_ITERATIONS = 100000;
static constexpr auto SMALL_NUMBER = 0.000001;

std::vector<Frame> OSMReader::ReadFrames(ABPFileType file_type)
{
    file_type_ = file_type;
    if (!IsOpen())
    {
        throw std::runtime_error("File not open");
    }

    frame_time_.clear();

    auto num_messages = FindMessageNumber();

    auto data = LoadFrames(num_messages);

    if (file_type == ABPFileType::ABP_D)
    {
        nRows = 720;
        nCols = 1280;
    }

    return data;
}

std::vector<Frame> OSMReader::LoadFrames(uint32_t num_messages)
{
    std::vector<Frame> frames;

    // combine_tracks is ALWAYS true

    arma::vec frame_time_vec(frame_time_);
    arma::vec diff = arma::diff(frame_time_vec);
    arma::uvec index = arma::find(diff != 0);

    frames.reserve(index.n_elem);

    Seek(0, SEEK_SET);

    for (auto i = 0u; i < num_messages; ++i)
    {
        Frame current_frame;
        current_frame.msg_header = ReadMessageHeader();
        if (static_cast<int64_t>(current_frame.msg_header.size) < 0)
        {
            return {};
        }

        current_frame.frame_header = ReadFrameHeader();
        current_frame.data = ReadFrameData();

        if (current_frame.data.ecf.empty())
        {
            return {};
        }

        if (i == 0 || frame_time_[i] - frame_time_[i - 1] != 0.00)
        {
            frames.emplace_back(std::move(current_frame));
        }
        else if (!frames.empty())
        {
            frames[frames.size() - 1].data.num_tracks += 1;
            frames[frames.size() - 1].data.track_data.push_back(current_frame.data.track_data[0]);
        }
    }

    return frames;
}

MessageHeader OSMReader::ReadMessageHeader()
{
    MessageHeader header{};

    auto seconds = Read<uint64_t>();
    auto nano_seconds = Read<uint64_t>();
    auto tsize = Read<uint64_t>();

    if (static_cast<double>(tsize) < SMALL_NUMBER)
    {
        header.size = -1;
        return header;
    }

    header.seconds = static_cast<double>(seconds) + static_cast<double>(nano_seconds) * 1e-9;
    header.size = tsize;

    return header;
}

FrameHeader OSMReader::ReadFrameHeader()
{
    FrameHeader fh{};

    // Frame header requires reading big-endian integers

    fh.authorization = Read<uint64_t>(true);
    fh.classification = Read<uint32_t>(true);
    fh.type = Read<uint32_t>(true);
    fh.priority = Read<uint32_t>(true);
    fh.oper_indicator = Read<uint32_t>(true);
    fh.info_source = Read<uint32_t>(true);
    fh.info_destination = Read<uint32_t>(true);

    // TODO: Matlab has commented out input for sensor ID. Check that this is correct.

    auto frame_seconds = Read<uint32_t>(true);
    auto frame_micro_seconds = Read<uint32_t>(true);
    fh.time_generated_seconds = frame_seconds + frame_micro_seconds * 1e-6;
    fh.transaction_id = Read<uint32_t>(true);
    fh.ack_req_indicator = Read<uint32_t>(true);
    fh.ack_response = Read<uint32_t>(true);
    fh.cant_pro_reason = Read<uint32_t>(true);
    fh.message_length = Read<uint32_t>(true);
    fh.software_version = Read<uint32_t>(true);

    return fh;
}

FrameData OSMReader::ReadFrameData()
{
    FrameData data{};

    // Frame data requires reading big-endian integers

    data.task_id = Read<uint32_t>(true);
    auto osm_seconds = Read<uint32_t>(true);
    auto osm_micro_seconds = Read<uint32_t>(true);

    data.frame_time = osm_seconds + osm_micro_seconds * 1e-6; // GPS Time since Jan 6, 1990
    data.julian_date = CalculateGpsUtcJulianDate(data.frame_time);

    auto modified_julian_date = data.julian_date + 0.5;
    int midnight_julian = static_cast<int>(std::floor(modified_julian_date));

    data.seconds_past_midnight = (modified_julian_date - midnight_julian) * 86400.;

    data.mrp = ReadVector<double>(3, true);
    data.mrp_cov_rand = ReadVector<double>(6, true);
    data.mrp_cov_bias = ReadVector<double>(6, true);

    data.ecf = ReadVector<double>(6, true);

    auto sum = std::accumulate(data.ecf.begin(), data.ecf.end(), 0.0);
    if (sum < SMALL_NUMBER)
    {
        if (location_from_file_)
        {
            data.ecf = file_ecef_vector_;
        }
        else
        {
            LocationInput location_input;
            auto response = location_input.exec();
            location_from_file_ = true;

            if (response && location_input.path_set)
            {
                file_ecef_vector_ = location_input.GetECEFVector();
            }
            else
            {
                FrameData bad_input{};
                location_from_file_ = false;
                return bad_input;
            }
        }
    }

    data.lla = CalculateLatLonAltVector(data.ecf);
    data.dcm = CalculateDirectionCosineMatrix(data.mrp);
    data.i_fov_x = Read<double>(true);
    data.i_fov_y = Read<double>(true);
    data.num_tracks = Read<uint32_t>(true);

    data.az_el_boresight = CalculateAzimuthElevation(nRows, nCols, 0, 0, data);

    for (auto j = 0u; j < data.num_tracks; ++j)
    {
        auto current_Track = GetTrackData(data);
        data.track_data.emplace_back(std::move(current_Track));
    }

    return data;
}

TrackData OSMReader::GetTrackData(const FrameData& input)
{
    TrackData current_track{};

    current_track.track_id = Read<uint32_t>(true);
    current_track.sensor_type = Read<uint32_t>(true);

    auto num_bands = Read<uint32_t>(true);
    current_track.num_bands = num_bands;

    for (auto k = 0u; k < num_bands; ++k)
    {
        SumCountsData ir_data{};
        ir_data.band_id = Read<uint32_t>(true);
        ir_data.num_measurements = Read<uint32_t>(true);

        std::vector<double> irrad, irr_sigma, irr_time;
        for (auto m = 0u; m < ir_data.num_measurements; ++m)
        {
            ir_data.ir_radiance.emplace_back(Read<double>(true));
            ir_data.ir_sigma.emplace_back(Read<double>(true));

            auto osm_seconds = Read<uint32_t>(true);
            auto osm_micro_seconds = Read<uint32_t>(true);
            auto measurement_time = osm_seconds + osm_micro_seconds * 1e-6; // GPS Time since Jan 6, 1990
            ir_data.time.emplace_back(measurement_time);
        }

        current_track.ir_measurements.emplace_back(std::move(ir_data));
    }

    current_track.roiBLX = Read<uint32_t>(true);
    current_track.roiBLY = Read<uint32_t>(true);
    current_track.roiURX = Read<uint32_t>(true);
    current_track.roiURY = Read<uint32_t>(true);

    current_track.semi_major_axis = Read<double>(true);
    current_track.semi_minor_axis = Read<double>(true);

    current_track.orientation_angle = Read<double>(true);

    current_track.maxX = Read<double>(true);
    current_track.maxY = Read<double>(true);

    current_track.frame_stabilize_x = Read<double>(true);
    current_track.frame_stabilize_y = Read<double>(true);

    current_track.stable_frame_shift_x = Read<double>(true);
    current_track.stable_frame_shift_y = Read<double>(true);

    current_track.centroid_x = Read<double>(true);
    current_track.centroid_y = Read<double>(true);
    current_track.centroid_variance_x = Read<double>(true);
    current_track.centroid_variance_y = Read<double>(true);

    current_track.az_el_track = CalculateAzimuthElevation(nRows, nCols, static_cast<int>(current_track.centroid_x),
                                                          static_cast<int>(current_track.centroid_y), input);

    current_track.covariance = Read<double>(true);
    current_track.num_pixels = Read<uint32_t>(true);
    current_track.object_type = Read<uint32_t>(true);

    return current_track;
}

std::vector<double> OSMReader::CalculateAzimuthElevation(const int nRows, const int nCols, const int x_pixel, const int y_pixel, const FrameData& input)
{
    std::vector<double> results = AzElCalculation::calculateAzEl(nRows, nCols, x_pixel, y_pixel, input.lla[0], input.lla[1], input.dcm,
                                                             input.i_fov_x, input.i_fov_y, false);
    return results;
}

std::vector<double> OSMReader::CalculateDirectionCosineMatrix(const std::vector<double>& input)
{
    arma::vec mr(input);

    double sig = 1. - arma::accu(arma::square(mr));
    double norm = 1. / std::pow(1 + arma::accu(arma::square(mr)), 2);
    double sig2 = std::pow(sig, 2);

    double mr02 = mr(0) * mr(0);
    double mr12 = mr(1) * mr(1);
    double mr22 = mr(2) * mr(2);

    arma::mat dcos(3, 3);
    dcos(0, 0) = 4 * (mr02 - mr12 - mr22) + sig2;
    dcos(0, 1) = 8 * mr(0) * mr(1) + 4 * mr(2) * sig;
    dcos(0, 2) = 8 * mr(0) * mr(2) - 4 * mr(1) * sig;

    dcos(1, 0) = 8 * mr(0) * mr(1) - 4 * mr(2) * sig;
    dcos(1, 1) = 4 * (-mr02 + mr12 - mr22) + sig2;
    dcos(1, 2) = 8 * mr(1) * mr(2) + 4 * mr(0) * sig;

    dcos(2, 0) = 8 * mr(0) * mr(2) + 4 * mr(1) * sig;
    dcos(2, 1) = 8 * mr(1) * mr(2) - 4 * mr(0) * sig;
    dcos(2, 2) = 4 * (-mr02 - mr12 + mr22) + sig2;

    dcos = norm * dcos;

    arma::vec temp = arma::conv_to<arma::vec>::from(arma::vectorise(dcos));
    std::vector<double> out = arma::conv_to<std::vector<double>>::from(temp);

    return out;
}

std::vector<double> OSMReader::CalculateLatLonAltVector(const std::vector<double>& ecf)
{
    arma::vec ecf_vector(ecf);
    ecf_vector = ecf_vector / 1000;

    arma::vec lla = earth::ECFtoLLA(ecf_vector.subvec(0, 2));

    std::vector<double> out = arma::conv_to<std::vector<double>>::from(lla);

    out[2] *= 1000;

    return out;
}

uint32_t OSMReader::FindMessageNumber()
{
    static constexpr auto NUM_HEADER_VALUES = 3;

    uint32_t num_messages = 0;
    uint32_t num_iterations = 0;

    while (num_iterations < MAX_NUMBER_ITERATIONS)
    {
        uint64_t header[NUM_HEADER_VALUES]{};

        auto status_code = ReadArray(header);
        if (status_code == NUM_HEADER_VALUES && header[2])
        {
            num_messages++;
            int64_t seek_position = 92 - 24; // TODO: Don't use magic numbers

            Seek(seek_position, SEEK_CUR);

            uint32_t data[2];
            ReadArray(data, true); 

            double value = data[0] + data[1] * 1e-6;
            frame_time_.emplace_back(value);

            seek_position = static_cast<int64_t>(header[2]) - 76; // TODO: Don't use magic numbers
            Seek(seek_position, SEEK_CUR);
        }
        else
        {
            break;
        }

        num_iterations++;
    }

    return num_messages;
}

double OSMReader::CalculateGpsUtcJulianDate(double offset_gps_seconds)
{
    // Load leap seconds file organized as year, month, day
    arma::mat leap_seconds_matrix;
    leap_seconds_matrix.load("config/year_month_leap_seconds.txt", arma::raw_ascii);

    // Store leap seconds as julian date
    auto num_entries = leap_seconds_matrix.n_rows;
    arma::vec julian_date_leap_seconds(num_entries);

    // Convert dates that leap seconds occurred to julian dates
    for (auto i = 0u; i < num_entries; i++)
    {
        auto julian_date =
            jtime::JulianDate(static_cast<int>(leap_seconds_matrix(i, 0)), static_cast<int>(leap_seconds_matrix(i, 1)),
                              static_cast<int>(leap_seconds_matrix(i, 2)), 0, 0, 0);
        julian_date_leap_seconds(i) = julian_date;
    }

    // The last date that UTC and GPS time were aligned
    double base = jtime::JulianDate(1980, 1, 6, 0, 0, 0);
    arma::vec modified_julian_date_leap_seconds = julian_date_leap_seconds - base;

    // Find the number of leap seconds for a given date
    arma::uvec index_number_leap_seconds = arma::find(offset_gps_seconds / 86400. >= modified_julian_date_leap_seconds);
    unsigned int number_leap_seconds = index_number_leap_seconds.n_rows;

    // Adjust gps time
    double day_offset = (offset_gps_seconds - number_leap_seconds) / 86400.;

    // Establish utc julian date
    double utc_julian_date = base + day_offset;

    return utc_julian_date;
}
