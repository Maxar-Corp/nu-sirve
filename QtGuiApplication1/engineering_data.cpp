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


void Engineering_Data::get_luminosity_data() {

	std::vector<Track_Irradiance> out;
	unsigned int num_messages = osm.size();

	for (unsigned int i = 0; i < num_messages; i++) {

		uint32_t number_tracks = osm[i].data.num_tracks;
		if (number_tracks > 0) {

			for (unsigned int track_index = 0; track_index < number_tracks; track_index++) {

				unsigned int num_track_ids = out.size();
				bool found_track_id = false;

				//TODO Assumes that there is only a single ir band. Function and struct will need to be updated if multiple bands are being tracked

				if (track_index < num_track_ids) {
					double irradiance_value = osm[i].data.track_data[track_index].ir_measurements[0].ir_radiance[0];

					out[track_index].frame_number.push_back(i);
					out[track_index].irradiance.push_back(irradiance_value);
					out[track_index].maximum_frame_number = i;

					if (irradiance_value < out[track_index].minimum_irradiance)
					{
						out[track_index].minimum_irradiance = irradiance_value;
					}

					if (irradiance_value > out[track_index].maximum_irradiance)
					{
						out[track_index].maximum_irradiance = irradiance_value;
					}

				}
				else {
					Track_Irradiance temp;

					temp.frame_number.push_back(i);
					temp.irradiance.push_back(osm[i].data.track_data[track_index].ir_measurements[0].ir_radiance[0]);

					temp.minimum_frame_number = i;
					temp.maximum_frame_number = i;
					temp.minimum_irradiance = osm[i].data.track_data[track_index].ir_measurements[0].ir_radiance[0];
					temp.maximum_irradiance = temp.minimum_irradiance;

					out.push_back(temp);
				}

			}

		}

	}

	emit plot_luminosity(out);
}

void Engineering_Data::get_azimuth_elevation_data()
{
	Plotting_Data out;

	for (unsigned int i = 0; i < osm.size(); i++) {

		double azimuth = osm[i].data.az_el_boresight[0];
		double elevation = osm[i].data.az_el_boresight[1];

		out.azimuth.push_back(azimuth);
		out.elevation.push_back(elevation);
		out.frame_time.push_back(osm[i].data.julian_date);
		out.frame_number.push_back(i + 1);
	}

	out.max_frame = out.frame_number.back();
	out.min_frame = out.frame_number[0];

	emit plot_azimuth_elevation(out, true, true);
}


