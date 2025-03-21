#ifndef VIDEO_DETAILS_H
#define VIDEO_DETAILS_H

#include <vector>

struct VideoDetails {
	int x_pixels = 0;
	int y_pixels = 0;
	int max_value = 0;
	std::vector<std::vector<uint16_t>> frames_16bit;
};

#endif // VIDEO_DETAILS_H
