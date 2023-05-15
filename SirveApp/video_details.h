#pragma once

#include <map>
#include <vector>

enum struct Video_Parameters
{
	original,
	background_subtraction,
	non_uniformity_correction,
	deinterlace_max_absolute_value,
	deinterlace_centroid,
	deinterlace_avg_cross_correlation,

	number_of_parameters
};

struct video_details {

	std::map<Video_Parameters, bool> properties;
	int number_of_bits, x_pixels, y_pixels, number_pixels;

	std::vector<std::vector<uint16_t>> frames_16bit;
	std::vector<std::vector<unsigned int>> histogram_data;

	std::vector<std::vector<double>> boresight_pointing;
	std::vector<std::vector<double>> tgt_az_el;

	video_details();
	~video_details();

	void set_image_size(int x, int y);
	void set_number_of_bits(int num_bits);

	void set_video_frames(std::vector<std::vector<uint16_t>> video_in);
	void clear_16bit_vector();

	bool operator==(const video_details &);
	template <typename Map> bool map_compare(Map const &lhs, Map const &rhs);
};