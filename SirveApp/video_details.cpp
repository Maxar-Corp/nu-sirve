#include "video_details.h"

video_details::video_details() {
	number_of_bits = 0;
}

video_details::~video_details()
{
	clear_16bit_vector();
	histogram_data.clear();
}

void video_details::set_image_size(int x, int y)
{
	x_pixels = x;
	y_pixels = y;
	number_pixels = x * y;
}

void video_details::set_number_of_bits(int num_bits)
{
	number_of_bits = num_bits;
}

void video_details::set_video_frames(std::vector<std::vector<uint16_t>> video_in)
{
	frames_16bit = video_in;
}

void video_details::clear_16bit_vector()
{
	frames_16bit.clear();
}