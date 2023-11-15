#pragma once

#ifndef TRACKS_H
#define TRACKS_H

#include <QFile>
#include <QString>

#include <map>
#include <set>
#include <vector>

#include "Data_Structures.h"

struct PlottingTrackDetails {
    int track_id;
    double irradiance;
    double azimuth;
    double elevation;
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

class TrackInformation {
    public:
        TrackInformation(unsigned int num_frames);
        TrackInformation(const std::vector<Frame> & osm_file_frames);
        std::vector<TrackFrame> get_frames(int start_index, int end_index);
        std::vector<PlottingTrackFrame> get_plotting_tracks();
        int get_count_of_tracks();
        std::set<int> get_manual_track_ids();
        void add_manual_tracks(std::vector<TrackFrame> new_frames);

        TrackFileReadResult read_tracks_from_file(QString file_name);
    private:
        TrackInformation();
        std::vector<PlottingTrackFrame> osm_plotting_track_frames;
        std::vector<TrackFrame> osm_frames;
        std::set<int> osm_track_ids;

        std::vector<TrackFrame> manual_frames;
        std::set<int> manual_track_ids;
};

#endif