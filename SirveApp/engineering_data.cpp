#include "engineering_data.h"

Engineering_Data::Engineering_Data(const std::vector<Frame> & osm_frames)
{
	extract_engineering_data(osm_frames);
	timing_offset = 0;

}

Engineering_Data::~Engineering_Data()
{

}

void Engineering_Data::update_epoch_time(double new_julian_date)
{
	int length = seconds_from_epoch.size();

	user_epoch_date = new_julian_date;

}

std::vector<double> Engineering_Data::get_epoch(const std::vector<Frame> & osm_frames)
{
	std::vector<double>out;

	double epoch0 = osm_frames[0].data.julian_date - osm_frames[0].data.seconds_past_midnight / 86400.0 + timing_offset / 86400.0;
	
	arma::vec date_time = jtime::DateTime(epoch0);
	out = arma::conv_to<std::vector<double>>::from(date_time);
	
	return out;
}

std::vector<double> Engineering_Data::get_adj_epoch(double num_days, const std::vector<Frame> & osm_frames)
{
	std::vector<double>out;

	double epoch0 = osm_frames[0].data.julian_date - osm_frames[0].data.seconds_past_midnight / 86400.0 + timing_offset / 86400.0;
	epoch0 = epoch0 + num_days;

	arma::vec date_time = jtime::DateTime(epoch0);
	out = arma::conv_to<std::vector<double>>::from(date_time);

	return out;
}

void Engineering_Data::write_track_date_to_csv(std::string save_path, int min_frame, int max_frame)
{
	// if (max_frame == 0)
	// {
	// 	max_frame = static_cast<int>(frame_data.size());
	// }

	// std::ofstream myfile;
	// myfile.open(save_path);

	// unsigned int initial_frame = min_frame;
	// unsigned int final_frame = max_frame;

	// std::vector<Plotting_Frame_Data> output = get_plotting_frame_data();

	// std::string epoch_seconds, track_id, azimuth, elevation, counts;

	// // export header
	// myfile << "Epoch Second,Track ID,Azimuth (deg),Elevation (deg),Sensor Counts" << std::endl;

	// for (unsigned int i = initial_frame; i <= final_frame; i++)
	// {
		
	// 	epoch_seconds = std::to_string(output[i].seconds_past_midnight + timing_offset);
	// 	track_id = std::to_string(0);
	// 	azimuth = std::to_string(output[i].azimuth_sensor);
	// 	elevation = std::to_string(output[i].elevation_sensor);
	// 	counts = std::to_string(0);

	// 	myfile << epoch_seconds << ", " << track_id << ", " << azimuth  << ", " << elevation << ", " << counts << std::endl;

	// 	unsigned int num_points = output[i].ir_data.size();
	// 	for (unsigned int j = 0; j < num_points; j++)
	// 	{
	// 		track_id = std::to_string(output[i].ir_data[j].track_id);
	// 		azimuth = std::to_string(output[i].ir_data[j].azimuth);
	// 		elevation = std::to_string(output[i].ir_data[j].elevation);
	// 		counts = std::to_string(output[i].ir_data[j].irradiance);

	// 		myfile << epoch_seconds << ", " << track_id << ", " << azimuth << ", " << elevation << ", " << counts << std::endl;
	// 	}

	// }

	// myfile.close();
}

double Engineering_Data::get_offset_time() {
	
	return timing_offset;
}

void Engineering_Data::set_offset_time(double offset)
{
	timing_offset = offset;
}

std::vector<double> Engineering_Data::get_julian_date()
{
	if (std::abs(timing_offset) < 0.001)
		return julian_date;

	std::vector<double> output;
	int length = julian_date.size();

	for (size_t i = 0; i < length; i++)
	{
		output.push_back(julian_date[i] + timing_offset / 86400.0);
	}

	return output;
}

std::vector<double> Engineering_Data::get_seconds_from_midnight()
{
	if (std::abs(timing_offset) < 0.001)
		return seconds_from_midnight;

	std::vector<double> output;
	int length = seconds_from_midnight.size();

	for (size_t i = 0; i < length; i++)
	{
		output.push_back(seconds_from_midnight[i] + timing_offset);
	}

	return output;
}

std::vector<double> Engineering_Data::get_seconds_from_epoch()
{
	if (std::abs(timing_offset) < 0.001 && std::abs(data_epoch_date - user_epoch_date) < 0.0000001)
		return seconds_from_epoch;

	std::vector<double> output;
	int length = seconds_from_epoch.size();

	for (size_t i = 0; i < length; i++)
	{
		double current_julian_date = julian_date[i] + timing_offset / 86400.0;
		double calculated_value = (current_julian_date - user_epoch_date) * 86400.0;

		output.push_back(calculated_value);
	}

	return output;
}

