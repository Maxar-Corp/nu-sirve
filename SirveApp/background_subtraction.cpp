#include "background_subtraction.h"
#include <vector>
#include <random>

std::vector<std::vector<double>> AdaptiveNoiseSuppression::get_correction(int start_frame, int number_of_frames, video_details & original, QProgressDialog & progress)
{
	// Initialize output
	std::vector<std::vector<double>>out;
	int num_video_frames = original.frames_16bit.size();
	out.reserve(num_video_frames);

	//Initialize video frame storage
	int num_pixels = original.frames_16bit[0].size();
	arma::mat frame_data(num_pixels, 1);

	// initialize noise frames
	int index_first_frame, index_last_frame;
	if (start_frame < 0) {
		index_first_frame = 0;
		index_last_frame = number_of_frames - 1;
	}
	else {
		index_first_frame = start_frame - (number_of_frames - 1);
		index_last_frame = start_frame;
	}
	
	for (int i = index_last_frame; i >= index_first_frame; i--)
	{
		std::vector<double> frame_values(original.frames_16bit[i].begin(), original.frames_16bit[i].end());
		arma::vec frame_vector(frame_values);

		if (i == index_last_frame)
			frame_data.col(0) = frame_vector;
		else
			frame_data.insert_cols(0, frame_vector);
	}
	
	//iterate through frames to calculate suppression for each individual frame
	for (int i = 0; i < num_video_frames; i++)
	{
		if (progress.wasCanceled())
		{
			return std::vector<std::vector<double>>();
		}

		//arma::vec mean_frame = arma::mean(values, 1);
		progress.setValue(i);

		if (i > 0) {

			index_first_frame = i + start_frame - (number_of_frames - 1);
			index_last_frame = i + start_frame;
			
			if (index_first_frame > 0 && index_last_frame < num_video_frames)
			{
				std::vector<double> frame_values(original.frames_16bit[index_first_frame].begin(), original.frames_16bit[index_first_frame].end());
				arma::vec frame_vector(frame_values);

				int num_cols = frame_data.n_cols;
				frame_data.insert_cols(num_cols, frame_vector);
				frame_data.shed_col(0);
			}
		}

		// Take the mean of each row
		arma::vec mean_frame = arma::mean(frame_data, 1);

		//Convert mean to double and store
		std::vector<double> vector_mean = arma::conv_to<std::vector<double>>::from(mean_frame);

		out.push_back(vector_mean);
	}

	return out;
}

std::vector<std::vector<double>> FixedNoiseSuppression::get_correction(int start_frame, int number_of_frames, video_details & original, QProgressDialog & progress)
{
	// Initialize output
	std::vector<std::vector<double>>out;
	int num_video_frames = original.frames_16bit.size();
	out.reserve(num_video_frames);

	//Initialize video frame storage
	int num_pixels = original.frames_16bit[0].size();
	// arma::mat frame_data(num_pixels, 1);

	// initialize noise frames
	int index_first_frame, index_last_frame;

		index_first_frame = start_frame - 1;
		
		if (index_first_frame < 0)
		{
			index_first_frame = 0;
		}
		
		index_last_frame = start_frame + (number_of_frames - 1);

		if (index_last_frame > num_video_frames - 1)
		{
			index_last_frame = num_video_frames - 1;
		}

	// Create an Armadillo matrix
    arma::mat frame_data(num_pixels, number_of_frames);
	std::vector<int> framesi(number_of_frames);
	std::iota (framesi.begin(), framesi.end(), index_first_frame);
    // Fill the Armadillo matrix from the std::vector
    for (int i = 0; i < number_of_frames; i++) {
        frame_data.col(i) = arma::conv_to<arma::vec>::from(original.frames_16bit[framesi[i]]);
    }

	// Take the mean of each row
	arma::vec mean_frame = arma::mean(frame_data, 1);

	for (int i = 0; i < num_video_frames; i++)
	{
		if (progress.wasCanceled())
		{
			return std::vector<std::vector<double>>();
		}

		//Convert mean to double and store
		std::vector<double> vector_mean = arma::conv_to<std::vector<double>>::from(mean_frame);

		out.push_back(vector_mean);
	}

	return out;
}

