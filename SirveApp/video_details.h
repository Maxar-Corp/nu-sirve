#pragma once

#include <vector>

struct VideoDetails {
	int x_pixels, y_pixels, max_value;
	std::vector<std::vector<uint16_t>> frames_16bit;
};
