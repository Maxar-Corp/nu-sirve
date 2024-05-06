#include "background_subtraction.h"
#include <vector>
#include <random>

std::vector<std::vector<uint16_t>> FixedNoiseSuppression::process_frames(QString image_path, QString path_video_file, int start_frame, int end_frame, double version, video_details & original, QProgressDialog & progress)
{
	// Initialize output
	std::vector<std::vector<uint16_t>> frames_out;

	int num_video_frames = original.frames_16bit.size();
	int number_avg_frames;
	int  index_first_frame, index_last_frame;
	index_first_frame = start_frame - 1;

	ABIR_Data_Result abir_result;
	int compare = QString::compare(path_video_file, image_path, Qt::CaseInsensitive);
		if (compare!=0){ 
			QByteArray array = image_path.toLocal8Bit();
			char* buffer = array.data();
			abir_result = abir_data.Get_Frames(buffer, start_frame, end_frame, version, false);
			if (abir_result.had_error) {
				return frames_out;
			}
		 number_avg_frames = abir_result.video_frames_16bit.size();
		 progress.setWindowTitle("External Fixed Noise Suppression");
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

	double tmpMax = window_data.max();
	// Take the mean of each row
	arma::vec mean_frame = arma::mean(window_data, 1);
	
	arma::vec frame_vector(num_pixels, 1);
	//Loop through frames to subtract mean
	for (int i = 0; i < num_video_frames; i++){
		if (progress.wasCanceled()){
			return std::vector<std::vector<uint16_t>>();
		}
		progress.setValue(i);
		frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
		frame_vector = frame_vector - mean_frame;
		//Renormalize to 0-16383
		frame_vector = frame_vector - arma::min(frame_vector);
		frame_vector = 16383 * frame_vector / frame_vector.max();
		frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector));
    }
	return frames_out;
}

std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppression::process_frames_fast(int start_frame, int number_of_frames, video_details & original,  QString & hide_shadow_choice, QProgressDialog & progress)
{
	int num_video_frames = original.frames_16bit.size();
	
	int num_pixels = original.frames_16bit[0].size();
	 // Create an Armadillo matrix
    arma::mat frame_data(num_pixels,num_video_frames);

    // Fill the Armadillo matrix from the std::vector
    for (int i = 0; i < num_video_frames; i++) {
		progress.setValue(i);
        frame_data.col(i) = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
    }
    
	int index_last_frame;

	arma::mat moving_mean(num_pixels, num_video_frames);

	for (int i = 0; i < num_video_frames; i++)
	{
		if (progress.wasCanceled())
		{
			return std::vector<std::vector<uint16_t>>();
		}

		progress.setValue(i + num_video_frames);
        index_last_frame = std::min(i + number_of_frames - 1,num_video_frames - 1);  
        moving_mean.col(i) = arma::mean(frame_data.cols(i,index_last_frame), 1);
    }

	moving_mean = arma::shift(moving_mean,-start_frame,1);
	frame_data = frame_data - moving_mean;
	moving_mean.clear();
	arma::vec frame_vector(num_pixels,1) ;
	arma::rowvec min_values = arma::min(frame_data,0);
	std::vector<std::vector<uint16_t>> frames_out;
	arma::uvec index_negative;
	arma::uvec index_positive;
	if (hide_shadow_choice == "Hide Shadow"){
		for (int i = 0; i < num_video_frames; i++){
			progress.setValue(i + 2*num_video_frames);
			index_negative = arma::find(frame_data.col(i) < 0);
			index_positive = arma::find(frame_data.col(i) > 0);
			frame_vector = frame_data.col(i) - min_values(i)*arma::ones(num_pixels,1);
			NoiseSuppressionGeneral::remove_shadow(frame_vector, index_negative, index_positive);
			frame_vector = 16383 * frame_vector / frame_vector.max();
			frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector));
			}
		}
	else
	{
		frame_data = frame_data - arma::repmat(min_values,num_pixels,1);
		for (int i = 0; i < num_video_frames; i++){
			progress.setValue(i + 2*num_video_frames);	
			frame_vector = 16383 * frame_data.col(i) / frame_data.col(i).max();
			frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector));
		}
	}
	return frames_out;
}

std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppression::process_frames_conserve_memory(int start_frame, int number_of_frames, video_details & original,  QString & hide_shadow_choice, QProgressDialog & progress)
{
	int num_video_frames = original.frames_16bit.size();
	int num_pixels = original.frames_16bit[0].size();
	int index_first_frame, index_last_frame;

	std::vector<std::vector<uint16_t>> frames_out;

  arma::mat window_data(num_pixels,number_of_frames);
	arma::vec moving_mean(num_pixels, 1);
	arma::vec frame_vector(num_pixels,1);
	double min_value;

	for (int j = 0; j < number_of_frames - 1; j++) { 
		index_first_frame = std::max(start_frame,0);
		window_data.col(j) = arma::conv_to<arma::vec>::from(original.frames_16bit[j+index_first_frame]);
	}

	window_data.insert_cols(0,window_data.col(0));

    for (int i = 0; i < num_video_frames; i++) {
		
		if (progress.wasCanceled())
		{
			return std::vector<std::vector<uint16_t>>();
		}

		progress.setValue(i);
		index_first_frame = std::max(i + start_frame,0);
        index_last_frame = std::min(index_first_frame + number_of_frames - 1,num_video_frames - 1);
		frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
		window_data.insert_cols(window_data.n_cols,arma::conv_to<arma::vec>::from(original.frames_16bit[index_last_frame]));
		window_data.shed_col(0);
		moving_mean = arma::mean(window_data,1);
		frame_vector = frame_vector - moving_mean;
		min_value = arma::min(frame_vector);
		arma::uvec index_negative;
		arma::uvec index_positive;
		if (hide_shadow_choice == "Hide Shadow"){
			index_negative = arma::find(frame_vector < 0);
			index_positive = arma::find(frame_vector > 0);
			frame_vector = frame_vector - min_value;
			NoiseSuppressionGeneral::remove_shadow(frame_vector, index_negative, index_positive);
			frame_vector = 16383 * frame_vector / frame_vector.max();
		}
		else{
			frame_vector = frame_vector + min_value;
			frame_vector = 16383 * frame_vector / frame_vector.max();
		}
		frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector));
    }


	return frames_out;
}

void NoiseSuppressionGeneral::remove_shadow(arma::vec & frame_vector, arma::uvec index_negative, arma::uvec index_positive)
{
	if (index_negative.size() > 0) {
		double m = arma::mean(frame_vector);
		double s = arma::stddev(frame_vector);
		if (index_positive.size() > 0){
			m = arma::mean(frame_vector(index_positive));
			s = arma::stddev(frame_vector(index_positive));
		}
		arma::vec v = arma::randn<arma::vec>(index_negative.size());
		frame_vector.elem(index_negative) = s * v + m;
	}
}