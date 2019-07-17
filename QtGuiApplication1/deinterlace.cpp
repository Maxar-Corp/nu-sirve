#include "deinterlace.h"

Deinterlace::Deinterlace(deinterlace_type input_type, int x_pixel_input, int y_pixel_input)
{
	deinterlace_method = input_type;
	x_pixels = x_pixel_input;
	y_pixels = y_pixel_input;
	number_pixels = x_pixels * y_pixels;

}

std::vector<uint16_t> Deinterlace::deinterlace_frame(std::vector<uint16_t>& frame)
{

	arma::mat even_frames, odd_frames, mat_frame, cross_correlation;
	mat_frame = create_frame(frame);
	create_even_odd_frames(mat_frame, odd_frames, even_frames);

	cross_correlation = cross_correlate_frame(mat_frame, odd_frames, even_frames);

	arma::uvec offsets;

	switch (deinterlace_method)
	{
	case max_absolute_value:
		{
			arma::uword i_max = cross_correlation.index_max();
			arma::uvec peak_index = arma::ind2sub(arma::size(cross_correlation), i_max);

			arma::SizeMat cc_size = arma::size(cross_correlation);
			offsets << (cc_size.n_rows - peak_index(0)) << (cc_size.n_cols - peak_index(1));

			break;
		}
	case centroid:
	{
		
		break;
	}
	
	case avg_cross_correlation:
	{
	
		break;
	}
	default:
		return std::vector<uint16_t>();
	}

	arma::mat temp = arma::shift(odd_frames, offsets(0) % odd_frames.n_rows, 0);
	arma::mat  odd = arma::shift(temp, offsets(1) % odd_frames.n_cols, 1);

	arma::mat out_frame(y_pixels, x_pixels, arma::fill::zeros);
	
	for (int i = 0; i < even_frames.n_rows; i++)
	{
		out_frame.row(2 * i) = odd.row(i);
		out_frame.row(2 * i + 1) = even_frames.row(i);
	}

	// Put matrix back into C++ vector
	out_frame = out_frame.t();
	arma::vec out_frame_flat = arma::vectorise(out_frame);
	std::vector<double>out_vector = arma::conv_to<std::vector<double>>::from(out_frame_flat);
	std::vector<uint16_t> converted_values(out_vector.begin(), out_vector.end());

	return converted_values;
}

arma::mat Deinterlace::cross_correlate_frame(arma::mat & mat_frame, arma::mat odd_frames, arma::mat even_frames)
{
	double mean_value = arma::mean(arma::mean(mat_frame));
	arma::mat frame1 = odd_frames - mean_value;
	arma::mat frame2 = even_frames - mean_value;

	int rows_frame1 = frame1.n_rows;
	int cols_frame1 = frame1.n_cols;
	int rows_frame2 = frame2.n_rows;
	int cols_frame2 = frame2.n_cols;

	arma::mat frame1_pad(rows_frame1 + rows_frame2 - 1, cols_frame1 + cols_frame2 - 1, arma::fill::zeros);
	arma::mat frame2_pad(frame1_pad);

	frame1_pad.submat(0, 0, rows_frame1 - 1, cols_frame1 - 1) = frame1;

	arma::uvec reverse_index_rows = arma::regspace<arma::uvec>(rows_frame2 - 1, 0);
	arma::uvec reverse_index_cols = arma::regspace<arma::uvec>(cols_frame2 - 1, 0);
	frame2_pad.submat(0, 0, rows_frame2 - 1, cols_frame2 - 1) = frame2.submat(reverse_index_rows, reverse_index_cols);

	arma::cx_mat fft_frame1 = arma::fft2(frame1_pad);
	arma::cx_mat fft_frame2 = arma::fft2(frame2_pad);

	arma::mat cc_mat = arma::real(arma::ifft2(fft_frame1 % fft_frame2));

	return cc_mat;
}

void Deinterlace::test_conversion(std::vector<uint16_t>& frame)
{
	
	// Test shift algorithm
	arma::mat test(10, 10, arma::fill::zeros);
	test(0, 0) = 1;
	test(0, 1) = 1;
	test(1, 0) = 1;
	test(1, 1) = 1;

	arma::mat test2 = arma::shift(test, 3, 0);
	arma::mat test3 = arma::shift(test, 3, 1);
	//test.save("test.txt", arma::arma_ascii);
	//test2.save("test_shift2.txt", arma::arma_ascii);
	//test3.save("test_shift3.txt", arma::arma_ascii);
	
	
	// Convert the frame from a vector into armadillo matrix 
	std::vector<double> converted_values(frame.begin(), frame.end());
	arma::vec temp(converted_values);
	arma::mat mat_frame(temp);
	mat_frame.reshape(y_pixels, x_pixels);


	arma::vec out_frame_flat = arma::vectorise(mat_frame);
	std::vector<double>out_vector = arma::conv_to<std::vector<double>>::from(out_frame_flat);

	bool check = arma::approx_equal(temp, out_frame_flat, "absdiff", 0.0001);
}

arma::mat Deinterlace::create_frame(std::vector<uint16_t>& frame) {


	// Convert the frame from a vector into armadillo matrix 
	std::vector<double> converted_values(frame.begin(), frame.end());
	arma::vec temp(converted_values);
	arma::mat mat_frame(temp);
	mat_frame.reshape(x_pixels, y_pixels);
	mat_frame = mat_frame.t();

	return mat_frame;
}

void Deinterlace::create_even_odd_frames(arma::mat & mat_frame, arma::mat & odd_frame, arma::mat & even_frame)
{

	arma::uvec odd_rows = arma::regspace<arma::uvec>(1, 2, y_pixels);
	arma::uvec even_rows = arma::regspace<arma::uvec>(2, 2, y_pixels);

	// Set index values to start counting from zero
	even_rows = even_rows - 1;
	odd_rows = odd_rows - 1;

	//Setup odd / even video frames
	odd_frame = mat_frame.rows(odd_rows);
	even_frame = mat_frame.rows(even_rows);

}

Deinterlace::~Deinterlace()
{
}

