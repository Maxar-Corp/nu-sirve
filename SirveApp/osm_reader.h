#ifndef OSM_READER2_H
#define OSM_READER2_H

#include "binary_reader.h"
#include "data_structures.h"
#include "abir_reader.h"

class OSMReader : public BinaryReader
{
public:
    OSMReader() = default;
    ~OSMReader() override = default;

    std::vector<Frame> ReadFrames(ABPFileType file_type);

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
    static std::vector<double> CalculateAzimuthElevation(int nRows, int nCols, int x_pixel, int y_pixel, const FrameData& input);

    std::vector<double> frame_time_;
    bool location_from_file_ = false;
    std::vector<double> file_ecef_vector_;
    int nRows = 480;
    int nCols = 640;
    ABPFileType file_type_{ABPFileType::ABP_B};
};

#endif // OSM_READER2_H
