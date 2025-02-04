#include "data_export.h"

void DataExport::WriteTrackDataToCsv(   std::string save_path,
                                        std::vector<PlottingFrameData> frame_data,
                                        std::vector<PlottingTrackFrame> track_data,
                                        std::vector<ManualPlottingTrackFrame> manual_track_data,
                                        int min_frame, int max_frame)
{
    if (max_frame == 0)
	{
		max_frame = static_cast<int>(frame_data.size());
	}

	std::ofstream myfile;
	myfile.open(save_path);

	unsigned int initial_frame = min_frame-1;
	unsigned int final_frame = max_frame;

    std::string frame_time, julian_date, seconds_past_midnight, frame_number, track_id, azimuth, elevation, centroid_x_boresight, centroid_y_boresight, centroid_x, centroid_y, counts, timing_offset;

	// export header
    myfile << "Frame Number, Frame Time, Julian Date, Second Past Midnight, Timeing Offset, Data Type, Track ID, Azimuth (deg), Elevation (deg), Centroid X Boresight, Centroid Y Boresight, Centroid X, Centroid Y, Sensor Counts" << std::endl;

	for (unsigned int i = initial_frame; i < final_frame; i++)
	{
        frame_number = std::to_string(i + 1);
        frame_time = std::to_string(frame_data[i].frame_time);
        julian_date = std::to_string(frame_data[i].julian_date);
		seconds_past_midnight = std::to_string(frame_data[i].seconds_past_midnight);
        timing_offset = std::to_string(frame_data[i].timing_offset);
		track_id = std::to_string(0);
		azimuth = std::to_string(frame_data[i].azimuth_sensor);
		elevation = std::to_string(frame_data[i].elevation_sensor);
		counts = std::to_string(0);

        myfile << frame_number << ", " << frame_time << ", " << julian_date << ", " << seconds_past_midnight << ", " << timing_offset << ", Boresight, " << track_id << ", " << azimuth  << ", " << elevation << ", " << "0" << ", " << "0" << ", " << "0" << ", " << "0"<< ", " << counts << std::endl;

        for (size_t j = 0; j < track_data[i].details.size(); j++)
        {
            track_id = std::to_string(track_data[i].details[j].track_id);
            azimuth = std::to_string(track_data[i].details[j].azimuth);
            elevation = std::to_string(track_data[i].details[j].elevation);

            centroid_x_boresight = std::to_string(track_data[i].details[j].centroid.centroid_x);
            centroid_y_boresight = std::to_string(track_data[i].details[j].centroid.centroid_y);
            centroid_x = std::to_string(track_data[i].details[j].centroid.centroid_x + 320);
            centroid_y = std::to_string(track_data[i].details[j].centroid.centroid_y + 240);
            counts = std::to_string(track_data[i].details[j].irradiance);

            myfile  << frame_number << ", " << frame_time << ", " << julian_date << ", " << seconds_past_midnight << ", " << timing_offset << ", OSM Track, " << track_id << ", " << azimuth << ", " << elevation << ", " << centroid_x_boresight << ", " << centroid_y_boresight << ", " << centroid_x << ", " << centroid_y << ", " << counts << std::endl;
        }

        for (auto track : manual_track_data[i].tracks)
        {
            track_id = std::to_string(track.first);
            azimuth = std::to_string(manual_track_data[i].tracks[track.first].azimuth);
            elevation = std::to_string(manual_track_data[i].tracks[track.first].elevation);

            centroid_x_boresight = std::to_string(track.second.centroid.centroid_x - 320);
            centroid_y_boresight = std::to_string(track.second.centroid.centroid_y - 240);
            centroid_x = std::to_string(track.second.centroid.centroid_x);
            centroid_y = std::to_string(track.second.centroid.centroid_y);

            counts = std::to_string(track.second.irradiance);

            myfile  << frame_number << ", " << frame_time << ", " << julian_date << ", " << seconds_past_midnight << ", " << timing_offset << ", Manual/Auto Track, " << track_id << ", " << azimuth << ", " << elevation << ", " << centroid_x_boresight << ", " << centroid_y_boresight << ", " << centroid_x << ", " << centroid_y << ", " << counts << std::endl;
        }
	}

	myfile.close();
}
