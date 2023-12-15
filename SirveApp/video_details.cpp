#include "video_details.h"

video_details::video_details()
{
}

video_details::~video_details()
{
}

void video_details::set_image_size(int x, int y)
{
	x_pixels = x;
	y_pixels = y;
	number_pixels = x * y;
}

void video_details::set_video_frames(std::vector<std::vector<uint16_t>> video_in)
{
	frames_16bit = video_in;
}