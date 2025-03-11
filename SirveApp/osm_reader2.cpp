#include "osm_reader2.h"

static constexpr auto MAX_NUMBER_ITERATIONS = 100000;
static constexpr auto SMALL_NUMBER = 0.000001;

std::vector<Frame> OSMReader2::ReadFrames()
{
    if (!IsOpen()) {
        throw std::runtime_error("File not open");
    }

    frame_time_.clear();

    auto num_messages = FindMessageNumber();

    auto data = LoadFrames(num_messages);

    return {};
}

std::vector<Frame> OSMReader2::LoadFrames(uint32_t num_messages)
{
    std::vector<Frame> frames;

    // combine_tracks is ALWAYS true

    arma::vec frame_time_vec(frame_time_);
    arma::vec diff = arma::diff(frame_time_vec);
    arma::uvec index = arma::find(diff != 0);

    frames.reserve(index.n_elem);

    Seek(0, std::ios_base::beg);

    for (auto i = 0u; i < num_messages; ++i) {
        Frame current_frame;
        current_frame.msg_header = ReadMessageHeader();
        if (current_frame.msg_header.size < 0) {
            return {};
        }

        current_frame.frame_header = ReadFrameHeader();
    }

    return frames;
}

MessageHeader OSMReader2::ReadMessageHeader()
{
    MessageHeader header{};

    auto seconds = Read<uint64_t>();
    auto nano_seconds = Read<uint64_t>();
    auto tsize = Read<uint64_t>();

    if (tsize < SMALL_NUMBER) {
        header.size = -1;
        return header;
    }

    header.seconds = static_cast<double>(seconds + nano_seconds * 1e-9);
    header.size = tsize;

    return header;
}

FrameHeader OSMReader2::ReadFrameHeader()
{
    FrameHeader fh{};

    // FIXME: frame header requires reading big-endian integers

    fh.authorization = Read<uint64_t>(true);

    return fh;
}

FrameData OSMReader2::ReadFrameData()
{
    FrameData data{};

    // FIXME: frame data requires reading big-endian integers

    data.task_id = Read<uint32_t>();
    auto osm_seconds = Read<uint32_t>();


    return data;
}

uint32_t OSMReader2::FindMessageNumber()
{
    static constexpr auto NUM_HEADER_VALUES = 3;

    uint32_t num_messages = 0;
    uint32_t num_iterations = 0;

    while (num_iterations < MAX_NUMBER_ITERATIONS) {
        uint64_t header[NUM_HEADER_VALUES];

        size_t status_code = ReadArray(header);
        if (status_code == NUM_HEADER_VALUES && header[2]) {
            num_messages++;
            auto current_p = Tell();
            int64_t seek_position = 92 - 24; // TODO: MAGIC!
            Seek(seek_position, std::ios_base::cur);
            auto current_p1 = Tell();
            uint32_t data[2];
            status_code = ReadArray(data);
            double value = data[0] + data[1] * 1e-6;
            frame_time_.push_back(value);
            seek_position = header[2] - 76; // TODO: MAGIC!
            Seek(seek_position, std::ios_base::cur);
        } else {
            break;
        }

        num_iterations++;
    }

    return 0;
}
