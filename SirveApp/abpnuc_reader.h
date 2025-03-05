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
    int         frame_number;
    uint        ir_temperature;
    uint        measured_det_cal_factor;
    uint        nuc_environment;
    int32_t     tec_temperature_x100, tec_temperature_t1_x100, tec_temperature_t2_x100, tec_temperature_t3_x100;
    int32_t     scene_mean_t1, scene_mean_t2, scene_mean_t3, scene_mean;

    double      ambient;
    double      afocal1, afocal2;
    double      detector_temperature;
    double      frame_time;
    double      ir_atherm, ir_integration_time_usec;
    double      seconds;
};

//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------

class ABPNUCData : private BinaryFileReader
{
public:
	ABPNUCData(char* path);
	~ABPNUCData();

    std::vector<ABPNUCFrame>    data;
    const char*                 full_file_path;
    int                         number_of_frames;
    int                         read_status;

	void ReadABPNUCFile();

};

#endif
