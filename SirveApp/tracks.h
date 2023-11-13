#pragma once

#ifndef TRACKS_H
#define TRACKS_H

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

class TrackInformation {
    public:
        TrackInformation(const std::vector<Frame> & osm_frames);
        std::vector<TrackFrame> get_frames(int start_index, int end_index);
        std::vector<PlottingTrackFrame> get_plotting_tracks();
        int get_count_of_tracks();
    private:
        std::vector<PlottingTrackFrame> plotting_track_frames;
        std::vector<TrackFrame> frames;
        std::set<int> track_ids;
};

#endif