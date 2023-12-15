#pragma once

#include <vector>

struct video_details {
	int x_pixels, y_pixels, number_pixels;

	std::vector<std::vector<uint16_t>> frames_16bit;
	std::vector<std::vector<unsigned int>> histogram_data;

	std::vector<std::vector<double>> boresight_pointing;
	std::vector<std::vector<double>> tgt_az_el;

	video_details();
	~video_details();

	void set_image_size(int x, int y);

	void set_video_frames(std::vector<std::vector<uint16_t>> video_in);
};