#include "data_export.h"

void DataExport::WriteTrackDataToCsv(   std::string save_path,
                                        std::vector<PlottingFrameData> frame_data,
                                        std::vector<PlottingTrackFrame> track_data,
                                        std::vector<ManualPlottingTrackFrame> manual_track_data,
                                        std::vector<TrackFrame> osm_track_frame_data,
                                        int min_frame, int max_frame)
{
    if (max_frame == 0)
	{
		max_frame = static_cast<int>(frame_data.size());
	}

	std::ofstream myfile;
	myfile.open(save_path);

	unsigned int initial_frame = min_frame;
	unsigned int final_frame = max_frame;

    std::string epoch_seconds, frame_number, track_id, azimuth, elevation, centroid_x, centroid_y, counts;

	// export header
    myfile << "Frame ID, Epoch Second, Data Type, Track ID, Azimuth (deg), Elevation (deg), Centroid X, Centroid Y, Sensor Counts" << std::endl;

	for (unsigned int i = initial_frame; i < final_frame; i++)
	{
		epoch_seconds = std::to_string(frame_data[i].seconds_past_midnight);
        frame_number = std::to_string(i);
		track_id = std::to_string(0);
		azimuth = std::to_string(frame_data[i].azimuth_sensor);
		elevation = std::to_string(frame_data[i].elevation_sensor);
		counts = std::to_string(0);

        myfile << frame_number << ", " << epoch_seconds << ", Boresight, " << track_id << ", " << azimuth  << ", " << elevation << ", " << "0" << ", " << "0" << ", " << counts << std::endl;

		size_t num_tracks = track_data[i].details.size();
		for (size_t j = 0; j < num_tracks; j++)
		{
            track_id = std::to_string(track_data[i].details[j].track_id);
            azimuth = std::to_string(track_data[i].details[j].azimuth);
            elevation = std::to_string(track_data[i].details[j].elevation);

            std::map<int, TrackDetails> track_map = osm_track_frame_data[i].tracks;
            centroid_x = std::to_string(track_map.find(track_data[i].details[j].track_id)->second.centroid_x);
            centroid_y = std::to_string(track_map.find(track_data[i].details[j].track_id)->second.centroid_y);

            counts = std::to_string(track_data[i].details[j].irradiance);

            myfile << frame_number << ", " << epoch_seconds  << ", OSM Track, " << track_id << ", " << azimuth << ", " << elevation << ", " << centroid_x << ", " << centroid_y << ", " << counts << std::endl;
		}

        for (auto track : manual_track_data[i].tracks)
        {
            track_id = std::to_string(track.first);
            azimuth = std::to_string(manual_track_data[i].tracks[track.first].azimuth);
            elevation = std::to_string(manual_track_data[i].tracks[track.first].elevation);

            centroid_x = std::to_string(track.second.centroid.centroid_x);
            centroid_y = std::to_string(track.second.centroid.centroid_y);

            myfile << frame_number << ", " << epoch_seconds << ", Manual Track, " << track_id << ", " << azimuth << ", " << elevation << ", " << centroid_x << ", " << centroid_y << ", " << "0" << std::endl;
        }
	}

	myfile.close();
}
