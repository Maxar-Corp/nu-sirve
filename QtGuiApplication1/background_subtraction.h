#pragma once
#ifndef BACKGROUND_SUBTRACTION_H
#define BACKGROUND_SUBTRACTION_H

#include "video_container.h"
#include "logging.h"

#include <armadillo>
#include <qstring.h>

class BackgroundSubtraction
{
public:
	BackgroundSubtraction(int number_of_frames_input);
	~BackgroundSubtraction();

	int number_of_frames;

	std::vector<std::vector<double>> get_correction(video_details & original);
	std::vector<uint16_t> apply_correction(std::vector<uint16_t> frame, std::vector<double> correction);

private:

};


#endif