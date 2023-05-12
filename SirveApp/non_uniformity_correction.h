#pragma once
#ifndef NUC_H
#define NUC_H

#include "abir_reader.h"
#include "logging.h"

#include <armadillo>
#include <qstring.h>

class NUC
{
public:
	QString input_video_file; 
	char* buffer;
	std::vector<double> nuc_correction;

	NUC(QString path_video_file, double version);
	~NUC();

	std::vector<double> get_nuc_correction(unsigned int min_frame, unsigned int max_frame);
	std::vector<uint16_t> apply_nuc_correction(std::vector<uint16_t> frame);
	

private:
	int x_pixels, y_pixels;
	double file_version;
	ABIR_Data abir_data;
	arma::uvec pixels_dead, pixels_happy;
	arma::vec adj_mean_frame;
	arma::mat kernel;

	arma::vec apply_kernel(arma::vec data, arma::uvec indices);
	arma::mat ordfilt2(arma::mat input_matrix, int order, arma::mat domain);
	double ordfilt2(arma::mat input_matrix, int order, arma::mat domain, int i, int j);

	std::vector<std::vector<uint16_t>> import_frames(unsigned int min_frame, unsigned int max_frame);
	arma::vec replace_broken_pixels(arma::vec values);
	void replace_pixels(arma::vec &base, arma::vec &updated, arma::uvec pixels);
	void replace_image_pixels(arma::vec &frame, arma::uvec &indices, arma::vec &update);

};


#endif