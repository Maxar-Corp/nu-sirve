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
   
        for (unsigned int absolute_indx_0 = absolute_indx_start_0; absolute_indx_0 < absolute_indx_stop_0; absolute_indx_0++)
        {
            relative_indx = absolute_indx_0 - absolute_indx_start_0;
            double frame_integration_time = input_frame_header[relative_indx].int_time;
            for (size_t j = 0; j < track_data[absolute_indx_0].details.size(); j++)
            {
                cv::Rect bbox;
                bbox.x = track_data[absolute_indx_0].details[j].centroid.bbox_x;
                bbox.height = track_data[absolute_indx_0].details[j].centroid.bbox_width;
                bbox.width = track_data[absolute_indx_0].details[j].centroid.bbox_height;
                bbox.y = track_data[absolute_indx_0].details[j].centroid.bbox_y - bbox.height;

                std::array<double, 3> measurements = {0.,0.,0.};

                measurements = SharedTrackingFunctions::CalculateIrradiance(relative_indx, bbox, base_processing_state_details, frame_integration_time, calibration_model);
                track_data[absolute_indx_0].details[j].centroid.peak_irradiance = measurements[0];
                track_data[absolute_indx_0].details[j].centroid.mean_irradiance = measurements[1];
                track_data[absolute_indx_0].details[j].centroid.sum_irradiance = measurements[2];
                track_data[absolute_indx_0].details[j].nuc_calibration_file = calibration_model.path_nuc;
                track_data[absolute_indx_0].details[j].nuc_image_file = calibration_model.path_image;
                track_data[absolute_indx_0].details[j].mean_temp1 = calibration_model.user_selection1.temperature_mean;
                track_data[absolute_indx_0].details[j].mean_temp2 = calibration_model.user_selection2.temperature_mean;
                track_data[absolute_indx_0].details[j].start_frame1 = calibration_model.user_selection1.initial_frame;
                track_data[absolute_indx_0].details[j].num_frames1 = calibration_model.user_selection1.num_frames;
                track_data[absolute_indx_0].details[j].start_frame2 = calibration_model.user_selection2.initial_frame;
                track_data[absolute_indx_0].details[j].num_frames2 = calibration_model.user_selection2.num_frames;
                track_data[absolute_indx_0].details[j].num_frames2 = calibration_model.user_selection2.num_frames;  
                track_data[absolute_indx_0].details[j].peak_irradiance = measurements[0];
                track_data[absolute_indx_0].details[j].mean_irradiance = measurements[1];
                track_data[absolute_indx_0].details[j].sum_irradiance = measurements[2]; 
                track_data[absolute_indx_0].details[j].is_calibrated = true;
            }
        }
    }

}

