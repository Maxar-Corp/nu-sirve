#pragma once

#ifndef ENGINEERING_DATA_H
#define ENGINEERING_DATA_H

#include <iostream>
#include <string>

#include <vector>
#include <math.h>
#include <qwidget.h>

#include <armadillo>
#include "jtime.h"
#include "osm_reader.h"
#include "Data_Structures.h"

class Engineering_Data : public	QWidget
{
	Q_OBJECT;
	public:
		Engineering_Data(std::vector<Frame> & osm_data);
		~Engineering_Data();

		std::vector<double>frame_numbers, julian_date, seconds_from_midnight, seconds_from_epoch;
		
		std::vector<Plotting_Frame_Data> frame_data;
		std::vector<Track_Irradiance> track_irradiance_data;

		void update_epoch_time(double new_julian_date);
		std::vector<double>get_epoch();
		std::vector<double>get_adj_epoch(double num_days);
		void write_track_date_to_csv(std::string save_path, int min_frame, int max_frame);
		
		int max_number_tracks;

	private:

		std::vector<Frame> osm;
		Plotting_Data data;
				
		void extract_engineering_data();
		void fill_irradiance_vector();
};

#endif