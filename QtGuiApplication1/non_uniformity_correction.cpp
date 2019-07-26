#include "non_uniformity_correction.h"

NUC::NUC(QString path_video_file, unsigned int first_frame, unsigned int last_frame, double version)
{

	file_version = version;
	input_video_file = path_video_file;

	frame_numbers = { first_frame, last_frame };

	//TODO apply fix to creating frame that is similar to tested de-interlace function
}

NUC::~NUC()
{
}

std::vector<double> NUC::get_nuc_correction(int max_used_bits)
{
	
	QByteArray array = input_video_file.toLocal8Bit();
	buffer = array.data();
		
	abir_data.File_Setup(buffer, file_version);
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

	values = values / number_frames;

	arma::mat adjusted_mean_frames = replace_dead_pixels(values, max_used_bits);

	double min_value = adjusted_mean_frames.min();

	adjusted_mean_frames = adjusted_mean_frames / min_value;

	arma::vec adjusted_mean_flat = arma::vectorise(adjusted_mean_frames);

	std::vector<double>out = arma::conv_to<std::vector<double>>::from(adjusted_mean_flat);

	nuc_correction = out;

	return out;

}

std::vector<uint16_t> NUC::apply_nuc_correction(std::vector<uint16_t> frame)
{

	std::vector<double> converted_values(frame.begin(), frame.end());

	arma::vec original_frame(converted_values);
	arma::vec nuc_values(nuc_correction);

	arma::vec corrected_values = original_frame / nuc_values;

	//original_frame.save("original_frame.txt", arma::arma_ascii);
	//nuc_values.save("nuc_values.txt", arma::arma_ascii);
	//corrected_values.save("updated_frame.txt", arma::arma_ascii);

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

	//original_frame.save("original_frame.txt", arma::arma_ascii);
	//nuc_values.save("nuc_values.txt", arma::arma_ascii);
	//corrected_values.save("updated_frame.txt", arma::arma_ascii);
	
	std::vector<double> vector_double = arma::conv_to<std::vector<double>>::from(corrected_values);
	std::vector<uint16_t> vector_int (vector_double.begin(), vector_double.end());
	
	return vector_int;
}

std::vector<std::vector<uint16_t>> NUC::import_frames() {

	std::vector<std::vector<uint16_t>> video_frames_16bit;

	//TODO check that the frame numbers are within upper bounds
	if (frame_numbers[0] < 0 || frame_numbers[1] < 0)
		return video_frames_16bit;

	video_frames_16bit = abir_data.Get_Data_and_Frames(frame_numbers, false);

	x_pixels = abir_data.ir_data[0].header.image_x_size;
	y_pixels = abir_data.ir_data[0].header.image_y_size;

	return video_frames_16bit;
}

arma::mat NUC::replace_dead_pixels(arma::vec values, int max_used_bits)
{

	double eps = std::pow(10, -6);

	arma::uvec pixels_dead = arma::find(values < 1);
	arma::uvec pixels_happy = arma::find(values > std::pow(2, max_used_bits) - 1);

	arma::mat kernel = {{0, 0, 1, 0, 0},
						{0, 0, 1, 0, 0},
						{1, 1, 0, 1, 1},
						{0, 0, 1, 0, 0},
						{0, 0, 1, 0, 0} };

	arma::mat mean_frame(values);
	mean_frame.reshape(y_pixels, x_pixels);
	
	int pixel_index, pixel_row, pixel_col;
	
	pixel_index = pixels_dead(0);
	pixel_row = pixel_index % y_pixels;
	pixel_col = pixel_index / y_pixels;

	arma::mat adj_mean_frame = arma::conv2(mean_frame, kernel);

	replace_pixels(mean_frame, adj_mean_frame, pixels_dead);
	replace_pixels(mean_frame, adj_mean_frame, pixels_happy);
	
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
		value_new = updated(pixel_row + 2, pixel_col + 2);
		
		base(pixel_row, pixel_col) = value_new;
	}

}