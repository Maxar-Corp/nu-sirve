#pragma once

#ifndef TRACKS_H
#define TRACKS_H

#include <QFile>
#include <QString>

#include <map>
#include <set>
#include <vector>
#include <optional>

#include "Data_Structures.h"
#include "support/az_el_calculation.h"

struct PlottingTrackDetails {
    int track_id;
    double irradiance;
    double azimuth;
    double elevation;
};

struct ManualPlottingTrackDetails {
    double azimuth;
    double elevation;
};

struct ManualPlottingTrackFrame {
    std::map<int, ManualPlottingTrackDetails> tracks;
};

struct TrackDetails {
    int centroid_x;
    int centroid_y;
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
    double i_fov_x, i_fov_y;
    double boresight_lat, boresight_long;
    std::vector<double> dcm;
};

class TrackInformation {
    public:
        TrackInformation(unsigned int num_frames);
        TrackInformation(const std::vector<Frame> & osm_file_frames);

        TrackFileReadResult ReadTracksFromFile(QString file_name) const;
        void AddManualTracks(std::vector<TrackFrame> new_frames);
        void RemoveManualTrack(int track_id);
        void AddCreatedManualTrack(int track_id, const std::vector<std::optional<TrackDetails>> & new_track_details, QString new_track_file_name);
        std::vector<std::optional<TrackDetails>> CopyManualTrack(int track_id);
        std::vector<std::optional<TrackDetails>> GetEmptyTrack();

        std::vector<TrackFrame> get_osm_frames(int start_index, int end_index);
        std::vector<TrackFrame> get_manual_frames(int start_index, int end_index);
        std::vector<PlottingTrackFrame> get_plotting_tracks();
        std::vector<ManualPlottingTrackFrame> get_manual_plotting_tracks();

        int get_count_of_tracks();
        std::set<int> get_manual_track_ids();

    private:
        TrackInformation();
        ManualPlottingTrackDetails CalculateAzimuthElevation(int frame_number, int centroid_x, int centroid_y);

        std::vector<PlottingTrackFrame> osm_plotting_track_frames;
        std::vector<TrackFrame> osm_frames;
        std::vector<TrackEngineeringData> track_engineering_data;
        std::set<int> osm_track_ids;

        std::vector<TrackFrame> manual_frames;
        std::set<int> manual_track_ids;
        std::vector<ManualPlottingTrackFrame> manual_plotting_frames;
};

#endif
