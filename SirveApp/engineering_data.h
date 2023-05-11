#pragma once

#ifndef ENGINEERING_DATA_H
#define ENGINEERING_DATA_H

#include <iostream>
#include <string>

#include <vector>
#include <math.h>
#include <qwidget.h>

#include <armadillo>
#include "support/jtime.h"
#include "osm_reader.h"
#include "Data_Structures.h"

class Engineering_Data : public	QWidget
{
	Q_OBJECT;
	public:
		Engineering_Data(const std::vector<Frame> & osm_frames);
		~Engineering_Data();

		std::vector<double>frame_numbers, julian_date, seconds_from_midnight, seconds_from_epoch;
		
		std::vector<Plotting_Frame_Data> frame_data;
		std::vector<Track_Irradiance> track_irradiance_data;

		void update_epoch_time(double new_julian_date);
		std::vector<double>get_epoch(const std::vector<Frame> & osm_frames);
		std::vector<double>get_adj_epoch(double num_days, const std::vector<Frame> & osm_frames);
		void write_track_date_to_csv(std::string save_path, int min_frame, int max_frame);
		
		double get_offset_time();
		void set_offset_time(double offset);

		double get_epoch_time_from_index(int index);
		std::vector<double> get_julian_date();
		std::vector<double> get_seconds_from_midnight();
		std::vector<double> get_seconds_from_epoch();
		std::vector<Plotting_Frame_Data> get_plotting_frame_data();
		std::vector<Plotting_Frame_Data> get_subset_plotting_frame_data(int index0, int index1);
		std::vector<Track_Irradiance> get_track_irradiance_data();
				
		int max_number_tracks;

	private:
		Plotting_Data data;
		double timing_offset, data_epoch_date, user_epoch_date;
				
		void extract_engineering_data(const std::vector<Frame> & osm_frames);
		void fill_irradiance_vector();
};

#endif