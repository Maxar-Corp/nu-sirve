#pragma once
#ifndef NOISE_SUPPRESSION_H
#define NOISE_SUPPRESSION_H

#include <armadillo>
#include <qstring.h>
#include <QProgressDialog>
#include <fstream>
#include "abir_reader.h"
#include "process_file.h"
#include "video_details.h"

class AdaptiveNoiseSuppression
{
private:
	arma::mat kernel;
	std::ofstream outfile;
	void remove_shadow(int nRows, int nCols, arma::vec & frame_vector, arma::mat window_data, arma::vec moving_mean, int NThresh, int i);	

public:
	AdaptiveNoiseSuppression();
	~AdaptiveNoiseSuppression();

	std::vector<std::vector<uint16_t>> ProcessFramesConserveMemory(int start_frame, int number_of_frames_input, int NThresh, VideoDetails & original, QString & hide_shadow_choice, QProgressDialog & progress);
};



class FixedNoiseSuppression
{
private:
	ABIRData abir_data;

public:

    std::vector<std::vector<uint16_t>> ProcessFrames(QString image_path, QString path_video_file, int start_frame, int end_frame, double version, VideoDetails & original, QProgressDialog & progress);
};

#endif
