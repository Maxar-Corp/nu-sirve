#pragma once
#ifndef CALIBRATE_OSM_TRACKS_H
#define DCALIBRATE_OSM_TRACKS_H

#include <vector>
#include <string>
#include <fstream>

#include "data_structures.h"
#include "tracks.h"
#include "calibration_data.h"
#include "shared_tracking_functions.h"

namespace CalibrateExistingTracks
{
    void CalibrateOSMTracks(    
                                CalibrationData& calibration_model,
                                std::vector<Frame> frame_data,
                                std::vector<PlottingTrackFrame> & track_data, 
                                const VideoDetails& base_processing_state_details,
                                const std::vector<ABIRFrameHeader>& input_frame_header,                              
                                int min_frame = 1,
                                int max_frame = 0,
                                ABPFileType file_type = ABPFileType::ABP_B);
};

#endif
