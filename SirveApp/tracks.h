#ifndef TRACKS_H
#define TRACKS_H

#include <QString>

#include <map>
#include <set>
#include <vector>
#include <optional>

#include "data_structures.h"
#include "abir_reader.h"

struct TrackDetails {
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

struct PlottingTrackDetails {
    int track_id = -1;
    TrackDetails centroid;
    double sum_relative_counts = 0.0;
    double azimuth = 0.0;
    double elevation = 0.0;
    double irradiance = 0.0;
};

struct ManualPlottingTrackDetails {
    TrackDetails centroid;
    double azimuth = 0.0;
    double elevation = 0.0;
    double irradiance = 0.0;
    double sum_relative_counts = 0.0;
};

struct ManualPlottingTrackFrame {
    std::map<int, ManualPlottingTrackDetails> tracks;
};

struct PlottingTrackFrame {
    std::vector<PlottingTrackDetails> details;
};

struct TrackFrame {
    std::map<int, TrackDetails> tracks;
};

struct TrackFileReadResult {
    std::vector<TrackFrame> frames;
    std::set<int> track_ids;
    QString error_string;
};

struct TrackEngineeringData {
    double i_fov_x = 0.0, i_fov_y = 0.0;
    double boresight_lat = 0.0, boresight_long = 0.0;
    std::vector<double> dcm;
};

class TrackInformation {
    public:
        TrackInformation(unsigned int num_frames);
        TrackInformation(const std::vector<Frame> & osm_file_frames, ABPFileType file_type);

        TrackFileReadResult ReadTracksFromFile(QString file_name) const;
        void AddManualTracks(const std::vector<TrackFrame>& new_frames);
        void RemoveManualTrack(int track_id);
        void RemoveManualTrackPlotting(int track_id);
        void RemoveManualTrackImage(int track_id);
        void AddCreatedManualTrack(const std::vector<PlottingFrameData>& frame_data,int track_id, const std::vector<std::optional<TrackDetails>> & new_track_details, const QString& new_track_file_name);
        std::vector<std::optional<TrackDetails>> CopyManualTrack(int track_id);
        std::vector<std::optional<TrackDetails>> GetEmptyTrack() const;

        std::vector<TrackFrame> get_osm_frames(int start_index, int end_index);
        std::vector<TrackFrame> get_manual_frames(int start_index, int end_index);
        std::vector<PlottingTrackFrame> get_osm_plotting_track_frames();
        std::vector<ManualPlottingTrackFrame> get_manual_plotting_frames();
        std::vector<TrackFrame> get_manual_image_frames();

        int get_track_count() const;
        int get_frame_count() const;
        std::set<int> get_manual_track_ids();
        std::set<int> get_OSM_track_ids();
        int nRows = 480;
        int nCols = 640;

    private:
        TrackInformation();
        ManualPlottingTrackDetails GetManualPlottingTrackDetails(int frame_number, int centroid_x, int centroid_y, double sum_relative_counts) const;

        std::vector<PlottingTrackFrame> osm_plotting_track_frames;
        std::vector<TrackFrame> osm_frames;
        std::vector<TrackEngineeringData> track_engineering_data;
        std::set<int> osm_track_ids;

        std::vector<TrackFrame> manual_frames;
        std::set<int> manual_track_ids;
        std::vector<ManualPlottingTrackFrame> manual_plotting_frames;
        std::vector<TrackFrame> manual_image_frames;
};

#endif
