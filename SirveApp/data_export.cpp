#include "data_export.h"

void DataExport::write_track_date_to_csv(std::string save_path, std::vector<Plotting_Frame_Data> frame_data, std::vector<PlottingTrackFrame> track_data, int min_frame, int max_frame)
{
    if (max_frame == 0)
	{
		max_frame = static_cast<int>(frame_data.size());
	}

	std::ofstream myfile;
	myfile.open(save_path);

	unsigned int initial_frame = min_frame;
	unsigned int final_frame = max_frame;

	std::string epoch_seconds, track_id, azimuth, elevation, counts;

	// export header
	myfile << "Epoch Second,Track ID,Azimuth (deg),Elevation (deg),Sensor Counts" << std::endl;

	for (unsigned int i = initial_frame; i <= final_frame; i++)
	{
		epoch_seconds = std::to_string(frame_data[i].seconds_past_midnight);
		track_id = std::to_string(0);
		azimuth = std::to_string(frame_data[i].azimuth_sensor);
		elevation = std::to_string(frame_data[i].elevation_sensor);
		counts = std::to_string(0);

		myfile << epoch_seconds << ", " << track_id << ", " << azimuth  << ", " << elevation << ", " << counts << std::endl;

		size_t num_tracks = track_data[i].details.size();
		for (size_t j = 0; j < num_tracks; j++)
		{
			track_id = std::to_string(track_data[i].details[j].track_id);
			azimuth = std::to_string(track_data[i].details[j].azimuth);
			elevation = std::to_string(track_data[i].details[j].elevation);
			counts = std::to_string(track_data[i].details[j].irradiance);

			myfile << epoch_seconds << ", " << track_id << ", " << azimuth << ", " << elevation << ", " << counts << std::endl;
		}

	}

	myfile.close();
}