#include "calibrate_OSM_tracks.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

void CalibrateExistingTracks::CalibrateOSMTracks(   
                                                    CalibrationData& calibration_model,
                                                    std::vector<Frame> frame_data,
                                                    std::vector<PlottingTrackFrame> & track_data,
                                                    const VideoDetails& base_processing_state_details,
                                                    const std::vector<ABIRFrameHeader>& input_frame_header,
                                                    int min_frame, int max_frame, ABPFileType file_type)
{
    if (max_frame == 0)
	{
		max_frame = static_cast<int>(frame_data.size());
	}

    int nRows = 480;
    int nCols = 640;

    if (file_type == ABPFileType::ABP_D){
        nRows = 720;
        nCols = 1280;
    }
    int nRows2 = nRows/2;
    int nCols2 = nCols/2;

    uint relative_indx;
    
    if (calibration_model.calibration_available){
   
        for (unsigned int absolute_indx = min_frame; absolute_indx < max_frame; absolute_indx++)
        {
            relative_indx = absolute_indx - min_frame;
            double frame_integration_time = input_frame_header[relative_indx].int_time;
            for (size_t j = 0; j < track_data[absolute_indx].details.size(); j++)
            {
                cv::Rect bbox;
                bbox.x = frame_data[absolute_indx].data.track_data[j].roiBLX;
                bbox.y = frame_data[absolute_indx].data.track_data[j].roiBLY;
                bbox.height = frame_data[absolute_indx].data.track_data[j].roiBLY - frame_data[absolute_indx].data.track_data[j].roiURY;
                bbox.width = frame_data[absolute_indx].data.track_data[j].roiURX - frame_data[absolute_indx].data.track_data[j].roiBLX;
                std::array<double, 3> measurements = {0,0,0};

                measurements = SharedTrackingFunctions::CalculateSumCounts(relative_indx, bbox, base_processing_state_details, frame_integration_time, calibration_model);
                track_data[absolute_indx].details[j].centroid.peak_irradiance = measurements[0];
                track_data[absolute_indx].details[j].centroid.mean_irradiance = measurements[1];
                track_data[absolute_indx].details[j].centroid.sum_irradiance = measurements[2];
    
            }

        }
    }

}
