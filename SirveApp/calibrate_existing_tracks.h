#pragma once
#ifndef CALIBRATE_EXISTING_TRACKS_H
#define DCALIBRATE_EXISTING_TRACKS_H

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
                                std::vector<Frame> & frame_data,
                                std::vector<PlottingTrackFrame> & track_data, 
                                const VideoDetails& base_processing_state_details,
                                const std::vector<ABIRFrameHeader>& input_frame_header,                              
                                int absolute_indx_start_0 = 1,
                                int absolute_indx_stop_0 = 0,
                                ABPFileType file_type = ABPFileType::ABP_B);
                                
    void CalibrateManualTracks(    
                                CalibrationData& calibration_model,
                                std::vector<TrackFrame>  & frame_data,
                                std::vector<ManualPlottingTrackFrame> & track_data, 
                                const VideoDetails& base_processing_state_details,
                                const std::vector<ABIRFrameHeader>& input_frame_header,                              
                                int absolute_indx_start_0 = 1,
                                int absolute_indx_stop_0 = 0,
                                ABPFileType file_type = ABPFileType::ABP_B,
                                bool recalibrateTF = true);

};

#endif
