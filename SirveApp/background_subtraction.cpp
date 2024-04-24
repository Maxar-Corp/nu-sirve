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
	arma:: vec limVal = arma::zeros(num_video_frames);
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
	arma::mat frame_data(num_pixels, 1);

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

	for (int i = index_first_frame; i <= index_last_frame; i++)
	{
		std::vector<double> frame_values(original.frames_16bit[i].begin(), original.frames_16bit[i].end());
		arma::vec frame_vector(frame_values);

		frame_data.insert_cols(0, frame_vector);
	}
	frame_data.shed_col(frame_data.n_cols - 1);

	arma:: vec limVal = arma::zeros(num_video_frames);
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

	std::vector<std::vector<uint16_t>> video_frames_16bit = abir_result.video_frames_16bit;
	//Initialize video frame storage
	int number_frames = video_frames_16bit.size();
	out.reserve(number_of_frames);
	int num_pixels = abir_result.video_frames_16bit[0].size();
	arma::mat frame_data(num_pixels, 1);

	for (int i = 0; i < number_frames; i++)
	{
		std::vector<double> frame_values(video_frames_16bit[i].begin(), video_frames_16bit[i].end());
		arma::vec frame_vector(frame_values);
		frame_data.insert_cols(0, frame_vector);
	}
	frame_data.shed_col(frame_data.n_cols - 1);

	arma:: vec limVal = arma::zeros(number_frames);
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