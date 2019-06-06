#pragma once

#ifndef OSM_READER_H
#define OSM_READER_H

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <type_traits>
#include <armadillo>

#include "Data_Structures.h"
#include "binary_file_reader.h"
#include "earth.h"

const int kMAX_NUMBER_ITERATIONS = 10000;
const double kSMALL_NUMBER = 1e-8;

struct Track_Irradiance{

    uint32_t track_id;
    uint32_t band_id;
    std::vector<double> frame_number;
    std::vector<double> irradiance;
	std::vector<double> frame_time;

	unsigned int minimum_frame_number;
	unsigned int maximum_frame_number;
	double minimum_irradiance;
	double maximum_irradiance;
};

struct Az_El_Data {
	std::vector<double> azimuth;
	std::vector<double> elevation;
	std::vector<double> frame_time;
	std::vector<double> frame_number;

	int min_frame, max_frame;
	double min_az, max_az, min_el, max_el;
};


class OSMReader: private BinaryFileReader
{
public:

    uint32_t num_messages;
    std::vector<Frame> data;
    bool contains_data;

	OSMReader();
	~OSMReader();
    int LoadFile(char *file_path, bool input_combine_tracks = false);
    
	// Pull data from file
	std::vector<Track_Irradiance> Get_Irradiance_Data();
	Az_El_Data Get_Boresight_Azimuth_Elevation();

private:

	bool combine_tracks;	

	std::vector<double> frame_time;

	//TODO write the check file function
    int CheckFilePath();
    void FindMessageNumber();
    void InitializeVariables();
    void LoadData();
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
