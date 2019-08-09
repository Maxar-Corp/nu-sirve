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

	values = values / number_frames;

	arma::mat adjusted_mean_frames = replace_dead_pixels(values);

	double min_value = adjusted_mean_frames.min();

	adjusted_mean_frames = adjusted_mean_frames / min_value;

	arma::vec adjusted_mean_flat = arma::vectorise(adjusted_mean_frames);

	out = arma::conv_to<std::vector<double>>::from(adjusted_mean_flat);

	nuc_correction = out;

	DEBUG << "NUC: Normalizing value for adjusted mean frame: " << min_value;
	DEBUG << "NUC: First value of the normalized adjusted mean frame: " << adjusted_mean_flat(0);

	adjusted_mean_frames.save("adj_mean_frames.txt", arma::arma_ascii);

	return out;

}

std::vector<uint16_t> NUC::apply_nuc_correction(std::vector<uint16_t> frame)
{

	std::vector<double> converted_values(frame.begin(), frame.end());

	arma::vec original_frame(converted_values);
	arma::vec nuc_values(nuc_correction);

	arma::vec corrected_values = original_frame / nuc_values;

	DEBUG << "NUC: NUC correction applied. First value after correction " << corrected_values(0);

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

	DEBUG << "NUC: NUC correction applied. First value after correction " << corrected_values(0);
	
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

arma::mat NUC::replace_dead_pixels(arma::vec values)
{
	double eps = std::pow(10, -6);

	double min_value_vector = values.min() + 0.000001;
	double max_value_vector = values.max() - 0.000001;

	DEBUG << "NUC: Fixing pixels. Min value found is " << min_value_vector;
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

	DEBUG << "NUC: Fixing pixels. Number of dead pixels: " << pixels_dead.n_elem;
	DEBUG << "NUC: Fixing pixels. Number of happy pixels " << pixels_happy.n_elem;

	DEBUG << "NUC: Mean frame value 2nd row, 1st col " << mean_frame(1, 0);
	
	int pixel_index, pixel_row, pixel_col;
	
	pixel_index = pixels_dead(0);
	pixel_row = pixel_index % y_pixels;
	pixel_col = pixel_index / y_pixels;

	arma::mat adj_mean_frame = arma::conv2(mean_frame, kernel);

	DEBUG << "NUC: Adjusted mean frame value 2nd row, 1st col " << adj_mean_frame(1, 0);

	DEBUG << "NUC: Replacing dead pixels";
	replace_pixels(mean_frame, adj_mean_frame, pixels_dead);

	DEBUG << "NUC: Replacing happy pixels";
	replace_pixels(mean_frame, adj_mean_frame, pixels_happy);

	mean_frame.save("mean_frame.txt", arma::arma_ascii);
	
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

		DEBUG << "NUC: Replacing pixel at index " << std::to_string(pixel_index);

		value_before = base(pixel_row, pixel_col);
		value_new = updated(pixel_row + 2, pixel_col + 2);
		
		base(pixel_row, pixel_col) = value_new;
	}

}