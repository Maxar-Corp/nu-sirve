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
	std::vector<std::vector<uint16_t>> process_frames_fast(int start_frame, int number_of_frames_input, video_details & original, QString & hide_shadow_choice, QProgressDialog & progress);
	std::vector<std::vector<uint16_t>> process_frames_conserve_memory(int start_frame, int number_of_frames_input, video_details & original, QString & hide_shadow_choice, QProgressDialog & progress);
};

// namespace FixedNoiseSuppression
// {
// 	std::vector<std::vector<uint16_t>> process_frames(int start_frame, int number_of_frames_input, video_details & original, QProgressDialog & progress);
// };

namespace NoiseSuppressionGeneral
{
	void remove_shadow(arma::vec & frame_vector, arma::uvec index_negative, arma::uvec index_positive);
};

class FixedNoiseSuppression
{
public:

	std::vector<std::vector<uint16_t>> process_frames(AbpFileMetadata abp_file_metadata, QString path_video_file, int start_frame, int end_frame, double version, video_details & original, QProgressDialog & progress);

private:
	ABIR_Data abir_data;
};

#endif