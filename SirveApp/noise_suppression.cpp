#include "noise_suppression.h"
#include <vector>
#include <random>
#include <fstream>
#include <Qt>

AdaptiveNoiseSuppression::AdaptiveNoiseSuppression()
{
	kernel = {
				{0.0, 0.0, 0.0, 0.0012, 0.0050, 0.0063, 0.0050, 0.0012, 0.0, 0.0, 0.0},
    			{0.0, 0.0000, 0.0062, 0.0124, 0.0127, 0.0127, 0.0127, 0.0124, 0.0062, 0.0000, 0.0},
				{0.0, 0.0062, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0062, 0.0},
				{0.0012, 0.0124, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0124, 0.0012},
				{0.0050, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0050},
				{0.0063, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0063},
				{0.0050, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0050},
				{0.0012, 0.0124, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0124, 0.0012},
				{0.0, 0.0062, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0062, 0.0},
				{0.0, 0.0000, 0.0062, 0.0124, 0.0127, 0.0127, 0.0127, 0.0124, 0.0062, 0.0000, 0.0},
				{0.0, 0.0, 0.0, 0.0012, 0.0050, 0.0063, 0.0050, 0.0012, 0.0, 0.0, 0.0}
			};

}

AdaptiveNoiseSuppression::~AdaptiveNoiseSuppression()
{
}

std::vector<std::vector<uint16_t>> FixedNoiseSuppression::ProcessFrames(QString image_path, QString path_video_file, int start_frame, int end_frame, double version, VideoDetails & original)
{
	// Initialize output
	std::vector<std::vector<uint16_t>> frames_out;

	int num_video_frames = original.frames_16bit.size();
	int number_avg_frames;
	int index_first_frame, index_last_frame;
	index_first_frame = start_frame - 1;

	ABIRDataResult abir_result;
	int compare = QString::compare(path_video_file, image_path, Qt::CaseInsensitive);
	if (compare!=0){ 
			QByteArray array = image_path.toLocal8Bit();
			char* buffer = array.data();
			abir_result = abir_data.GetFrames(buffer, start_frame, end_frame, version, false);
			if (abir_result.had_error) {
				return frames_out;
			}
		 number_avg_frames = abir_result.video_frames_16bit.size();
	}
	else{
		abir_result.video_frames_16bit = original.frames_16bit;
		number_avg_frames = end_frame - start_frame + 1;
	}
	index_last_frame = index_first_frame + number_avg_frames - 1;
	int num_pixels = abir_result.video_frames_16bit[0].size();
	// Create an Armadillo matrix for submatrix average
    arma::mat window_data(num_pixels, number_avg_frames);

    // Fill the Armadillo matrix from the std::vector
	if (compare!=0){ 
		for (int i = 0; i < number_avg_frames; i++){
			window_data.col(i) = arma::conv_to<arma::vec>::from(abir_result.video_frames_16bit[i]);
		}
	}
	else{
		int k = 0;
		for (int i = index_first_frame; i < index_last_frame; i++){
			window_data.col(k) = arma::conv_to<arma::vec>::from(abir_result.video_frames_16bit[i]);
			k += 1;
		}
    }
	// Take the mean of each row
	arma::vec mean_frame = arma::mean(window_data, 1);
	double M;
	arma::vec frame_vector(num_pixels, 1);
	//Loop through frames to subtract mean
	for (int i = 0; i < num_video_frames; i++){
		UpdateProgressBar(i);
		QCoreApplication::processEvents();
		frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
		M = frame_vector.max();
		frame_vector -= mean_frame;
		frame_vector -= frame_vector.min();
		frame_vector = M * frame_vector / frame_vector.max();
		frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector));
    }
	return frames_out;
}


