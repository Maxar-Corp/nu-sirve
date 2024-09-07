#include "non_uniformity_correction.h"

NonUniformityCorrection::NonUniformityCorrection()
{
	kernel = { {0, 0, 1, 0, 0},
		       {0, 0, 1, 0, 0},
			   {1, 1, 0, 1, 1},
			   {0, 0, 1, 0, 0},
			   {0, 0, 1, 0, 0} };
}

NonUniformityCorrection::~NonUniformityCorrection()
{
}

std::vector<double> NonUniformityCorrection::CalculateNucCorrection(QString path_video_file, unsigned int min_frame, unsigned int max_frame, double version)
{
	std::vector<double>out;

    ABIRDataResult *abir_result = abir_data.GetFrames(path_video_file.toLocal8Bit().constData(), min_frame, max_frame, version, false);
    if (abir_result->had_error) {
		return out;
	}

    std::vector<std::vector<uint16_t>> video_frames_16bit = abir_result->video_frames_16bit;
    x_pixels = abir_result->x_pixels;
    y_pixels = abir_result->y_pixels;

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

	//DEBUG << "NUC: Summation of all values in 1st row, 1st column: " << std::to_string(values(0));

	values = values / number_frames;

	//DEBUG << "NUC: Summation of all values, normalized by number of frames, in 1st row, 1st column: " << std::to_string(values(0));

    arma::vec adjusted_mean_frames = ReplaceBrokenPixels(values);

	double min_value = adjusted_mean_frames.min();

	adjusted_mean_frames = adjusted_mean_frames / min_value;
	
	//arma::mat adjusted_mean_transpose = adjusted_mean_frames.t();
	//arma::vec adjusted_mean_flat = arma::vectorise(adjusted_mean_transpose);

	out = arma::conv_to<std::vector<double>>::from(adjusted_mean_frames);

	nuc_correction = out;

	//DEBUG << "NUC: Normalizing value for kernel adjusted mean frame: " << min_value;
	//DEBUG << "NUC: First value of the NUC correction: " << adjusted_mean_frames(0);

	arma::mat nuc_correction(adjusted_mean_frames);
	nuc_correction.reshape(x_pixels, y_pixels);
	nuc_correction = nuc_correction.t();
	
	return out;

}

std::vector<uint16_t> NonUniformityCorrection::ApplyNucCorrection(std::vector<uint16_t> frame)
{

	std::vector<double> converted_values(frame.begin(), frame.end());

	arma::vec original_frame(converted_values);
	arma::vec nuc_values(nuc_correction);
		
	// ----------------------------------------------------------------------------------------------

	// get updated values for happy and dead pixels
    arma::vec values_happy_pixels = ApplyKernel(original_frame, pixels_happy);
    arma::vec values_dead_pixels = ApplyKernel(original_frame, pixels_dead);

	// Replace happy/dead pixels with adjusted mean frame
	//INFO << "NUC: Replacing happy pixels in frame";
    ReplaceImagePixels(original_frame, pixels_happy, values_happy_pixels);
	//INFO << "NUC: Replacing dead pixels in frame";
    ReplaceImagePixels(original_frame, pixels_dead, values_dead_pixels);

	// ----------------------------------------------------------------------------------------------
	
	//INFO << "NUC: Applying NUC to frame";
	arma::vec corrected_values = original_frame / nuc_values;

	//DEBUG << "NUC: NUC correction applied. First value after correction " << std::to_string(corrected_values(0));

	std::vector<double> vector_double = arma::conv_to<std::vector<double>>::from(corrected_values);
	std::vector<uint16_t> vector_int(vector_double.begin(), vector_double.end());

	return vector_int;
}

