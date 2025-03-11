#pragma once

#include "binary_reader.h"

class OSMReader2 : public BinaryReader
{
public:
    OSMReader2() = default;
    ~OSMReader2() override = default;

    std::vector<Frame> ReadFrames();

private:
    std::vector<Frame> LoadFrames(uint32_t num_messages);
    MessageHeader ReadMessageHeader();
    FrameHeader ReadFrameHeader();
    FrameData ReadFrameData();

    uint32_t FindMessageNumber();

    std::vector<double> frame_time_;
};
