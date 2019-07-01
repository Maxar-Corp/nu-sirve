#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <qobject.h>

#include "Data_Structures.h"

enum struct Video_Parameters
{
	original,
	background_subtraction,
	non_uniformity_correction, 
	
	number_of_parameters
};

struct video_details {

	std::map<Video_Parameters, bool> properties;
	int number_of_bits, x_pixels, y_pixels, number_pixels;

	std::vector<std::vector<uint8_t>> frames_8bit;
	std::vector<std::vector<uint16_t>> frames_16bit;
	std::vector<std::vector<unsigned int>> histogram_data;

	std::vector<std::vector<double>> boresight_pointing;
	std::vector<std::vector<double>> tgt_az_el;

	video_details();
	~video_details();

	void set_image_size(int x, int y);
	void set_number_of_bits(int num_bits);
	
	void set_video_frames(std::vector<std::vector<uint8_t>> video_in);
	void set_video_frames(std::vector<std::vector<uint16_t>> video_in);
	void convert_16bit_to_8bit();

	void create_histogram_data();
	
	void clear_16bit_vector();
	void clear_8bit_vector();
	
	bool operator==(const video_details &);
	template <typename Map> bool map_compare(Map const &lhs, Map const &rhs);

};


class Video_Container : public QObject
{
	Q_OBJECT
	
	public:
		std::vector<video_details> something;
		int current_index;

		Video_Container();
		~Video_Container();

		bool display_original_data();
		bool display_data(video_details input);
		int find_data_index(video_details input);

	signals:
		void update_display_video(video_details &update);


	private:

};

