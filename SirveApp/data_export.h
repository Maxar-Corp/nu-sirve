#pragma once
#ifndef DATA_EXPORT_H
#define DATA_EXPORT_H

#include <vector>
#include <string>
#include <fstream>

#include "Data_Structures.h"
#include "tracks.h"

namespace DataExport
{
    void WriteTrackDataToCsv(std::string save_path, std::vector<PlottingFrameData> frame_data,
                                std::vector<PlottingTrackFrame> track_data, std::vector<ManualPlottingTrackFrame> manual_track_data,
                         int min_frame = 0, int max_frame = 0, const std::vector<TrackFrame> osm_track_frame_data = std::vector<TrackFrame>());
};

#endif
