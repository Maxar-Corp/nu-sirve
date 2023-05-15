#include "video_details.h"

video_details::video_details() {

	number_of_bits = 0;

	int number_of_enumerations = (int)Video_Parameters::number_of_parameters;

	for (int i = 0; i < number_of_enumerations; i++)
	{
		properties.insert(std::pair<Video_Parameters, bool>(Video_Parameters(i), false));
	}

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

template <typename Map>
bool video_details::map_compare(Map const &lhs, Map const &rhs) {
	// No predicate needed because there is operator== for pairs already.
	return lhs.size() == rhs.size()
		&& std::equal(lhs.begin(), lhs.end(),
			rhs.begin());
}

bool video_details::operator==(const video_details  &f) {

	//https://stackoverflow.com/questions/16093413/comparison-operator-overloading-for-a-struct-symmetrically-comparing-my-struct
	//https://stackoverflow.com/questions/8473009/how-to-efficiently-compare-two-maps-of-strings-in-c	

	return map_compare(properties, f.properties);
}