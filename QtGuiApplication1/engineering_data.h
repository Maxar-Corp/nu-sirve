#pragma once

#ifndef ENGINEERING_DATA_H
#define ENGINEERING_DATA_H

#include <iostream>
#include <string>

#include <vector>
#include <math.h>
#include <qwidget.h>

#include "osm_reader.h"
#include "Data_Structures.h"

class Engineering_Data : public	QWidget
{
	Q_OBJECT;
	public:
		Engineering_Data(std::vector<Frame> & osm_data);
		~Engineering_Data();

		std::vector<double>frame_numbers, julian_date, seconds_from_midnight;
		std::vector<Plotting_Frame_Data> frame_data;
		int max_number_tracks;

	private:

		std::vector<Frame> osm;
		Plotting_Data data;
				
		void extract_engineering_data();
};

#endif