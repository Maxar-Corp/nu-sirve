#pragma once
#ifndef NUC_H
#define NUC_H

#include "abir_reader.h"
#include "process_file.h"

#include <armadillo>
#include <qstring.h>

class NUC
{
public:
	QString input_video_file; 
	char* buffer;
	std::vector<double> nuc_correction;

	NUC(QString path_video_file, unsigned int first_frame, unsigned int last_frame, double version);
	~NUC();

	std::vector<double> get_nuc_correction(int max_used_bits);
	std::vector<uint16_t> apply_nuc_correction(std::vector<uint16_t> frame, std::vector<double>nuc);
	std::vector<uint16_t> apply_nuc_correction(std::vector<uint16_t> frame);

private:

	std::vector<unsigned int> frame_numbers;
	int x_pixels, y_pixels;
	double file_version;
	ABIR_Data abir_data;

	std::vector<std::vector<uint16_t>> import_frames();
	arma::mat replace_dead_pixels(arma::vec values, int max_used_bits);
	void replace_pixels(arma::mat &base, arma::mat &updated, arma::uvec pixels);

};


#endif