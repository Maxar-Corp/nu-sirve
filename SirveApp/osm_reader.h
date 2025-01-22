#pragma once

#ifndef OSM_READER_H
#define OSM_READER_H

#include <armadillo>
#include <excpt.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <qstring.h>
#include <stdexcept>
#include <type_traits>

#include "binary_file_reader.h"
#include "binary_file_reader.cpp"
#include "data_structures.h"
#include "location_input.h"
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

    std::vector<double> CalculateAzimuthElevation(int x_pixel, int y_pixel, FrameData & input);
    std::vector<double> CalculateDirectionCosineMatrix(std::vector<double> input);
    double CalculateGpsUtcJulianDate(double offset_gps_seconds);
    std::vector<double> CalculateLatLonAltVector(std::vector<double> ecf);

    uint32_t FindMessageNumber();
    TrackData GetTrackData(FrameData & input);
    std::vector<Frame> LoadData(uint32_t num_messages, bool combine_tracks);
    std::vector<Frame> LoadFrameVectors(const char *file_path, bool input_combine_tracks = false);

	FrameHeader ReadFrameHeader();
	FrameData ReadFrameData();
    MessageHeader ReadMessageHeader();
};

#endif
