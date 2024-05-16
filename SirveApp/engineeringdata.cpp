#include "engineeringdata.h"

EngineeringData::EngineeringData(const std::vector<Frame> & osm_frames)
{
	extract_engineering_data(osm_frames);
	timing_offset = 0;

}

EngineeringData::~EngineeringData()
{

}

void EngineeringData::update_epoch_time(double new_julian_date)
{
	int length = seconds_from_epoch.size();

	user_epoch_date = new_julian_date;

}

std::vector<double> EngineeringData::get_epoch(const std::vector<Frame> & osm_frames)
{
	std::vector<double>out;

	double epoch0 = osm_frames[0].data.julian_date - osm_frames[0].data.seconds_past_midnight / 86400.0 + timing_offset / 86400.0;
	
	arma::vec date_time = jtime::DateTime(epoch0);
	out = arma::conv_to<std::vector<double>>::from(date_time);
	
	return out;
}

std::vector<double> EngineeringData::get_adj_epoch(double num_days, const std::vector<Frame> & osm_frames)
{
	std::vector<double>out;

	double epoch0 = osm_frames[0].data.julian_date - osm_frames[0].data.seconds_past_midnight / 86400.0 + timing_offset / 86400.0;
	epoch0 = epoch0 + num_days;

	arma::vec date_time = jtime::DateTime(epoch0);
	out = arma::conv_to<std::vector<double>>::from(date_time);

	return out;
}

double EngineeringData::get_offset_time() {
	
	return timing_offset;
}

void EngineeringData::set_offset_time(double offset)
{
	timing_offset = offset;
}

std::vector<double> EngineeringData::get_seconds_from_midnight()
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

std::vector<double> EngineeringData::get_seconds_from_epoch()
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

std::vector<Plotting_Frame_Data> EngineeringData::get_plotting_frame_data()
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

std::vector<Plotting_Frame_Data> EngineeringData::get_subset_plotting_frame_data(int index0, int index1)
{
	
	std::vector<Plotting_Frame_Data> temp_data = get_plotting_frame_data();
	std::vector<Plotting_Frame_Data>::const_iterator first = temp_data.begin() + index0;
	std::vector<Plotting_Frame_Data>::const_iterator last = temp_data.begin() + index1;
	std::vector<Plotting_Frame_Data> subset_data(first, last);
	
	return subset_data;
}

void EngineeringData::extract_engineering_data(const std::vector<Frame> & osm_frames)
{
	for (unsigned int i = 0; i < osm_frames.size(); i++) {

		Plotting_Frame_Data temp;

		// ----------------------------------------------------------------------------------------
		// Get Az-El of sensor and primary target
		temp.azimuth_sensor = osm_frames[i].data.az_el_boresight[0];
		temp.elevation_sensor = osm_frames[i].data.az_el_boresight[1];
		temp.julian_date = osm_frames[i].data.julian_date;
		temp.seconds_past_midnight = osm_frames[i].data.seconds_past_midnight;

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