std::vector<Plotting_Frame_Data> Engineering_Data::get_plotting_frame_data()
{
	std::vector<Plotting_Frame_Data> output;
	int length = frame_data.size();

	if (std::abs(timing_offset) < 0.001)
		return frame_data;

	for (size_t i = 0; i < length; i++)
	{
		Plotting_Frame_Data temp;

		temp.azimuth_sensor = frame_data[i].azimuth_sensor;
		temp.elevation_sensor = frame_data[i].elevation_sensor;
		temp.julian_date = frame_data[i].julian_date + timing_offset / 86400.0;
		temp.seconds_past_midnight = frame_data[i].seconds_past_midnight + timing_offset;

		output.push_back(temp);
	}

	return output;
}

std::vector<Plotting_Frame_Data> Engineering_Data::get_subset_plotting_frame_data(int index0, int index1)
{
	
	std::vector<Plotting_Frame_Data> temp_data = get_plotting_frame_data();
	std::vector<Plotting_Frame_Data>::const_iterator first = temp_data.begin() + index0;
	std::vector<Plotting_Frame_Data>::const_iterator last = temp_data.begin() + index1;
	std::vector<Plotting_Frame_Data> subset_data(first, last);
	
	return subset_data;
}

std::vector<Track_Irradiance> Engineering_Data::get_track_irradiance_data()
{
	std::vector<Track_Irradiance> output;
	output = track_irradiance_data;

	if (std::abs(timing_offset) < 0.001 && std::abs(data_epoch_date - user_epoch_date) < 0.0000001)
		return track_irradiance_data;

	int num_tracks = output.size();

	for (size_t i = 0; i < num_tracks; i++)
	{
		int length = track_irradiance_data[i].frame_number.size();

		for (size_t j = 0; j < length; j++)
		{
			output[i].past_midnight[j] = track_irradiance_data[i].past_midnight[j] + timing_offset;
			output[i].julian_date[j] = track_irradiance_data[i].julian_date[j] + timing_offset / 86400.0;

			double calculated_value = (output[i].julian_date[j] - user_epoch_date) * 86400.0;
			output[i].past_epoch[j] = calculated_value;
		}
	}
	
	return output;
}

void Engineering_Data::extract_engineering_data(const std::vector<Frame> & osm_frames)
{
	for (unsigned int i = 0; i < osm_frames.size(); i++) {

		Plotting_Frame_Data temp;

		// ----------------------------------------------------------------------------------------
		// Get Az-El of sensor and primary target
		temp.azimuth_sensor = osm_frames[i].data.az_el_boresight[0];
		temp.elevation_sensor = osm_frames[i].data.az_el_boresight[1];
		temp.julian_date = osm_frames[i].data.julian_date;
		temp.seconds_past_midnight = osm_frames[i].data.seconds_past_midnight;

		// ----------------------------------------------------------------------------------------
		// Get irradiance track data
		int number_tracks = osm_frames[i].data.num_tracks;

		for (unsigned int track_index = 0; track_index < number_tracks; track_index++) {
			//TODO Assumes that there is only a single ir band. Function and struct will need to be updated if multiple bands are being tracked
			double irradiance = osm_frames[i].data.track_data[track_index].ir_measurements[0].ir_radiance[0];
			double azimuth = osm_frames[i].data.track_data[track_index].az_el_track[0];
			double elevation = osm_frames[i].data.track_data[track_index].az_el_track[1];


			// Filling the irradiance vector, moved from private method
			if (track_index < track_irradiance_data.size())
			{
				track_irradiance_data[track_index].frame_number.push_back(i + 1);

				track_irradiance_data[track_index].julian_date.push_back(temp.julian_date);
				track_irradiance_data[track_index].past_midnight.push_back(temp.seconds_past_midnight);
				track_irradiance_data[track_index].past_epoch.push_back(temp.seconds_past_midnight);

				track_irradiance_data[track_index].irradiance.push_back(irradiance);
				track_irradiance_data[track_index].azimuth.push_back(azimuth);
				track_irradiance_data[track_index].elevation.push_back(elevation);
			}
			else
			{
				Track_Irradiance temp_ti;

				temp_ti.track_id = track_index;

				temp_ti.frame_number.push_back(i + 1);
				temp_ti.julian_date.push_back(temp.julian_date);
				temp_ti.past_midnight.push_back(temp.seconds_past_midnight);
				temp_ti.past_epoch.push_back(temp.seconds_past_midnight);

				temp_ti.irradiance.push_back(irradiance);
				temp_ti.azimuth.push_back(azimuth);
				temp_ti.elevation.push_back(elevation);

				track_irradiance_data.push_back(temp_ti);
			}
		}

		frame_data.push_back(temp);
		julian_date.push_back(osm_frames[i].data.julian_date);
		seconds_from_midnight.push_back(osm_frames[i].data.seconds_past_midnight);
		seconds_from_epoch.push_back(osm_frames[i].data.seconds_past_midnight);
	}

	if (julian_date.size() > 0){

		arma::vec date = jtime::DateTime(julian_date[0]);
		double midnight_julian = jtime::JulianDate(date[0], date[1], date[2], 0, 0, 0);

		data_epoch_date = midnight_julian;
		user_epoch_date = midnight_julian;
	}
	else {
		data_epoch_date = 0.0;
		user_epoch_date = 0.0;
	}
}