#pragma once

#ifndef TRACKS_H
#define TRACKS_H

#include <map>
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
    private:
        std::vector<TrackFrame> frames;
};

#endif