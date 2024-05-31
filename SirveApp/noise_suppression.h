#pragma once
#ifndef NOISE_SUPPRESSION_H
#define NOISE_SUPPRESSION_H

#include <armadillo>
#include <qstring.h>
#include <QProgressDialog>
#include "abir_reader.h"
#include "process_file.h"
#include "video_details.h"

namespace AdaptiveNoiseSuppression
{
	// std::vector<std::vector<uint16_t>> process_frames_fast(int start_frame, int number_of_frames_input, video_details & original, QString & hide_shadow_choice, QProgressDialog & progress);
	std::vector<std::vector<uint16_t>> ProcessFramesConserveMemory(int start_frame, int number_of_frames_input, int NThresh, VideoDetails & original, QString & hide_shadow_choice, QProgressDialog & progress);
};

namespace NoiseSuppressionGeneral
{
	void remove_shadow(arma::vec & frame_vector, arma::mat window_data, arma::vec moving_mean, int NThresh);
};

class FixedNoiseSuppression
{
public:

    std::vector<std::vector<uint16_t>> ProcessFrames(QString image_path, QString path_video_file, int start_frame, int end_frame, double version, VideoDetails & original, QProgressDialog & progress);

private:
	ABIRData abir_data;

};

#endif
