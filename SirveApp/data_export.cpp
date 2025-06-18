#include "data_export.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

void DataExport::WriteTrackDataToCsv(   std::string save_path,
                                        std::vector<PlottingFrameData> frame_data,
                                        std::vector<PlottingTrackFrame> track_data,
                                        std::vector<ManualPlottingTrackFrame> manual_track_data,
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

	std::ofstream myfile;
	myfile.open(save_path);   
	unsigned int initial_frame = min_frame;
	unsigned int final_frame = max_frame;
    int frame_number, track_id, counts, centroid_x_boresight, centroid_y_boresight, centroid_x, centroid_y;
    double frame_time, julian_date, seconds_past_midnight, azimuth, elevation, timing_offset;

	// export header
    myfile << "Frame Number, Frame Time, Julian Date, Second Past Midnight, Timeing Offset, Data Type, Track ID, Azimuth (deg), Elevation (deg), Centroid X Boresight, Centroid Y Boresight, Centroid X, Centroid Y, Integrated Adjusted Counts" << std::endl;

	for (unsigned int i = initial_frame; i < final_frame; i++)
	{
        frame_number = i+1;
        frame_time = (frame_data[i].frame_time);
        julian_date = (frame_data[i].julian_date);
		seconds_past_midnight = (frame_data[i].seconds_past_midnight);
        timing_offset = (frame_data[i].timing_offset);
		track_id = (0);
		azimuth = (frame_data[i].azimuth_sensor);
		elevation = (frame_data[i].elevation_sensor);
		counts = (0);
        myfile  << std::fixed << std::setprecision(9);   
        myfile  << frame_number << ", "  << frame_time << ", " << julian_date << ", " << seconds_past_midnight << ", " << timing_offset << ", Boresight, " << track_id << ", " << azimuth  << ", " << elevation << ", " << "0" << ", " << "0" << ", " << "0" << ", " << "0"<< ", " << counts << std::endl;

        for (size_t j = 0; j < track_data[i].details.size(); j++)
        {
            track_id = (track_data[i].details[j].track_id);
            azimuth = (track_data[i].details[j].azimuth);
            elevation = (track_data[i].details[j].elevation);

            centroid_x_boresight = track_data[i].details[j].centroid.centroid_x;
            centroid_y_boresight = track_data[i].details[j].centroid.centroid_y;
            centroid_x =  centroid_x_boresight + nCols2;
            centroid_y = centroid_y_boresight + nRows2;
            counts = (track_data[i].details[j].sum_relative_counts);

            myfile << frame_number << ", " << frame_time << ", "<< julian_date << ", " << seconds_past_midnight << ", " << timing_offset << ", OSM Track, " << track_id << ", " << azimuth << ", " << elevation << ", " << centroid_x_boresight << ", " << centroid_y_boresight << ", " << centroid_x << ", " << centroid_y << ", " << counts << std::endl;
        }

        for (auto track : manual_track_data[i].tracks)
        {
            track_id = (track.first);
            azimuth = (manual_track_data[i].tracks[track.first].azimuth);
            elevation = (manual_track_data[i].tracks[track.first].elevation);
            centroid_x = track.second.centroid.centroid_x;
            centroid_y = track.second.centroid.centroid_y;
            centroid_x_boresight =  centroid_x - nCols2;
            centroid_y_boresight =  centroid_y - nRows2;
            counts = (track.second.sum_relative_counts);

            myfile  << frame_number << ", " << frame_time << ", "<< julian_date << ", " << seconds_past_midnight << ", " << timing_offset << ", Manual/Auto Track, " << track_id << ", " << azimuth << ", " << elevation << ", " << centroid_x_boresight << ", " << centroid_y_boresight << ", " << centroid_x << ", " << centroid_y << ", " << counts << std::endl;
        }
	}

	myfile.close();
}
