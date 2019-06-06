#pragma once

#ifndef ENGINEERING_DATA_H
#define ENGINEERING_DATA_H

#include <iostream>
#include <string>

#include <vector>
#include <math.h>
#include <qwidget.h>

#include "osm_reader.h"

class Engineering_Data : public	QWidget
{
	Q_OBJECT;
	public:
		Engineering_Data(std::vector<Frame> & osm_data);
		~Engineering_Data();

		void get_luminosity_data();
		void get_azimuth_elevation_data();

	signals:
		void plot_luminosity(std::vector<Track_Irradiance> data);
		void plot_azimuth_elevation(Az_El_Data data, bool plot_azimuth, bool plot_frames);

	private:

		std::vector<Frame> osm;

};


#endif