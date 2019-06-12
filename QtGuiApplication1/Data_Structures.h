#pragma once

#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <iostream>
#include <vector>
#include <map>

struct FrameHeader {

	uint64_t authorization;

	uint32_t classification, type, priority, oper_indicator, info_source,
		info_destination, transaction_id,
		ack_req_indicator, ack_response, cant_pro_reason, message_length, software_version;

	double time_generated_seconds;

};

struct MessageHeader
{
	uint64_t size;
	double seconds;
};

struct IR_Data {
	uint32_t band_id, num_measurements;

	std::vector<double> time, ir_radiance, ir_sigma;
};

struct TrackData {

	uint32_t track_id, sensor_type, num_bands;

	uint32_t roiBLX, roiBLY, roiURX, roiURY, num_pixels, object_type;

	double semi_major_axis, semi_minor_axis, orientation_angle, maxX, maxY;
	double frame_stabilize_x, frame_stabilize_y, stable_frame_shift_x, stable_frame_shift_y, centroid_x, centroid_y, centroid_variance_x, centroid_variance_y, covariance;

	std::vector<IR_Data> ir_measurements;
	std::vector <double> az_el_track;

};

struct FrameData {

	uint32_t task_id, num_tracks;
	double frametime, i_fov_x, i_fov_y, julian_date, seconds_past_midnight;

	std::vector<double>  mrp, mrp_cov_rand, mrp_cov_bias, ecf, lla, dcm, az_el_boresight;

	std::vector<TrackData> track_data;
};

struct Frame {

	FrameHeader frame_header;
	MessageHeader msg_header;
	FrameData data;

};


struct Track_Irradiance {

	uint32_t track_id;
	uint32_t band_id;
	std::vector<double> frame_number;
	std::vector<double> irradiance;
	std::vector<double> frame_time;

};

struct Track_Tgts {

	std::vector<int> x_pixels;
	std::vector<int> y_pixels;

};


struct Plotting_Data {
	std::vector<double> azimuth;
	std::vector<double> elevation;
	std::vector<double> frame_time;
	std::vector<double> frame_number;

	std::vector<Track_Irradiance> irradiance_data;
	std::vector<Track_Tgts> ir_tgt_data;

	int min_frame, max_frame;
};

struct Irradiance_Msrmnt {

	unsigned int track_id, band_id;
	double centroid_x, centroid_y;
	double irradiance;
};

struct Plotting_Frame_Data {
	double azimuth_sensor, elevation_sensor, azimuth_p_tgt, elevation_p_tgt, julian_date, seconds_past_midnight;

	std::vector<Irradiance_Msrmnt> ir_data;
	//std::vector<Track_Tgts> ir_tgt_data;
};

enum x_plot_variables{frames , seconds_past_midnight, seconds_from_epoch};

#endif