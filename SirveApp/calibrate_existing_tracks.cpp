#include "calibrate_Existing_tracks.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

void CalibrateExistingTracks::CalibrateOSMTracks(   
                                                    CalibrationData& calibration_model,
                                                    std::vector<Frame> & frame_data,
                                                    std::vector<PlottingTrackFrame> & track_data,
                                                    const VideoDetails& base_processing_state_details,
                                                    const std::vector<ABIRFrameHeader>& input_frame_header,
                                                    int absolute_indx_start_0, int absolute_indx_stop_0, ABPFileType file_type)
{
    if (absolute_indx_stop_0 == 0)
	{
		absolute_indx_stop_0 = static_cast<int>(frame_data.size());
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
   
        for (unsigned int absolute_indx_0 = absolute_indx_start_0; absolute_indx_0 <= absolute_indx_stop_0; absolute_indx_0++)
        {
            relative_indx = absolute_indx_0 - absolute_indx_start_0;
            double frame_integration_time = input_frame_header[relative_indx].int_time;
            for (size_t j = 0; j < track_data[absolute_indx_0].details.size(); j++)
            {
                cv::Rect bbox;
                bbox.x = frame_data[absolute_indx_0].data.track_data[j].roiBLX;
                bbox.y = frame_data[absolute_indx_0].data.track_data[j].roiBLY;
                bbox.height = frame_data[absolute_indx_0].data.track_data[j].roiBLY - frame_data[absolute_indx_0].data.track_data[j].roiURY;
                bbox.width = frame_data[absolute_indx_0].data.track_data[j].roiURX - frame_data[absolute_indx_0].data.track_data[j].roiBLX;

                std::array<double, 3> measurements = {0,0,0};

                measurements = SharedTrackingFunctions::CalculateSumCounts(relative_indx, bbox, base_processing_state_details, frame_integration_time, calibration_model);
                track_data[absolute_indx_0].details[j].centroid.peak_irradiance = measurements[0];
                track_data[absolute_indx_0].details[j].centroid.mean_irradiance = measurements[1];
                track_data[absolute_indx_0].details[j].centroid.sum_irradiance = measurements[2];
    
            }

        }
    }

}

void CalibrateExistingTracks::CalibrateManualTracks(   
                                                    CalibrationData& calibration_model,
                                                    std::vector<TrackFrame> & frame_data,
                                                    std::vector<ManualPlottingTrackFrame> & track_data,
                                                    const VideoDetails& base_processing_state_details,
                                                    const std::vector<ABIRFrameHeader>& input_frame_header,
                                                    int absolute_indx_start_0, int absolute_indx_stop_0, ABPFileType file_type, bool recalibrateTF)
{
    if (absolute_indx_stop_0 == 0)
	{
		absolute_indx_stop_0 = static_cast<int>(frame_data.size());
	}

    int nRows = 480;
    int nCols = 640;

    if (file_type == ABPFileType::ABP_D){
        nRows = 720;
        nCols = 1280;
    }
    int nRows2 = nRows/2;
    int nCols2 = nCols/2;

    uint relative_indx, track_id;
    
    if (calibration_model.calibration_available){
   
        for (unsigned int absolute_indx_0 = absolute_indx_start_0; absolute_indx_0 <= absolute_indx_stop_0; absolute_indx_0++)
        {
            relative_indx = absolute_indx_0 - absolute_indx_start_0;
            double frame_integration_time = input_frame_header[relative_indx].int_time;
            for (auto track : track_data[absolute_indx_0].tracks)
            {
                if ((track_data[absolute_indx_0].tracks[track.first].peak_irradiance<=0) || (recalibrateTF))
                {
                    track_id = (track.first);
                    cv::Rect bbox;

                    bbox.x = frame_data[relative_indx].tracks[track_id].bbox_x;
                    bbox.y = frame_data[relative_indx].tracks[track_id].bbox_y;
                    bbox.height = frame_data[relative_indx].tracks[track_id].bbox_height;
                    bbox.width = frame_data[relative_indx].tracks[track_id].bbox_width;

                    std::array<double, 3> measurements = {0,0,0};

                    measurements = SharedTrackingFunctions::CalculateSumCounts(relative_indx, bbox, base_processing_state_details, frame_integration_time, calibration_model);
                    (track_data[absolute_indx_0].tracks[track.first].peak_irradiance) = measurements[0];
                    (track_data[absolute_indx_0].tracks[track.first].mean_irradiance) = measurements[1];
                    (track_data[absolute_indx_0].tracks[track.first].sum_irradiance) = measurements[2];
                }
    
            }

        }
    }

}
