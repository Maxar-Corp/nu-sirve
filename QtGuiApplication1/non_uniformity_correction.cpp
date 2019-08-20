#include "non_uniformity_correction.h"

NUC::NUC(QString path_video_file, unsigned int first_frame, unsigned int last_frame, double version)
{

	file_version = version;
	input_video_file = path_video_file;

	frame_numbers = { first_frame, last_frame };

}

NUC::~NUC()
{
}

std::vector<double> NUC::get_nuc_correction()
{
	INFO << "NUC: Starting correction process";
	std::vector<double>out;

	QByteArray array = input_video_file.toLocal8Bit();
	buffer = array.data();
		
	int check_value = abir_data.File_Setup(buffer, file_version);
	if (check_value < 0) {

		return out;
	}
	std::vector<std::vector<uint16_t>> video_frames_16bit = import_frames();

	int number_frames = video_frames_16bit.size();
	int number_pixels = video_frames_16bit[0].size();

	arma::vec values(number_pixels);
	values.fill(0.0);

	for (int i = 0; i < number_frames; i++)
	{
		std::vector<double> converted_values(video_frames_16bit[i].begin(), video_frames_16bit[i].end());
		arma::vec temp(converted_values);
		values = values + temp;
	}

	DEBUG << "NUC: Summation of all values in 1st row, 1st column: " << std::to_string(values(0));

	values = values / number_frames;

	DEBUG << "NUC: Summation of all values, normalized by number of frames, in 1st row, 1st column: " << std::to_string(values(0));

	arma::mat adjusted_mean_frames = replace_broken_pixels(values);

	double min_value = adjusted_mean_frames.min();

	adjusted_mean_frames = adjusted_mean_frames / min_value;
	arma::mat adjusted_mean_transpose = adjusted_mean_frames.t();

	arma::vec adjusted_mean_flat = arma::vectorise(adjusted_mean_transpose);

	out = arma::conv_to<std::vector<double>>::from(adjusted_mean_flat);

	nuc_correction = out;

	DEBUG << "NUC: Normalizing value for kernel adjusted mean frame: " << std::to_string(min_value);
	DEBUG << "NUC: First value of the NUC correction: " << std::to_string(adjusted_mean_flat(0));

	adjusted_mean_frames.save("nuc_correction_matrix.txt", arma::arma_ascii);

	return out;

}

std::vector<uint16_t> NUC::apply_nuc_correction(std::vector<uint16_t> frame)
{

	std::vector<double> converted_values(frame.begin(), frame.end());

	arma::vec original_frame(converted_values);
	arma::vec nuc_values(nuc_correction);

	arma::vec corrected_values = original_frame / nuc_values;

	DEBUG << "NUC: NUC correction applied. First value after correction " << std::to_string(corrected_values(0));

	std::vector<double> vector_double = arma::conv_to<std::vector<double>>::from(corrected_values);
	std::vector<uint16_t> vector_int(vector_double.begin(), vector_double.end());

	return vector_int;
}

std::vector<uint16_t> NUC::apply_nuc_correction(std::vector<uint16_t> frame, std::vector<double> nuc)
{
	
	std::vector<double> converted_values(frame.begin(), frame.end());
	
	arma::vec original_frame(converted_values);
	arma::vec nuc_values(nuc);

	arma::vec corrected_values = original_frame / nuc_values;

	DEBUG << "NUC: NUC correction applied. First value after correction " << std::to_string(corrected_values(0));
	
	std::vector<double> vector_double = arma::conv_to<std::vector<double>>::from(corrected_values);
	std::vector<uint16_t> vector_int (vector_double.begin(), vector_double.end());
	
	return vector_int;
}

std::vector<std::vector<uint16_t>> NUC::import_frames() {

	std::vector<std::vector<uint16_t>> video_frames_16bit;

	if (frame_numbers[0] < 0 || frame_numbers[1] < 0)
		return video_frames_16bit;

	video_frames_16bit = abir_data.Get_Data_and_Frames(frame_numbers, false);

	DEBUG << "NUC: Retrieved video frames from ABIR file";

	x_pixels = abir_data.ir_data[0].header.image_x_size;
	y_pixels = abir_data.ir_data[0].header.image_y_size;

	return video_frames_16bit;
}

arma::mat NUC::replace_broken_pixels(arma::vec values)
{
	double eps = std::pow(10, -6);

	arma::vec sorted_values = arma::sort(values);

	int index_min = std::ceil(0.00001 * sorted_values.n_elem);
	int index_max = std::floor(0.99999 * sorted_values.n_elem);

	double min_value_vector = sorted_values(index_min);
	double max_value_vector = sorted_values(index_max);

	DEBUG << "NUC: Fixing pixels. Index of min value found is " << index_min;
	DEBUG << "NUC: Fixing pixels. Min value found is " << min_value_vector;
	DEBUG << "NUC: Fixing pixels. Index of max value found is " << index_max;
	DEBUG << "NUC: Fixing pixels. Max value found is " << max_value_vector;

	arma::mat kernel = {{0, 0, 1, 0, 0},
						{0, 0, 1, 0, 0},
						{1, 1, 0, 1, 1},
						{0, 0, 1, 0, 0},
						{0, 0, 1, 0, 0} };

	arma::mat mean_frame(values);
	mean_frame.reshape(x_pixels, y_pixels);
	mean_frame = mean_frame.t();

	arma::uvec pixels_dead = arma::find(mean_frame <= min_value_vector);
	arma::uvec pixels_happy = arma::find(mean_frame >= max_value_vector);

	DEBUG << "NUC: Mean frame value 2nd row, 1st col " << std::to_string(mean_frame(1, 0));

	DEBUG << "NUC: Fixing pixels. Number of dead pixels: " << pixels_dead.n_elem;
	DEBUG << "NUC: Fixing pixels. Number of happy pixels " << pixels_happy.n_elem;
		
	int pixel_index, pixel_row, pixel_col;
	
	arma::mat adj_mean_frame = arma::conv2(mean_frame, kernel, "same");
	mean_frame.save("nuc_mean_frame.txt", arma::arma_ascii);
	adj_mean_frame.save("nuc_post_convolution_mean_frame.txt", arma::arma_ascii);

	DEBUG << "NUC: Replacing dead pixels";
	replace_pixels(mean_frame, adj_mean_frame, pixels_dead);

	DEBUG << "NUC: Replacing happy pixels";
	replace_pixels(mean_frame, adj_mean_frame, pixels_happy);

	DEBUG << "NUC: Mean frame with pixels replaced and kernel applied; value at 2nd row, 1st col " << std::to_string(mean_frame(1, 0));
	
	return mean_frame;
}

void NUC::replace_pixels(arma::mat &base, arma::mat &updated, arma::uvec pixels) {

	int num_pixels = pixels.size();
	int rows = base.n_rows;
	int cols = base.n_cols;

	int pixel_index, pixel_row, pixel_col;
	double value_before, value_new;
	
	for (int i = 0; i < num_pixels; i++)
	{
		
		pixel_index = pixels(i);
		pixel_row = pixel_index % rows;
		pixel_col = pixel_index / rows;

		value_before = base(pixel_row, pixel_col);
		value_new = updated(pixel_row, pixel_col);

		DEBUG << "NUC: Replacing pixel at (row, col) " << pixel_row << ", " << pixel_col;
		DEBUG << "NUC: Original value was " << value_before;
		DEBUG << "NUC: New value is " << value_new;

		base(pixel_row, pixel_col) = value_new;
	}

}