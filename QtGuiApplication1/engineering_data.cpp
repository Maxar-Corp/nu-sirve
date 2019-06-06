#include "engineering_data.h"

Engineering_Data::Engineering_Data(std::vector<Frame> & osm_data)
{
	osm = osm_data;

}

Engineering_Data::~Engineering_Data()
{

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
				uint32_t current_track_id = osm[i].data.track_data[track_index].track_id;
				uint32_t current_band_id = osm[i].data.track_data[track_index].ir_measurements[0].band_id;
				unsigned int out_track_index = 0;

				for (out_track_index = 0; out_track_index < num_track_ids; ++out_track_index) {
					if (current_track_id == out[out_track_index].track_id) {
						found_track_id = true;
						break;
					}
				}

				if (found_track_id) {
					double irradiance_value = osm[i].data.track_data[track_index].ir_measurements[0].ir_radiance[0];

					out[out_track_index].frame_number.push_back(i);
					out[out_track_index].irradiance.push_back(irradiance_value);
					out[out_track_index].maximum_frame_number = i;

					if (irradiance_value < out[out_track_index].minimum_irradiance)
					{
						out[out_track_index].minimum_irradiance = irradiance_value;
					}

					if (irradiance_value > out[out_track_index].maximum_irradiance)
					{
						out[out_track_index].maximum_irradiance = irradiance_value;
					}

				}
				else {
					Track_Irradiance temp;
					temp.track_id = current_track_id;
					temp.band_id = current_band_id;
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
	Az_El_Data out;

	out.max_az = 0;
	out.min_az = 400;
	out.max_el = 0;
	out.min_el = 100;

	for (unsigned int i = 0; i < osm.size(); i++) {

		double azimuth = osm[i].data.az_el_boresight[0];
		double elevation = osm[i].data.az_el_boresight[1];

		out.azimuth.push_back(azimuth);
		out.elevation.push_back(elevation);
		out.frame_time.push_back(osm[i].data.julian_date);
		out.frame_number.push_back(i + 1);

		if (azimuth > out.max_az)
			out.max_az = azimuth;
		if (azimuth < out.min_az)
			out.min_az = azimuth;

		if (elevation > out.max_el)
			out.max_el = elevation;
		if (elevation < out.min_el)
			out.min_el = elevation;
	}

	out.max_frame = out.frame_number.back();
	out.min_frame = out.frame_number[0];

	emit plot_azimuth_elevation(out, true, true);
}

