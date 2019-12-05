#include "engineering_data.h"

Engineering_Data::Engineering_Data(std::vector<Frame> & osm_data)
{
	osm = osm_data;
	max_number_tracks = 0;
	extract_engineering_data();
	fill_irradiance_vector();

}

Engineering_Data::~Engineering_Data()
{

}

void Engineering_Data::update_epoch_time(double new_julian_date)
{
	int length = seconds_from_epoch.size();

	// update the vector
	for (int i = 0; i < length; i++)
	{
		seconds_from_epoch[i] = (julian_date[i] - new_julian_date) * 86400.;
	}

	//update tracks
	length = track_irradiance_data.size();
	for (int i = 0; i < length; i++)
	{
		int num_entries = track_irradiance_data[i].julian_date.size();
		for (int j = 0; j < num_entries; j++)
		{
			track_irradiance_data[i].past_epoch[j] = (track_irradiance_data[i].julian_date[j] - new_julian_date) * 86400.;
		}
	}

}

std::vector<double> Engineering_Data::get_epoch()
{
	std::vector<double>out;

	double epoch0 = track_irradiance_data[0].julian_date[0] - track_irradiance_data[0].past_epoch[0] / 86400;

	arma::vec date_time = jtime::DateTime(epoch0);
	out = arma::conv_to<std::vector<double>>::from(date_time);
	
	return out;
}

void Engineering_Data::extract_engineering_data()
{
	for (unsigned int i = 0; i < osm.size(); i++) {

		Plotting_Frame_Data temp;

		// ----------------------------------------------------------------------------------------
		// Get Az-El of sensor and primary target
		temp.azimuth_sensor = osm[i].data.az_el_boresight[0]; 	
		temp.elevation_sensor = osm[i].data.az_el_boresight[1];
		temp.julian_date = osm[i].data.julian_date;
		temp.seconds_past_midnight = osm[i].data.seconds_past_midnight;
		
		// ----------------------------------------------------------------------------------------
		// Get irradiance track data
		unsigned int number_tracks = osm[i].data.num_tracks;
		if (number_tracks > max_number_tracks)
		{
			max_number_tracks = number_tracks;
		}

		if (number_tracks > 0) {

			for (unsigned int track_index = 0; track_index < number_tracks; track_index++) {

				Irradiance_Msrmnt ir_data;
				//TODO Assumes that there is only a single ir band. Function and struct will need to be updated if multiple bands are being tracked

				ir_data.irradiance = osm[i].data.track_data[track_index].ir_measurements[0].ir_radiance[0];
				ir_data.centroid_x = osm[i].data.track_data[track_index].centroid_x;
				ir_data.centroid_y = osm[i].data.track_data[track_index].centroid_y;
				ir_data.band_id = osm[i].data.track_data[track_index].ir_measurements[0].band_id;
				ir_data.track_id = osm[i].data.track_data[track_index].track_id;
				ir_data.azimuth = osm[i].data.track_data[track_index].az_el_track[0];
				ir_data.elevation = osm[i].data.track_data[track_index].az_el_track[1];

				temp.ir_data.push_back(ir_data);
			}

			// Get primary target az-el, when it exists
			temp.azimuth_p_tgt = osm[i].data.track_data[0].az_el_track[0];
			temp.elevation_p_tgt = osm[i].data.track_data[0].az_el_track[1];
		}
		else {
			// Set values when no primary target
			temp.azimuth_p_tgt = -1001;
			temp.elevation_p_tgt = -1001;
		}

		frame_data.push_back(temp);
		julian_date.push_back(osm[i].data.julian_date);
		seconds_from_midnight.push_back(osm[i].data.seconds_past_midnight);
		seconds_from_epoch.push_back(osm[i].data.seconds_past_midnight);
		frame_numbers.push_back(i + 1);
	}

}

void Engineering_Data::fill_irradiance_vector()
{

	track_irradiance_data.reserve(max_number_tracks);
	int number_pts = frame_data.size();
		
	for (int i = 0; i < number_pts; i++)
	{
		
		int num_tracks_present = frame_data[i].ir_data.size();

		for (int j = 0; j < num_tracks_present; j++)
		{
			if (j < track_irradiance_data.size())
			{
				track_irradiance_data[j].frame_number.push_back(i + 1);
				track_irradiance_data[j].julian_date.push_back(frame_data[i].julian_date);
				track_irradiance_data[j].past_midnight.push_back(frame_data[i].seconds_past_midnight);
				track_irradiance_data[j].past_epoch.push_back(frame_data[i].seconds_past_midnight);

				track_irradiance_data[j].irradiance.push_back(frame_data[i].ir_data[j].irradiance);
				track_irradiance_data[j].azimuth.push_back(frame_data[i].ir_data[j].azimuth);
				track_irradiance_data[j].elevation.push_back(frame_data[i].ir_data[j].elevation);
			}
			else
			{
				Track_Irradiance temp;

				temp.band_id = 1;
				temp.track_id = j;

				temp.frame_number.push_back(i + 1);
				temp.julian_date.push_back(frame_data[i].julian_date);
				temp.past_midnight.push_back(frame_data[i].seconds_past_midnight);
				temp.past_epoch.push_back(frame_data[i].seconds_past_midnight);

				temp.irradiance.push_back(frame_data[i].ir_data[j].irradiance);
				temp.azimuth.push_back(frame_data[i].ir_data[j].azimuth);
				temp.elevation.push_back(frame_data[i].ir_data[j].elevation);

				track_irradiance_data.push_back(temp);
			}
		}	
	}
}

