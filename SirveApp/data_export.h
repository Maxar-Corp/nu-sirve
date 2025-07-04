#pragma once
#ifndef DATA_EXPORT_H
#define DATA_EXPORT_H

#include <vector>
#include <string>
#include <fstream>

#include "data_structures.h"
#include "tracks.h"
#include "calibration_data.h"

namespace DataExport
{
    void WriteTrackDataToCsv(   const CalibrationData model,
                                std::string save_path,
                                std::vector<PlottingFrameData> frame_data,
                                std::vector<PlottingTrackFrame> track_data,
                                std::vector<ManualPlottingTrackFrame> manual_track_data,
                                int min_frame = 1,
                                int max_frame = 0,
                                ABPFileType file_type = ABPFileType::ABP_B
                               );
};

#endif
