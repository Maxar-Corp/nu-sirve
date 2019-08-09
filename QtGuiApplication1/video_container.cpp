#include "video_container.h"

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
	clear_8bit_vector();
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

void video_details::set_video_frames(std::vector<std::vector<uint8_t>> video_in)
{
	frames_8bit = video_in;
	create_histogram_data();
}

void video_details::set_video_frames(std::vector<std::vector<uint16_t>> video_in)
{
	frames_16bit = video_in;
	convert_16bit_to_8bit();
	create_histogram_data();
}

void video_details::clear_16bit_vector()
{
	//int num_frames = frames_16bit.size();
	//
	//for (int j = 0; j < num_frames; j++)
	//{
	//	delete[] frames_16bit[j];
	//}

	frames_16bit.clear();
}

void video_details::clear_8bit_vector()
{
	//int num_frames = frames_8bit.size();

	//for (int j = 0; j < num_frames; j++)
	//{
	//	delete[] frames_8bit[j];
	//}

	frames_8bit.clear();
}

void video_details::convert_16bit_to_8bit() {

	uint16_t total_range = std::pow(2, number_of_bits);
	int number_of_frames = frames_16bit.size();

	QProgressDialog progress("", QString(), 0, 1000);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimum(0);
	progress.setMaximum(number_of_frames);
	progress.setWindowTitle(QString("Creating Video"));
	progress.setLabelText(QString("Down-converting video frames..."));

	//clear_8bit_vector();
	frames_8bit.clear();

	for (size_t i = 0; i < number_of_frames; i++)
	{
		progress.setValue(i);
		std::vector<uint8_t> raw_8bit_data;
		raw_8bit_data.reserve(number_pixels);

		for (int j = 0; j < number_pixels; j++) {

			uint16_t value = frames_16bit[i][j];
			double new_value = 255 * 1.0 * value / total_range;

			raw_8bit_data.push_back((uint8_t)new_value);
		}

		frames_8bit.push_back(raw_8bit_data);
	}
}

void video_details::create_histogram_data()
{

	int number_of_frames = frames_8bit.size();
	int number_of_bins = std::pow(2, number_of_bits);

	histogram_data.clear();
	histogram_data.reserve(number_of_frames);

	QProgressDialog progress("", QString(), 0, 1000);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimum(0);
	progress.setMaximum(number_of_frames);
	progress.setWindowTitle(QString("Creating Histogram"));
	progress.setLabelText(QString("Creating histogram data..."));

	for (int frame_number = 0; frame_number < number_of_frames; frame_number++) {

		progress.setValue(frame_number);
		std::vector<unsigned int> frame_histogram(number_of_bins, 0);
		
		for (int pixel_index = 0; pixel_index < number_pixels; pixel_index++)
		{
			uint8_t value = frames_8bit[frame_number][pixel_index];
			uint8_t index = (uint8_t)value;

			// Check if exceeds maximum bins, puts in last bin
			if (index > number_of_bins)
				index = number_of_bins - 1;

			frame_histogram[index]++;
		}

		histogram_data.push_back(frame_histogram);
	}
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

Video_Container::Video_Container()
{

}

Video_Container::~Video_Container()
{
	//int num_videos = something.size();
	//for (int i = 0; i < num_videos; i++)
	//{
	//	int num_frames = something[i].frames_8bit.size();

	//	for (int j = 0; j < num_frames; j++)
	//	{
	//		delete[] something[i].frames_8bit[j];
	//		//delete[] something[i].frames_16bit[j];
	//	}

	//	something[i].frames_8bit.clear();
	//	//something[i].frames_16bit.clear();
	//}	
}

bool Video_Container::display_original_data()
{
	int number_videos = something.size();

	if (number_videos > 0) {
		
		emit update_display_video(something[0]);

		return true;
	}

	return false;
}

bool Video_Container::display_data(video_details input)
{
	int number_videos = something.size();
	
	for (int i = 0; i < number_videos; i++)
	{
		if (input == something[i]) {
			emit update_display_video(something[i]);

			return true;
		}
	}

	return false;
}

int Video_Container::find_data_index(video_details input)
{
	int number_videos = something.size();

	for (int i = 0; i < number_videos; i++)
	{
		if (input == something[i]) {

			return i;
		}
	}

	return -1;
}
