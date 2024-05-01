#pragma once
#ifndef BACKGROUND_SUBTRACTION_H
#define BACKGROUND_SUBTRACTION_H

#include "video_details.h"

#include <armadillo>
#include <qstring.h>
#include <QProgressDialog>
#include "abir_reader.h"
#include "process_file.h"

namespace AdaptiveNoiseSuppression
{
	std::vector<std::vector<double>> get_correction(int start_frame, int number_of_frames_input, video_details & original, QProgressDialog & progress);
	std::vector<std::vector<uint16_t>> process_frames_fast(int start_frame, int number_of_frames_input, video_details & original, QString & hide_shadow_choice, QProgressDialog & progress);
	std::vector<std::vector<uint16_t>> process_frames_conserve_memory(int start_frame, int number_of_frames_input, video_details & original, QString & hide_shadow_choice, QProgressDialog & progress);
};

namespace FixedNoiseSuppression
{
	std::vector<std::vector<double>> get_correction(int start_frame, int number_of_frames_input, video_details & original, QProgressDialog & progress);
};

namespace ApplyCorrection
{
	std::vector<uint16_t> apply_correction(std::vector<uint16_t> frame, std::vector<double> correction, QString & hide_shadow_choice);
};

class FixedNoiseSuppressionExternal
{

public:

	//FixedNoiseSuppressionExternal();
	//~FixedNoiseSuppressionExternal();
	std::vector<std::vector<double>> get_correction(QString path_video_file, int start_frame, int end_frame, int number_of_frames, double version);

private:
	ABIR_Data abir_data;
	//Process_File file_processor;
};

#endif