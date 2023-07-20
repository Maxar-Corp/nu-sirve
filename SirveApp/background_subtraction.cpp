#include "background_subtraction.h"

std::vector<std::vector<double>> AdaptiveNoiseSuppression::get_correction(int start_frame, int number_of_frames, video_details & original)
{
	INFO << "Background Subtraction: Process started";

	// Initialize output
	std::vector<std::vector<double>>out;
	int num_video_frames = original.frames_16bit.size();
	out.reserve(num_video_frames);

	//Initialize video frame storage
	int num_pixels = original.number_pixels;
	arma::mat frame_data(num_pixels, 1);

	QProgressDialog progress("Creating adjustment for background subtraction", "Cancel", 0, num_video_frames);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimum(0);
	progress.setMaximum(num_video_frames);
	progress.setWindowTitle(QString("Adaptive Background Suppression"));
	progress.setLabelText(QString("Creating adjustment on per frame basis..."));
	progress.setMinimumWidth(300);
	progress.setMinimumDuration(500);

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
			DEBUG << "Background Subtraction: Adjustment process canceled";
			return std::vector<std::vector<double>>();
		}
	

		
		//arma::vec mean_frame = arma::mean(values, 1);
		DEBUG << "Background Subtraction: Processing adjustment for frame #" << i + 1;
		progress.setValue(i);

		if (i > 0) {

			index_first_frame = i + start_frame - (number_of_frames - 1);
			index_last_frame = i + start_frame;
			
			if (index_first_frame > 0 && index_last_frame < num_video_frames)
			{
				std::vector<double> frame_values(original.frames_16bit[i].begin(), original.frames_16bit[i].end());
				arma::vec frame_vector(frame_values);

				int num_cols = frame_data.n_cols;
				frame_data.insert_cols(num_cols, frame_vector);
				frame_data.shed_col(0);
			}
		}

		DEBUG << "Background Subtraction: Value of first pixel of the last frame used is " << std::to_string(frame_data(0, 0));

		// Take the mean of each row
		arma::vec mean_frame = arma::mean(frame_data, 1);

		//Convert mean to double and store
		std::vector<double> vector_mean = arma::conv_to<std::vector<double>>::from(mean_frame);

		out.push_back(vector_mean);
	}

	INFO << "Background Subtraction: Adjustment process completed";

	return out;
}

std::vector<uint16_t> AdaptiveNoiseSuppression::apply_correction(std::vector<uint16_t> frame, std::vector<double> correction)
{

	std::vector<double> converted_values(frame.begin(), frame.end());

	arma::vec original_frame(converted_values);
	arma::vec correction_values(correction);

	arma::vec corrected_values = original_frame - correction_values;
	double min_value = abs(corrected_values.min());
	corrected_values = corrected_values + min_value * arma::ones(corrected_values.size());

	//arma::uvec index_negative = arma::find(corrected_values < 0);
	//if (index_negative.size() > 0){
	//	DEBUG << "Background Subtraction: " << index_negative.size() << "number of negative pixels found during subtraction.";
	//	corrected_values.elem(index_negative) = arma::zeros(index_negative.size());
	//}

	std::vector<double> vector_double = arma::conv_to<std::vector<double>>::from(corrected_values);
	std::vector<uint16_t> vector_int(vector_double.begin(), vector_double.end());

	DEBUG << "Background Subtraction: Adjustment  applied";

	return vector_int;

}