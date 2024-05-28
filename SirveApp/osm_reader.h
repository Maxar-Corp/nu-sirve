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
#include "Data_Structures.h"

#include "support/earth.h"
#include "support/az_el_calculation.h"

const int kMAX_NUMBER_ITERATIONS = 100000;
const double kSMALL_NUMBER = 0.000001;


class OSMReader: private BinaryFileReader
{
public:
    std::vector<Frame> ReadOsmFileData(QString path);

	OSMReader();
	~OSMReader();
    
private:
	bool location_from_file;
	std::vector<double> file_ecef_vector;
	std::vector<double> frame_time;
	
    std::vector<Frame> LoadFrameVectors(const char *file_path, bool input_combine_tracks = false);

    uint32_t FindMessageNumber();
    std::vector<Frame> LoadData(uint32_t num_messages, bool combine_tracks);
    void AddTrackToLastFrame(std::vector<Frame> &data);
    std::vector<double> CalculateLatLonAltVector(std::vector<double> ecf);

	MessageHeader ReadMessageHeader();
	FrameHeader ReadFrameHeader();
	FrameData ReadFrameData();

    TrackData GetTrackData(FrameData & input);
	
    std::vector<double> CalculateDirectionCosineMatrix(std::vector<double> input);
    std::vector<double> CalculateAzimuthElevation(int x_pixel, int y_pixel, FrameData & input);
	
    double CalculateGpsUtcJulianDate(double offset_gps_seconds);
};

#endif
