#pragma once

#ifndef ABPNUC_READER_H
#define ABPNUC_READER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <type_traits>
#include <string>

#include <sys/stat.h>
#include <qprogressdialog.h>
#include <qinputdialog.h>

#include "binary_file_reader.h"
#include "binary_file_reader.cpp"

struct ABPNUC_Frame
{
	int frame_number;
	uint ir_temperature, tec_temperature_x100, tec_temperature_t1_x100, tec_temperature_t2_x100, tec_temperature_t3_x100,
		nuc_environment, measured_det_cal_factor, scene_mean_t1, scene_mean_t2, scene_mean_t3, scene_mean;

	double frame_time, seconds, ambient, afocal1, afocal2, ir_atherm, ir_integration_time_usec, detector_temperature;

};

//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------

class ABPNUC_Data : private BinaryFileReader
{
public:
	ABPNUC_Data(char* path);
	~ABPNUC_Data();

	const char* full_file_path;
	int read_status, number_of_frames;
	std::vector<ABPNUC_Frame> data;

	void read_apbnuc_file();

};

#endif
