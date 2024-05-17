#pragma once
#ifndef DEINTERLACE_H
#define DEINTERLACE_H

#include <vector>
#include <armadillo>
#include <fftw3.h>

#include "deinterlace_type.h"

class Deinterlace
{
public:

    DeinterlaceType deinterlace_method;
	int x_pixels, y_pixels, number_pixels;

    Deinterlace(DeinterlaceType input_type, int x_pixel_input, int y_pixel_input);
	
	std::vector<uint16_t> DeinterlaceFrame(std::vector<uint16_t> &frame);
    static std::vector<uint16_t> DeinterlaceFrame(DeinterlaceType input_type, int x_pixel_input, int y_pixel_input, std::vector<uint16_t> &frame);

	arma::mat CreateFrame(std::vector<uint16_t>& frame);
	static arma::mat CreateFrame(int x_pixel_input, int y_pixel_input, std::vector<uint16_t>& frame);

	void CreateEvenOddFrames(arma::mat & mat_frame, arma::mat &odd_frame, arma::mat &even_frame);
	static void CreateEvenOddFrames(arma::mat & mat_frame, arma::mat &odd_frame, arma::mat &even_frame, int y_pixel_input);

	static arma::mat CrossCorrelateFrame(arma::mat & mat_frame, arma::mat odd_frames, arma::mat even_frames);

	static arma::mat CalculateFastFourierTransformMatrix(arma::mat matrix1, arma::mat matrix2);
	static void CalculateMeshGrid(arma::vec x_input, arma::vec y_input, arma::mat &x_mat, arma::mat &y_mat);

	void TestConversion(std::vector<uint16_t>& frame);

	~Deinterlace();

private:

};


#endif
