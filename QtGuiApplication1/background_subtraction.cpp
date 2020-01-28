#include "background_subtraction.h"

BackgroundSubtraction::BackgroundSubtraction(int number_of_frames_input)
{
	number_of_frames = number_of_frames_input;
}

BackgroundSubtraction::~BackgroundSubtraction()
{
}

std::vector<std::vector<double>> BackgroundSubtraction::get_correction(video_details & original)
{
	INFO << "Background Subtraction: Process started";

	// Initialize output
	std::vector<std::vector<double>>out;
	int num_video_frames = original.frames_16bit.size();
	out.reserve(num_video_frames);

	//Initialize video frame storage
	int num_pixels = original.x_pixels * original.y_pixels;
	arma::mat frame_data(num_pixels, 1);

	QProgressDialog progress("", QString(), 0, 1000);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimum(0);
	progress.setMaximum(num_video_frames);
	progress.setWindowTitle(QString("Background Subtraction"));
	progress.setLabelText(QString("Creating adjustment for background subtraction..."));

	progress.setMinimumWidth(300);

	for (int i = 0; i < num_video_frames; i++)
	{
		DEBUG << "Background Subtraction: Processing adjustment for frame #" << i + 1;
		progress.setValue(i);

		std::vector<double> frame_values(original.frames_16bit[i].begin(), original.frames_16bit[i].end());
		arma::vec frame_vector(frame_values);

		if (i == 0)
		{
			frame_data.col(0) = frame_vector;
		}
		else
		{
			int num_cols = frame_data.n_cols;
			frame_data.insert_cols(num_cols, frame_vector);

			if (num_cols >= number_of_frames)
				frame_data.shed_col(0);

		}
		
		DEBUG << "Background Subtraction: Value of first pixel of the last frame used is " << std::to_string(frame_data(0, 0));

		// Take the mean of each row
		arma::vec mean_frame = arma::mean(frame_data, 1);

		//Convert mean to double and store
		std::vector<double> vector_mean = arma::conv_to<std::vector<double>>::from(mean_frame);
		out.push_back(vector_mean);
	}

	INFO << "Background Subtraction: Process completed";

	return out;
}

std::vector<uint16_t> BackgroundSubtraction::apply_correction(std::vector<uint16_t> frame, std::vector<double> correction)
{

	std::vector<double> converted_values(frame.begin(), frame.end());

	arma::vec original_frame(converted_values);
	arma::vec correction_values(correction);

	arma::vec corrected_values = original_frame - correction_values;

	arma::uvec index_negative = arma::find(corrected_values < 0);
	if (index_negative.size() > 0){
		DEBUG << "Background Subtraction: " << index_negative.size() << "number of negative pixels found during subtraction.";
		corrected_values.elem(index_negative) = arma::zeros(index_negative.size());
	}

	std::vector<double> vector_double = arma::conv_to<std::vector<double>>::from(corrected_values);
	std::vector<uint16_t> vector_int(vector_double.begin(), vector_double.end());

	DEBUG << "Background Subtraction: Adjustment  applied";

	return vector_int;

}
