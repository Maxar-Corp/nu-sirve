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
    void write_track_data_to_csv(std::string save_path, std::vector<Plotting_Frame_Data> frame_data,
                                std::vector<PlottingTrackFrame> track_data, std::vector<ManualPlottingTrackFrame> manual_track_data,
                                int min_frame = 0, int max_frame = 0);
};

#endif