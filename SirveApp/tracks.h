#ifndef TRACKS_H
#define TRACKS_H

#include <map>
#include <set>
#include <vector>
#include <optional>

#include "data_structures.h"
#include "abir_reader.h"

struct TrackDetails
{
    TrackDetails() = default;
    TrackDetails(const TrackDetails&) = default;

    TrackDetails(double frameTime, double julianDate, double secondPastMidnight, double timingOffset, int centroidXBoresight,
                 int centroidYBoresight, int centroidX, int centroidY, double az, double el, int peakCounts, int sumCounts, int meanCounts,
                 int numberPixels, double sumRelativeCounts, double peakIrradiance, double meanIrradiance, double sumIrradiance,
                 double sumRelativeIrradiance, int bboxX, int bboxY, int bboxWidth, int bboxHeight);

    TrackDetails(double centroid_x, double centroid_y);

    TrackDetails(const TrackData& track_data);
    TrackDetails& operator=(const TrackData& track_data);
    TrackDetails(const FrameData& frame_data, const TrackData& track_data);

    double frame_time = 0.0;
    double julian_date = 0.0;
    double second_past_midnight = 0.0;
    double timing_offset = 0.0;
    int centroid_x_boresight = 0;
    int centroid_y_boresight = 0;
    int centroid_x = 0;
    int centroid_y = 0;
    double az = 0.0;
    double el = 0.0;
    int peak_counts = 0;
    int sum_counts = 0;
    int mean_counts = 0;
    int number_pixels = 0;
    double sum_relative_counts = 0.0;
    double peak_irradiance = 0.0;
    double mean_irradiance = 0.0;
    double sum_irradiance = 0.0;
    double sum_relative_irradiance = 0.0;
    int bbox_x = 0;
    int bbox_y = 0;
    int bbox_width = 0;
    int bbox_height = 0;
};

struct PlottingTrackDetails
{
    PlottingTrackDetails() = default;
    PlottingTrackDetails(const PlottingTrackDetails&) = default;

    PlottingTrackDetails(int trackId, TrackDetails centroid, double sumRelativeCounts, double az, double el,
                         double irradiance);

    PlottingTrackDetails(const TrackData& track_data);
    PlottingTrackDetails& operator=(const TrackData& track_data);

    int track_id = -1;
    TrackDetails centroid;
    double sum_relative_counts = 0.0;
    double azimuth = 0.0;
    double elevation = 0.0;
    double irradiance = 0.0;
};

struct ManualPlottingTrackDetails
{
    TrackDetails centroid;
    double azimuth = 0.0;
    double elevation = 0.0;
    double irradiance = 0.0;
    double peak_irradiance = 0.0;
    double mean_irradiance = 0.0;
    double sum_irradiance = 0.0;
    double sum_relative_counts = 0.0;
};

struct ManualPlottingTrackFrame
{
    std::map<int, ManualPlottingTrackDetails> tracks;
};

struct PlottingTrackFrame
{
    std::vector<PlottingTrackDetails> details;
};

struct TrackFrame
{
    std::map<int, TrackDetails> tracks;
};

struct TrackFileReadResult
{
    std::vector<TrackFrame> frames;
    std::set<int> track_ids;
    QString error_string;
};

struct TrackEngineeringData
{
    TrackEngineeringData() = default;
    TrackEngineeringData(const TrackEngineeringData&) = default;
    TrackEngineeringData& operator=(const TrackEngineeringData&) = default;

    TrackEngineeringData(double iFovX, double iFovY, double boresightLat, double boresightLong, std::vector<double> dcm);

    TrackEngineeringData(const FrameData& frame_data);
    TrackEngineeringData& operator=(const FrameData& frame_data);

    double i_fov_x = 0.0, i_fov_y = 0.0;
    double boresight_lat = 0.0, boresight_long = 0.0;
    std::vector<double> dcm;
};

class TrackInformation
{
public:
    TrackInformation(size_t num_frames);
    TrackInformation(const std::vector<Frame>& osm_file_frames, ABPFileType file_type);

    TrackFileReadResult ReadTracksFromFile(QString file_name) const;
    void AddManualTracks(const std::vector<TrackFrame>& new_frames);
    void RemoveManualTrack(int track_id);
    void RemoveManualTrackPlotting(int track_id);
    void RemoveManualTrackImage(int track_id);
    void AddCreatedManualTrack(const std::vector<PlottingFrameData>& frame_data, int track_id,
                               const std::vector<std::optional<TrackDetails>>& new_track_details, const QString& new_track_file_name);
    std::vector<std::optional<TrackDetails>> CopyManualTrack(int track_id);
    std::vector<std::optional<TrackDetails>> GetEmptyTrack() const;
    void WriteManualTrackToFile(const std::vector<PlottingFrameData>& frame_data, int track_id, const std::vector<std::optional<TrackDetails>> & new_track_details, const QString& new_track_file_name);

    std::vector<TrackFrame> GetOsmFrames(size_t start_index, size_t end_index) const;
    std::vector<TrackFrame> GetManualFrames(int start_index, int end_index) const;
    std::vector<PlottingTrackFrame>& GetOsmPlottingTrackFrames();
    const std::vector<ManualPlottingTrackFrame>& GetManualPlottingFrames();
    const std::vector<TrackFrame>& GetManualImageFrames();

    size_t GetTrackCount() const;
    size_t GetFrameCount() const;
    const std::set<int>& GetManualTrackIds();
    const std::set<int>& GetOsmTrackIds();

private:
    TrackInformation() = default;
    ManualPlottingTrackDetails GetManualPlottingTrackDetails(int frame_number, int centroid_x, int centroid_y,
                                                             double sum_relative_counts) const;

    std::vector<PlottingTrackFrame> osm_plotting_track_frames;
    std::vector<TrackFrame> osm_frames;
    std::vector<TrackEngineeringData> track_engineering_data;
    std::set<int> osm_track_ids;

    std::vector<TrackFrame> manual_frames;
    std::set<int> manual_track_ids;
    std::vector<ManualPlottingTrackFrame> manual_plotting_frames;
    std::vector<TrackFrame> manual_image_frames;
    int nRows = 480;
    int nCols = 640;
};

#endif