std::vector<std::vector<double>> FixedNoiseSuppressionExternal::get_correction(QString path_video_file, int min_frame, int max_frame, int number_of_frames, double version)
{
	// Initialize output
	std::vector<std::vector<double>>out;

	ABIR_Data_Result abir_result = abir_data.Get_Frames(path_video_file.toLocal8Bit().constData(), min_frame, max_frame, version, false);
	
	if (abir_result.had_error) {
		return out;
	}

	int number_avg_frames = abir_result.video_frames_16bit.size();
	int num_pixels = abir_result.video_frames_16bit[0].size();
	out.reserve(number_of_frames);

    arma::mat frame_data(num_pixels, number_avg_frames);

    // Fill the Armadillo matrix from the std::vector
    for (int i = 0; i < number_avg_frames; i++) {
        frame_data.col(i) = arma::conv_to<arma::vec>::from(abir_result.video_frames_16bit[i]);
    }

	// Take the mean of each row
	arma::vec mean_frame = arma::mean(frame_data, 1);
	for (int i = 0; i < number_of_frames; i++)
	{
		//Convert mean to double and store
		std::vector<double> vector_mean = arma::conv_to<std::vector<double>>::from(mean_frame);
		out.push_back(vector_mean);
	}

	return out;
}

std::vector<uint16_t> ApplyCorrection::apply_correction(std::vector<uint16_t> frame, std::vector<double> correction, QString & hide_shadow_choice)
{
	std::vector<double> converted_values(frame.begin(), frame.end());

	arma::vec original_frame(converted_values);
	arma::vec correction_values(correction);

	arma::vec corrected_values = original_frame - correction_values;
	arma::uvec index_negative = arma::find(corrected_values <= 0);
	arma::uvec index_positive = arma::find(corrected_values > 0);
	double min_value = abs(corrected_values.min());
	corrected_values = corrected_values + min_value * arma::ones(corrected_values.size());

	if (hide_shadow_choice == "Hide Shadow"){
		if (index_negative.size() > 0){
			double m = arma::mean(corrected_values);
			double s = arma::stddev(corrected_values);
			if (index_positive.size() > 0){
				m = arma::mean(corrected_values(index_positive));
				s = arma::stddev(corrected_values(index_positive));
			}
			arma::vec v = arma::randn<arma::vec>(index_negative.size());
			corrected_values.elem(index_negative) = s * v + m;
		}
	}

	std::vector<double> vector_double = arma::conv_to<std::vector<double>>::from(corrected_values);
	std::vector<uint16_t> vector_int(vector_double.begin(), vector_double.end());

	return vector_int;
}

std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppression::process_frames(int start_frame, int number_of_frames, video_details & original,  QString & hide_shadow_choice, QProgressDialog & progress)
{
	// Initialize output

	int num_video_frames = original.frames_16bit.size();
	
	//Initialize video frame storage
	int num_pixels = original.frames_16bit[0].size();
	 // Create an Armadillo matrix
    arma::mat frame_data(num_pixels,num_video_frames);
	arma::mat moving_mean(num_pixels, num_video_frames);

    // Fill the Armadillo matrix from the std::vector
    for (int i = 0; i < num_video_frames; i++) {
		progress.setValue(i);
        frame_data.col(i) = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
    }
    
	// initialize noise frames
	int index_first_frame, index_last_frame;

	int half_window_length = std::round(number_of_frames/2);

	for (int i = 0; i < num_video_frames; i++)
	{
		if (progress.wasCanceled())
		{
			return std::vector<std::vector<uint16_t>>();
		}

		progress.setValue(i + num_video_frames);

        index_first_frame = std::max(i - half_window_length + 1,0);
        index_last_frame = std::min(i + half_window_length - 1,num_video_frames - 1);  
        moving_mean.col(i) = arma::mean(frame_data.cols(index_first_frame,index_last_frame), 1);
    }

	moving_mean = arma::shift(moving_mean,-start_frame,1);
	frame_data = frame_data - moving_mean;
	moving_mean.clear();
	arma::rowvec min_values = arma::abs(arma::min(frame_data,0));
	std::vector<std::vector<uint16_t>> frames_out;
	if (hide_shadow_choice == "Hide Shadow"){
		for (int i = 0; i < num_video_frames; i++){
			progress.setValue(i + 2*num_video_frames);
			arma::uvec index_negative = arma::find(frame_data.col(i) < 0);
			arma::uvec index_positive = arma::find(frame_data.col(i) > 0);
			arma::vec processed_frame = frame_data.col(i) + min_values(i)*arma::ones(num_pixels,1);
			if (index_negative.size() > 0){
				double m = arma::mean(processed_frame);
				double s = arma::stddev(processed_frame);
				if (index_positive.size() > 0){
					m = arma::mean(processed_frame(index_positive));
					s = arma::stddev(processed_frame(index_positive));
				}
				arma::vec v = arma::randn<arma::vec>(index_negative.size());
				processed_frame.elem(index_negative) = s * v + m;
			}
			frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(processed_frame));
			}
		}
	else
	{
		frame_data = frame_data + arma::repmat(min_values,num_pixels,1);
		for (int i = 0; i < num_video_frames; i++){
			progress.setValue(i + 2*num_video_frames);	
			frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_data.col(i)));
		}
	}
	return frames_out;
}
