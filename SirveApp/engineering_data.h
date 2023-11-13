#pragma once

#ifndef ENGINEERING_DATA_H
#define ENGINEERING_DATA_H

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

		void update_epoch_time(double new_julian_date);
		std::vector<double>get_epoch(const std::vector<Frame> & osm_frames);
		std::vector<double>get_adj_epoch(double num_days, const std::vector<Frame> & osm_frames);
		
		double get_offset_time();
		void set_offset_time(double offset);

		std::vector<double> get_julian_date();
		std::vector<double> get_seconds_from_midnight();
		std::vector<double> get_seconds_from_epoch();
		std::vector<Plotting_Frame_Data> get_plotting_frame_data();
		std::vector<Plotting_Frame_Data> get_subset_plotting_frame_data(int index0, int index1);

	private:
		double timing_offset, data_epoch_date, user_epoch_date;
				
		std::vector<double>julian_date, seconds_from_midnight, seconds_from_epoch;
		std::vector<Plotting_Frame_Data> frame_data;
		
		void extract_engineering_data(const std::vector<Frame> & osm_frames);
};

#endif