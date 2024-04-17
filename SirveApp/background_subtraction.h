#pragma once
#ifndef BACKGROUND_SUBTRACTION_H
#define BACKGROUND_SUBTRACTION_H

#include "video_details.h"

#include <armadillo>
#include <qstring.h>
#include <QProgressDialog>

namespace AdaptiveNoiseSuppression
{
	std::vector<std::vector<double>> get_correction(int start_frame, int number_of_frames_input, video_details & original, QProgressDialog & progress);
};

namespace FixedNoiseSuppression
{
	std::vector<std::vector<double>> get_correction(int start_frame, int number_of_frames_input, video_details & original, QProgressDialog & progress);
};

namespace ApplyCorrection
{
	std::vector<uint16_t> apply_correction(std::vector<uint16_t> frame, std::vector<double> correction, QString & hide_shadow_choice);
};

#endif