#include "engineering_data.h"

Engineering_Data::Engineering_Data(std::vector<Frame> & osm_data)
{
	osm = osm_data;
	max_number_tracks = 0;
	extract_engineering_data();

}

Engineering_Data::~Engineering_Data()
{

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
		frame_numbers.push_back(i + 1);
	}

}