arma::mat NonUniformityCorrection::ApplyFilterMatrixWithOrderReduction(arma::mat input_matrix, int order, arma::mat domain)
{
	//Assumes a domain matrix that is square n x n, where n is an odd number greater than 1
	
	int domain_rows = domain.n_rows;
	int domain_cols = domain.n_cols;

	int matrix_rows = input_matrix.n_rows;
	int matrix_cols = input_matrix.n_cols;

	int offset = domain_rows / 2;

	arma::mat output(matrix_rows, matrix_cols);

	for (int i = 0; i < matrix_rows; i++)
	{
		for (int j = 0; j < matrix_cols; j++)
		{
			double rtn_value;
			arma::mat values(domain.n_rows, domain.n_cols);
			if (i - offset >= 0 && j - offset >= 0 && i + offset < matrix_rows && j + offset < matrix_cols) {
				arma::mat sub_matrix = input_matrix.submat(i - offset, j - offset, i + offset, j + offset);
				values = sub_matrix % domain;				
			}
			else {

				for (int m = 0; m < domain_rows; m++)
				{
					int r = i + (m - offset);

					if (r < 0)
						r = matrix_rows + r;

					if (r >= matrix_rows)
						r = r - matrix_rows;


					for (int n = 0; n < domain_cols; n++)
					{
						int c = j + (n - offset);
						
						if (c < 0)
							c = matrix_cols + c;
						
						if (c >= matrix_cols)
							c = c - matrix_cols;

						values(m, n) = domain(m, n) * input_matrix(r, c);
					}
				}
				
			}
			
			arma::vec values_flatten = arma::vectorise(values);
			values_flatten = arma::sort(values_flatten, "descend");

			if (values_flatten.n_elem > order)
				rtn_value = values_flatten(order - 1);
			else
				rtn_value = values_flatten.min();

			output(i, j) = rtn_value;
		}
	}
	
	return output;
}

double NonUniformityCorrection::ApplyFilterMatrixWithOrderReduction(arma::mat input_matrix, int order, arma::mat domain, int i, int j)
{
    //Assumes a domain (kernel/filter) matrix that is square n x n, where n is an odd number greater than 1

	int domain_rows = domain.n_rows;
	int domain_cols = domain.n_cols;

	int matrix_rows = input_matrix.n_rows;
	int matrix_cols = input_matrix.n_cols;

	int offset = domain_rows / 2;

	double output = 0;		
	double rtn_value;
	arma::mat values(domain.n_rows, domain.n_cols);
	
	if (i - offset >= 0 && j - offset >= 0 && i + offset < matrix_rows && j + offset < matrix_cols) {
		arma::mat sub_matrix = input_matrix.submat(i - offset, j - offset, i + offset, j + offset);
		values = sub_matrix % domain;
	}
	else {

		for (int m = 0; m < domain_rows; m++)
		{
			int r = i + (m - offset);

			if (r < 0)
				r = matrix_rows + r;

			if (r >= matrix_rows)
				r = r - matrix_rows;


			for (int n = 0; n < domain_cols; n++)
			{
				int c = j + (n - offset);

				if (c < 0)
					c = matrix_cols + c;

				if (c >= matrix_cols)
					c = c - matrix_cols;

				values(m, n) = domain(m, n) * input_matrix(r, c);
			}
		}

	}

	arma::vec values_flatten = arma::vectorise(values);
	values_flatten = arma::sort(values_flatten, "descend");

	if (values_flatten.n_elem > order)
		rtn_value = values_flatten(order - 1);
	else
		rtn_value = values_flatten.min();

	output = rtn_value;
		
	return output;
}

arma::vec NonUniformityCorrection::ApplyKernel(arma::vec data, arma::uvec indices)
{

	arma::mat data_matrix(data);
	data_matrix.reshape(x_pixels, y_pixels);
	data_matrix = data_matrix.t();

	int num_pixels = indices.n_elem;
	int pixel_index, pixel_row, pixel_col;
	arma::vec output(num_pixels);

	double value_before, value_new;

	for (int i = 0; i < num_pixels; i++)
	{

		pixel_index = indices(i);
		pixel_row = pixel_index / x_pixels;
		pixel_col = pixel_index % x_pixels;

		value_before = data_matrix(pixel_row, pixel_col);
        value_new = ApplyFilterMatrixWithOrderReduction(data_matrix, 5, kernel, pixel_row, pixel_col);

		output(i) = value_new;
	}
	
	return output;
}

