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
#include <QWidget>
#include <QtWidgets>
#include <QtCore>
class AdaptiveNoiseSuppression : public QObject
{
	Q_OBJECT
private:
	arma::mat kernel;
	std::ofstream outfile;
	void remove_shadow(int nRows, int nCols, arma::vec & frame_vector, arma::mat window_data, int NThresh, int num_of_averaging_frames, int i);	

signals:
     void SignalProgress(unsigned int frameval);

public:
	AdaptiveNoiseSuppression();
	~AdaptiveNoiseSuppression();
	void UpdateProgressBar(unsigned int value);
	std::vector<std::vector<uint16_t>> ProcessFramesConserveMemory(int start_frame, int num_of_averaging_frames_input, int NThresh, VideoDetails & original, QString & hide_shadow_choice);
};



class FixedNoiseSuppression: public QObject
{
	Q_OBJECT
private:

	ABIRData abir_data;

signals:
     void SignalProgress(unsigned int frameval);

public:
	void UpdateProgressBar(unsigned int value);
    std::vector<std::vector<uint16_t>> ProcessFrames(QString image_path, QString path_video_file, int start_frame, int end_frame, double version, VideoDetails & original);
};

#endif
