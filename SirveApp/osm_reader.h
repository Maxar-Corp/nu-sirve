#pragma once

#ifndef OSM_READER_H
#define OSM_READER_H

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <type_traits>
#include <armadillo>
#include <math.h>
#include <qstring.h>

#include <excpt.h>

#include "location_input.h"
#include "binary_file_reader.h"
#include "binary_file_reader.cpp"
#include "logging.h"

#include "Data_Structures.h"
#include "support/earth.h"

const int kMAX_NUMBER_ITERATIONS = 100000;
const double kSMALL_NUMBER = 0.000001;


class OSMReader: private BinaryFileReader
{
public:
    std::vector<Frame> data;

	bool read_osm_file(QString path);

	OSMReader();
	~OSMReader();
    
private:
	bool location_from_file;
	std::vector<double> file_ecef_vector;
	std::vector<double> frame_time;
	
    int LoadFile(char *file_path, bool input_combine_tracks = false);

    uint32_t FindMessageNumber();
    void InitializeVariables(uint32_t num_messages, bool combine_tracks);
    void LoadData(uint32_t num_messages, bool combine_tracks);
    void AddTrackToLastFrame();
	std::vector<double> get_lat_lon_alt(std::vector<double> ecf);

	MessageHeader ReadMessageHeader();
	FrameHeader ReadFrameHeader();
	FrameData ReadFrameData();

    TrackData GetTrackData(FrameData & input);
	

	std::vector<double> mr2dcos(std::vector<double> input);
	std::vector<double> calculation_azimuth_elevation(int x_pixel, int y_pixel, FrameData & input);
	
	double get_gps_time(double offset_gps_seconds);
};

#endif
