#pragma once

#ifndef ABIR_READER_H
#define ABIR_READER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <type_traits>
#include <QtWidgets>
#include <sys/stat.h>
#include <qprogressdialog.h>
#include <qinputdialog.h>

#include "binary_file_reader.h"
#include "binary_file_reader.cpp"


struct Object {

	uint32_t imu_count;
	std::vector <std::vector<float>> imu_angle, imu_vel;

    double alpha, beta, alpha_dot, beta_dot, frame_time, imc_az, imc_el;
	std::vector<double> ars_ypr, p_lla, p_ypr, p_vel, fpa_ypr;
		
	//For versions >= 4.1 ...
	std::vector<double> p_ypr_dot;

	//For versions >= 4.2 ...
	std::vector<double> imu_sum;

};


struct ABIR_Header
{
	//Base variables for all versions >= 1.0 ...

    uint16_t                bits_per_pixel;
    uint16_t                image_x_size, image_y_size;
    uint16_t                pixel_depth;
    uint16_t                sensor_id;

    uint32_t                frame_number;
    uint32_t                image_origin;
    uint32_t                sensor_fov;

    uint64_t                image_size, image_size_double;
    uint64_t                size;

    std::vector<uint32_t>   guid, guid_source;
	
    double alpha, beta, alpha_dot, beta_dot, frame_time, seconds;

	//For version >= 2 ...
	uint16_t focus;

	//For versions >= 2.1 ...
	uint32_t msg_type, intended_fov;
	float msg_version;

	//For versions >= 3.0 ...
	std::vector<double> p_lla;
	std::vector<double> p_vel_xyz;

	// For versions = 3.0 ...
	double p_heading, p_pitch, p_roll, p_alt_gps, p_heading_mag;
	uint32_t p_heading_ref;

	//For versions >= 3.1 ...
	float int_time;
	std::vector<double> p_ypr;
	
	//For versions >= 4.1 ...
	std::vector<double> p_ypr_dot;
	float temp_k, pressure, relative_humidity;

	//For version < 4 ...
	// double ars_ypr;

	//For versions >= 5.0 ...
	double g_roll, g_roll_rate;

	//For versions >= 4.0 ...
	float fpa_gain;
	Object one, two;

};


struct ABIR_Frame
{
	ABIR_Header header;
};

struct ABIRDataResult
{
    bool    had_error;
    int     last_valid_frame;
    int     max_value;
    int     x_pixels, y_pixels;

    std::vector<std::vector<uint16_t>> video_frames_16bit;
};

class ABIRData : public BinaryFileReader
{
        Q_OBJECT
    public:
        ABIRData();
        ~ABIRData();

        const char* full_file_path;
        double file_version;
        std::vector<ABIR_Frame> ir_data;

        ABIRDataResult* GetFrames(const char* file_path, unsigned int min_frame, unsigned int max_frame, double version_number = -0.1, bool header_only = false);


    signals:
        void advanceFrame(int);

    private:
        int FileSetup(const char* file_path, double version_number = -0.1);
        double GetVersionNumberFromFile();
};


#endif
