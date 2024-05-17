#pragma once
#ifndef NUC_H
#define NUC_H

#include "abir_reader.h"

#include <armadillo>
#include <qstring.h>

class NUC
{
public:
	std::vector<double> nuc_correction;

	NUC();
	~NUC();

	std::vector<double> CalculateNucCorrection(QString path_video_file, unsigned int min_frame, unsigned int max_frame, double version);
    std::vector<uint16_t> ApplyNucCorrection(std::vector<uint16_t> frame);
	

private:
	int x_pixels, y_pixels;
	ABIRData abir_data;
	arma::uvec pixels_dead, pixels_happy;
	arma::vec adj_mean_frame;
	arma::mat kernel;

    arma::vec ApplyKernel(arma::vec data, arma::uvec indices);
    arma::mat ApplyFilterMatrixWithOrderReduction(arma::mat input_matrix, int order, arma::mat domain);
    double ApplyFilterMatrixWithOrderReduction(arma::mat input_matrix, int order, arma::mat domain, int i, int j);

    arma::vec ReplaceBrokenPixels(arma::vec values);
    void ReplacePixels(arma::vec &base, arma::vec &updated, arma::uvec pixels);
    void ReplaceImagePixels(arma::vec &frame, arma::uvec &indices, arma::vec &update);
};


#endif