void CalibrateExistingTracks::CalibrateManualTracks(   
                                                    CalibrationData& calibration_model,
                                                    std::vector<TrackFrame> & track_frame_data,
                                                    std::vector<ManualPlottingTrackFrame> & track_data,
                                                    const VideoDetails& base_processing_state_details,
                                                    const std::vector<ABIRFrameHeader>& input_frame_header,
                                                    int absolute_indx_start_0, int absolute_indx_stop_0, ABPFileType file_type, bool recalibrateTF)
{
    if (absolute_indx_stop_0 == 0)
	{
		absolute_indx_stop_0 = static_cast<int>(track_frame_data.size());
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
   
        for (unsigned int absolute_indx_0 = absolute_indx_start_0; absolute_indx_0 < absolute_indx_stop_0; absolute_indx_0++)
        {
            relative_indx = absolute_indx_0 - absolute_indx_start_0;
            if (relative_indx < track_frame_data.size()){
                double frame_integration_time = input_frame_header[relative_indx].int_time;
                for (auto track : track_data[absolute_indx_0].tracks)
                {
                    if (!track_data[absolute_indx_0].tracks[track.first].is_calibrated || recalibrateTF)
                    {
                        track_data[absolute_indx_0].tracks[track.first].is_calibrated = true;
                        track_data[absolute_indx_0].tracks[track.first].mean_temp1 = calibration_model.user_selection1.temperature_mean;
                        track_data[absolute_indx_0].tracks[track.first].mean_temp2 = calibration_model.user_selection2.temperature_mean;
                        track_data[absolute_indx_0].tracks[track.first].start_frame1 = calibration_model.user_selection1.initial_frame;
                        track_data[absolute_indx_0].tracks[track.first].start_frame2 = calibration_model.user_selection2.initial_frame;
                        track_data[absolute_indx_0].tracks[track.first].num_frames1 = calibration_model.user_selection1.num_frames;
                        track_data[absolute_indx_0].tracks[track.first].num_frames2 = calibration_model.user_selection2.num_frames;
                        track_data[absolute_indx_0].tracks[track.first].nuc_calibration_file = calibration_model.path_nuc;
                        track_data[absolute_indx_0].tracks[track.first].nuc_image_file = calibration_model.path_image;


                        track_id = (track.first);
                        cv::Rect bbox;

                        bbox.x = track_frame_data[relative_indx].tracks[track_id].bbox_x;
                        bbox.y = track_frame_data[relative_indx].tracks[track_id].bbox_y;
                        bbox.height = track_frame_data[relative_indx].tracks[track_id].bbox_height;
                        bbox.width = track_frame_data[relative_indx].tracks[track_id].bbox_width;

                        std::array<double, 3> measurements = {0.,0.,0.};
                        measurements = SharedTrackingFunctions::CalculateIrradiance(relative_indx, bbox, base_processing_state_details, frame_integration_time, calibration_model);

                        track_data[absolute_indx_0].tracks[track.first].peak_irradiance = measurements[0];
                        track_data[absolute_indx_0].tracks[track.first].mean_irradiance = measurements[1];
                        track_data[absolute_indx_0].tracks[track.first].sum_irradiance = measurements[2];
                        track_data[absolute_indx_0].tracks[track.first].centroid.peak_irradiance = measurements[0];
                        track_data[absolute_indx_0].tracks[track.first].centroid.mean_irradiance = measurements[1];
                        track_data[absolute_indx_0].tracks[track.first].centroid.sum_irradiance = measurements[2];
                        track_frame_data[relative_indx].tracks[track_id].peak_irradiance = measurements[0];
                        track_frame_data[relative_indx].tracks[track_id].mean_irradiance = measurements[1];
                        track_frame_data[relative_indx].tracks[track_id].sum_irradiance = measurements[2];
                        track_frame_data[relative_indx].tracks[track_id].is_calibrated = true;
                        track_frame_data[relative_indx].tracks[track_id].mean_temp1 = calibration_model.user_selection1.temperature_mean;
                        track_frame_data[relative_indx].tracks[track_id].mean_temp2 = calibration_model.user_selection2.temperature_mean;
                        track_frame_data[relative_indx].tracks[track_id].start_frame1 = calibration_model.user_selection1.initial_frame;
                        track_frame_data[relative_indx].tracks[track_id].start_frame2 = calibration_model.user_selection2.initial_frame;
                        track_frame_data[relative_indx].tracks[track_id].num_frames1 = calibration_model.user_selection1.num_frames;
                        track_frame_data[relative_indx].tracks[track_id].num_frames2 = calibration_model.user_selection2.num_frames;
                        track_frame_data[relative_indx].tracks[track_id].nuc_calibration_file = calibration_model.path_nuc;
                        track_frame_data[relative_indx].tracks[track_id].nuc_image_file = calibration_model.path_image;
                        track_data[absolute_indx_0].tracks[track.first].centroid.peak_irradiance = track_frame_data[relative_indx].tracks[track_id].peak_irradiance;
                        track_data[absolute_indx_0].tracks[track.first].centroid.mean_irradiance = track_frame_data[relative_indx].tracks[track_id].mean_irradiance;
                        track_data[absolute_indx_0].tracks[track.first].centroid.sum_irradiance = track_frame_data[relative_indx].tracks[track_id].sum_irradiance;
                    }
        
                }
            }

        }
    }

}
