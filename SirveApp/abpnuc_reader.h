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

struct ABPNUCFrame
{
	int frame_number;
	uint ir_temperature, nuc_environment, measured_det_cal_factor;
	int32_t tec_temperature_x100, tec_temperature_t1_x100, tec_temperature_t2_x100, tec_temperature_t3_x100, scene_mean_t1, scene_mean_t2, scene_mean_t3, scene_mean;

	double frame_time, seconds, ambient, afocal1, afocal2, ir_atherm, ir_integration_time_usec, detector_temperature;

};

//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------

class ABPNUCData : private BinaryFileReader
{
public:
	ABPNUCData(char* path);
	~ABPNUCData();

	const char* full_file_path;
	int read_status, number_of_frames;
    std::vector<ABPNUCFrame> data;

	void ReadABPNUCFile();

};

#endif
