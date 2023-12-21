#pragma once

#include <vector>

struct video_details {
	int x_pixels, y_pixels;
	std::vector<std::vector<uint16_t>> frames_16bit;
};