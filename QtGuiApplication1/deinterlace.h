#pragma once
#ifndef DEINTERLACE_H
#define DEINTERLACE_H

#include <vector>
#include <armadillo>
#include <fftw3.h>

enum deinterlace_type { max_absolute_value, centroid, avg_cross_correlation };

class Deinterlace
{
public:

	deinterlace_type deinterlace_method;
	int x_pixels, y_pixels, number_pixels;

	Deinterlace(deinterlace_type input_type, int x_pixel_input, int y_pixel_input);
	
	std::vector<uint16_t> deinterlace_frame(std::vector<uint16_t> &frame);

	arma::mat create_frame(std::vector<uint16_t>& frame);
	void create_even_odd_frames(arma::mat & mat_frame, arma::mat &odd_frame, arma::mat &even_frame);
	arma::mat cross_correlate_frame(arma::mat & mat_frame, arma::mat odd_frames, arma::mat even_frames);

	arma::mat fast_fourier_transform(arma::mat matrix1, arma::mat matrix2);

	void test_conversion(std::vector<uint16_t>& frame);

	~Deinterlace();

private:

};


#endif
