#ifndef OSM_READER2_H
#define OSM_READER2_H

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
    TrackData GetTrackData(const FrameData& input);

    uint32_t FindMessageNumber();
    static double CalculateGpsUtcJulianDate(double offset_gps_seconds);
    static std::vector<double> CalculateLatLonAltVector(const std::vector<double>& ecf);
    static std::vector<double> CalculateDirectionCosineMatrix(const std::vector<double>& input);
    static std::vector<double> CalculateAzimuthElevation(int x_pixel, int y_pixel, const FrameData& input);

    std::vector<double> frame_time_;
    bool location_from_file_ = false;
    std::vector<double> file_ecef_vector_;
};

#endif // OSM_READER2_H
