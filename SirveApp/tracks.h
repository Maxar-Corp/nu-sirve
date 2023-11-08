#pragma once

#ifndef TRACKS_H
#define TRACKS_H

#include <map>
#include <set>
#include <vector>
#include "Data_Structures.h"

struct TrackDetails {
    int centroid_x;
    int centroid_y;
};

struct TrackFrame {
    std::map<int, TrackDetails> tracks;
};

class TrackInformation {
    public:
        TrackInformation(const std::vector<Frame> & osm_frames);
        std::vector<TrackFrame> get_frames(int start_index, int end_index);
        int get_count_of_tracks();
    private:
        std::vector<TrackFrame> frames;
        std::set<int> track_ids;
};

#endif