std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppression::ProcessFramesConserveMemory(int start_frame, int num_of_averaging_frames, int NThresh, VideoDetails & original,  QString & hide_shadow_choice)
{

	int num_video_frames = original.frames_16bit.size();
	int num_pixels = original.frames_16bit[0].size();
	int nRows = original.y_pixels;
    int nCols = original.x_pixels;
	int index_first_frame, index_last_frame, index_frame, abs_start_frame;
	double M;
	abs_start_frame = std::abs(start_frame);
	std::vector<std::vector<uint16_t>> frames_out;
  	arma::mat window_data(num_pixels,num_of_averaging_frames);
	arma::mat adjusted_window_data(num_pixels,1);
	adjusted_window_data.fill(0.0);
	arma::vec moving_mean(num_pixels, 1);
	arma::vec frame_vector(num_pixels,1);
	arma::vec frame_vector_out(num_pixels,1);

	for (int j = 0; j < num_of_averaging_frames - 1; j++) { 
		index_frame = std::max(start_frame + j,0);
		window_data.col(j) = arma::conv_to<arma::vec>::from(original.frames_16bit[index_frame]);
	}
    for (int i = 0; i < num_video_frames; i++) {
	 	UpdateProgressBar(i);
		QCoreApplication::processEvents();
		frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);

		index_first_frame = std::max(i + start_frame,0);
        index_last_frame = std::min(index_first_frame + num_of_averaging_frames,num_video_frames - 1);
		window_data.insert_cols(window_data.n_cols,arma::conv_to<arma::vec>::from(original.frames_16bit[index_last_frame]));
		window_data.shed_col(0);

		moving_mean = arma::mean(window_data,1);

		M = frame_vector.max();
		frame_vector -= moving_mean;

		if (hide_shadow_choice == "Hide Shadow"){
			AdaptiveNoiseSuppression::remove_shadow(nRows, nCols, frame_vector, adjusted_window_data, NThresh, num_of_averaging_frames, i);
			if(i == 0){
				adjusted_window_data.insert_cols(0,frame_vector);
				adjusted_window_data.shed_col(0);
			}
			else{
				adjusted_window_data.insert_cols(adjusted_window_data.n_cols,frame_vector);
				if(adjusted_window_data.n_cols>abs_start_frame){
					adjusted_window_data.shed_col(0);	
				}		
			}
		}
		else
		{
			frame_vector -= frame_vector.min();
		}

		frame_vector_out = M * frame_vector / frame_vector.max();
		frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector_out));
    }
	return frames_out;
}

void AdaptiveNoiseSuppression::remove_shadow(int nRows, int nCols, arma::vec & frame_vector, arma::mat window_data, int NThresh, int num_of_averaging_frames, int i)
{	
	arma::mat frame_matrix = arma::reshape(frame_vector,nCols,nRows).t();
	arma::mat frame_matrix_blurred = arma::conv2(frame_matrix,kernel,"same");
	frame_matrix_blurred -= arma::mean(frame_matrix_blurred.as_col());
	arma::vec frame_vector_blurred = frame_matrix_blurred.t().as_col();
	arma::uvec index_negative = arma::find(frame_vector_blurred < -NThresh*arma::stddev(frame_vector_blurred));
	arma::uvec index_change;
	arma::vec old_frame_sum;
	int numCols = window_data.n_cols;
	int maxNcols = std::min(num_of_averaging_frames-1,numCols-1);
	double MEAN, SIGMA;
	if(index_negative.n_elem>0){
		if(maxNcols>0){
			old_frame_sum = arma::sum(window_data.cols(0,maxNcols),1);
		}
		else{
			old_frame_sum = window_data.col(0);
		}
		arma::mat old_frame_sum_mat = arma::reshape(old_frame_sum,nCols,nRows).t();
		arma::mat old_frame_sum_mat_blurred = arma::conv2(old_frame_sum_mat,kernel,"same");
		old_frame_sum_mat_blurred -= arma::mean(old_frame_sum_mat_blurred.as_col());
		arma::vec old_frame_sum_vec_blurred = old_frame_sum_mat_blurred.t().as_col();
		arma::uvec index_positive = arma::find(old_frame_sum_vec_blurred > NThresh*arma::stddev(old_frame_sum_vec_blurred));
		index_change = arma::intersect(index_negative,index_positive);
		if(index_change.n_elem>0){
			arma::uvec index_other = arma::find(arma::abs(frame_vector) <= 3.*arma::stddev(frame_vector));
			if(index_other.n_elem>0){
				MEAN = arma::mean(frame_vector.elem(index_other));
				SIGMA = arma::stddev(frame_vector.elem(index_other));
			}
			else{
				MEAN = arma::mean(frame_vector);
				SIGMA = arma::stddev(frame_vector);
			}
			if (SIGMA!=0){	
				frame_vector.elem(index_change) = arma::randn<arma::vec>(index_change.size(),arma::distr_param(MEAN,SIGMA));
			}
		}
	}

	frame_vector -= frame_vector.min();
}

void AdaptiveNoiseSuppression::UpdateProgressBar(unsigned int val) {

    emit SignalProgress(val);
}

void FixedNoiseSuppression::UpdateProgressBar(unsigned int val) {

    emit SignalProgress(val);
}