arma::vec NonUniformityCorrection::ReplaceBrokenPixels(arma::vec values)
{
	// -------------------------------------------------------------
	// Find values for happy/dead pixels in selected frames
	arma::vec sorted_values = arma::sort(values);

	int index_min = std::ceil(0.00001 * sorted_values.n_elem);
	int index_max = std::floor(0.99999 * sorted_values.n_elem);

	double min_value_vector = sorted_values(index_min);
	double max_value_vector = sorted_values(index_max);

	//DEBUG << "NUC: Fixing pixels. Index of min value found is " << index_min;
	//DEBUG << "NUC: Fixing pixels. Min value found is " << min_value_vector;
	//DEBUG << "NUC: Fixing pixels. Index of max value found is " << index_max;
	//DEBUG << "NUC: Fixing pixels. Max value found is " << max_value_vector;

	// -------------------------------------------------------------
	// Apply kernel to the mean frame to replace happy/dead pixels

	arma::mat kernel = {{0, 0, 1, 0, 0},
						{0, 0, 1, 0, 0},
						{1, 1, 0, 1, 1},
						{0, 0, 1, 0, 0},
						{0, 0, 1, 0, 0} };

	arma::mat mean_frame(values);
	mean_frame.reshape(x_pixels, y_pixels);
	mean_frame = mean_frame.t();

	//arma::mat adj_mean_frame_matrix = arma::conv2(mean_frame, kernel, "same");
    arma::mat adj_mean_frame_matrix = ApplyFilterMatrixWithOrderReduction(mean_frame, 5, kernel);
	
	adj_mean_frame = arma::vectorise(adj_mean_frame_matrix.t());

	// -------------------------------------------------------------
	// Find indices of happy/dead pixels

	pixels_dead = arma::find(values <= min_value_vector);
	pixels_happy = arma::find(values >= max_value_vector);

	//DEBUG << "NUC: Mean frame value 2nd row, 1st col " << values(x_pixels);
	
	// -------------------------------------------------------------
	// Replace the happy/dead pixels in mean frame
    ReplacePixels(values, adj_mean_frame, pixels_dead);
    ReplacePixels(values, adj_mean_frame, pixels_happy);
	
	// -------------------------------------------------------------
	// DEBUG STATEMENTS
	double updated_min_value = values.min();
	double updated_max_value = values.max();
	//DEBUG << "NUC: After replacing all dead pixels, new minimum value is: " << updated_min_value;
	//DEBUG << "NUC: After replacing all happy pixels, new maximum value is: " << updated_max_value;

	arma::uvec check_pixels_happy = arma::find(values >= updated_max_value);
	arma::uvec check_pixels_dead = arma::find(values <= updated_min_value);

	//DEBUG << "NUC: Number of pixels below or equal to the new minimum: " << check_pixels_dead.n_elem;
	//DEBUG << "NUC: Number of pixels above or equal to the new maximum: " << check_pixels_happy.n_elem;
	// -------------------------------------------------------------

	//DEBUG << "NUC: Mean frame with pixels replaced and kernel applied; value at 2nd row, 1st col " << values(x_pixels);
	
	return values;
}

void NonUniformityCorrection::ReplacePixels(arma::vec &base, arma::vec &updated, arma::uvec pixels) {

	int num_pixels = pixels.size();

	int pixel_index, pixel_row, pixel_col;
	double value_before, value_new;
	
	for (int i = 0; i < num_pixels; i++)
	{
		
		pixel_index = pixels(i);
		pixel_row = pixel_index / x_pixels;
		pixel_col = pixel_index % x_pixels;

		value_before = base(pixel_index);
		value_new = updated(pixel_index);

		//DEBUG << "NUC: Replacing pixel at (row, col) " << pixel_row << ", " << pixel_col;
		//DEBUG << "NUC: Original value was " << value_before;
		//DEBUG << "NUC: New value is " << value_new;

		base(pixel_index) = value_new;

	}

}

void  NonUniformityCorrection::ReplaceImagePixels(arma::vec &frame, arma::uvec &indices, arma::vec &update)
{
	int num_pixels = indices.n_elem;
	int pixel_index, pixel_row, pixel_col;

	double value_before, value_new;

	for (int i = 0; i < num_pixels; i++)
	{

		pixel_index = indices(i);
		pixel_row = pixel_index / x_pixels;
		pixel_col = pixel_index % x_pixels;

		value_before = frame(pixel_index);
		value_new = update(i);

		//DEBUG << "NUC: Replacing frame pixel at (row, col) " << pixel_row << ", " << pixel_col;
		//DEBUG << "NUC: Original value was " << value_before;
		//DEBUG << "NUC: New value is " << value_new;

		frame(pixel_index) = value_new;

	}

}
