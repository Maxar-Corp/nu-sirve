#pragma once
#ifndef NUC_H
#define NUC_H

#include "abir_reader.h"
#include "process_file.h"
#include "logging.h"

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

	std::vector<double> get_nuc_correction();
	std::vector<uint16_t> apply_nuc_correction(std::vector<uint16_t> frame, std::vector<double>nuc);
	std::vector<uint16_t> apply_nuc_correction(std::vector<uint16_t> frame);
	arma::mat ordfilt2(arma::mat input_matrix, int order, arma::mat domain);
	

private:

	std::vector<unsigned int> frame_numbers;
	int x_pixels, y_pixels;
	double file_version;
	ABIR_Data abir_data;
	arma::uvec pixels_dead, pixels_happy;
	arma::vec adj_mean_frame;

	std::vector<std::vector<uint16_t>> import_frames();
	arma::mat replace_broken_pixels(arma::vec values);
	void replace_pixels(arma::vec &base, arma::vec &updated, arma::uvec pixels);
	void replace_image_pixels(arma::vec &frame, arma::vec &update, arma::uvec &indices);

};


#endif