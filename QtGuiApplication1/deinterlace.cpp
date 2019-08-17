#include "deinterlace.h"

Deinterlace::Deinterlace(deinterlace_type input_type, int x_pixel_input, int y_pixel_input)
{
	deinterlace_method = input_type;
	x_pixels = x_pixel_input;
	y_pixels = y_pixel_input;
	number_pixels = x_pixels * y_pixels;

	/*
	arma::mat temp1(9, 10, arma::fill::zeros);
	arma::mat temp2(temp1);
	temp1(1, 3) = 1;
	temp1(2, 3) = 1;
	temp1(3, 3) = 1;
	temp1(4, 3) = 1;
	temp1(4, 4) = 1;
	temp1(4, 5) = 1;

	temp2(5, 6) = 1;
	temp2(6, 6) = 1;
	temp2(7, 6) = 1;
	temp2(8, 6) = 1;
	temp2(8, 7) = 1;
	temp2(8, 8) = 1;

	arma::mat zeros(9, 10, arma::fill::zeros);
	arma::mat cross_correlation = cross_correlate_frame(zeros, temp1, temp2);

	arma::ivec offsets;
	arma::uword i_max = arma::abs(cross_correlation).index_max();
	arma::uvec peak_index = arma::ind2sub(arma::size(cross_correlation), i_max);

	//---------------------------------------------------------------------------------------
	int x = cross_correlation.n_rows;
	int y = cross_correlation.n_cols;

	arma::vec x_values = arma::regspace<arma::vec>(0, y - 1);
	arma::vec y_values = arma::regspace<arma::vec>(0, x - 1);
	arma::mat x_mat(x, y, arma::fill::zeros);
	arma::mat y_mat(x, y, arma::fill::zeros);

	mesh_grid(x_values, y_values, x_mat, y_mat);

	arma::mat v(cross_correlation);
	double max_value = cross_correlation.max();
	arma::uvec below_max = arma::find(cross_correlation < 0.5 * max_value);

	arma::vec zeros_v(below_max.n_elem, arma::fill::zeros);
	v.elem(below_max) = zeros_v;

	double v_sum = arma::accu(v);

	double ux = std::round(arma::accu(v % x_mat) / v_sum);
	double uy = std::round(arma::accu(v % y_mat) / v_sum);

	//---------------------------------------------------------------------------------------------

	int y_max_abs_value = (temp2.n_rows - 1.0) - peak_index(0);
	int x_max_abs_value = (temp2.n_cols - 1.0) - peak_index(1);

	int y_avg_cc = (temp2.n_rows - 1.0) - uy;
	int x_avg_cc = (temp2.n_cols - 1.0) - ux;

	DEBUG << "De-interlace: Using avereage cross correlation method. Max y was " << y_max_abs_value << " and x centroid was " << y_avg_cc;
	DEBUG << "De-interlace: Using avereage cross correlation method. Max x was " << x_max_abs_value << " and x centroid was " << x_avg_cc;

	int offset1a = std::round((((temp2.n_rows - 1.0) - peak_index(0)) + ((temp2.n_rows - 1.0) - uy)) / 2.0);
	int offset2b = std::round((((temp2.n_cols - 1.0) - peak_index(1)) + ((temp2.n_cols - 1.0) - ux)) / 2.0);

	int offset1 = std::round((y_max_abs_value + y_avg_cc) * 0.5);
	int offset2 = std::round((x_max_abs_value + x_avg_cc) * 0.5);

	//---------------------------------------------------------------------------------------------

	offsets << offset1 << offset2;

	int row_shift = arma::sign(offsets(0)) * (std::abs(offsets(0)) % temp1.n_rows);
	int col_shift = arma::sign(offsets(1)) * (std::abs(offsets(1)) % temp1.n_cols);
	
	arma::mat temp = arma::shift(temp1, row_shift, 0);
	arma::mat  odd = arma::shift(temp, col_shift, 1);

	temp1.save("original_mat.txt", arma::arma_ascii);
	temp2.save("shifted_mat.txt", arma::arma_ascii);
	odd.save("calculated_shift_mat.txt", arma::arma_ascii);
	*/

}

std::vector<uint16_t> Deinterlace::deinterlace_frame(std::vector<uint16_t>& frame)
{
	INFO << "De-interlace: De-interlace processing started";

	arma::mat even_frames, odd_frames, mat_frame, cross_correlation;
	mat_frame = create_frame(frame);
	create_even_odd_frames(mat_frame, odd_frames, even_frames);

	cross_correlation = cross_correlate_frame(mat_frame, odd_frames, even_frames);

	arma::ivec offsets;

	switch (deinterlace_method)
	{
	case max_absolute_value:
		{
			arma::uword i_max = arma::abs(cross_correlation).index_max();
			arma::uvec peak_index = arma::ind2sub(arma::size(cross_correlation), i_max);

			offsets << ((even_frames.n_rows - 1) - peak_index(0)) << ((even_frames.n_cols - 1) - peak_index(1));

			DEBUG << "De-interlace: Offsets for max absolute method are " << (even_frames.n_rows - 1) - peak_index(0) << " " << (even_frames.n_cols - 1) - peak_index(1);

			break;
		}
	case centroid:
	{
		int x = cross_correlation.n_rows;
		int y = cross_correlation.n_cols;
		
		arma::vec x_values = arma::regspace<arma::vec>(0, y - 1);
		arma::vec y_values = arma::regspace<arma::vec>(0, x - 1);
		arma::mat x_mat(x, y, arma::fill::zeros);
		arma::mat y_mat(x, y, arma::fill::zeros);
		
		mesh_grid(x_values, y_values, x_mat, y_mat);

		arma::mat v(cross_correlation);
		double max_value = cross_correlation.max();
		arma::uvec below_max = arma::find(cross_correlation < 0.5 * max_value);

		arma::vec zeros(below_max.n_elem, arma::fill::zeros);
		v.elem(below_max) = zeros;

		double v_sum = arma::accu(v);

		double ux = std::round(arma::accu(v % x_mat) / v_sum);
		double uy = std::round(arma::accu(v % y_mat) / v_sum);
		
		offsets << ((even_frames.n_rows - 1) - uy) << ((even_frames.n_cols - 1) - ux);

		DEBUG << "De-interlace: Offsets for centroid method are " << ((even_frames.n_rows - 1) - uy) << " " << ((even_frames.n_cols - 1) - ux);

		break;
	}
	
	case avg_cross_correlation:
	{
		//--------------------------------------------------------------------------------------
		
		arma::uword i_max = arma::abs(cross_correlation).index_max();
		arma::uvec peak_index = arma::ind2sub(arma::size(cross_correlation), i_max);
		
		//--------------------------------------------------------------------------------------
		
		int x = cross_correlation.n_rows;
		int y = cross_correlation.n_cols;

		arma::vec x_values = arma::regspace<arma::vec>(0, y - 1);
		arma::vec y_values = arma::regspace<arma::vec>(0, x - 1);
		arma::mat x_mat(x, y, arma::fill::zeros);
		arma::mat y_mat(x, y, arma::fill::zeros);

		mesh_grid(x_values, y_values, x_mat, y_mat);

		arma::mat v(cross_correlation);
		double max_value = cross_correlation.max();
		arma::uvec below_max = arma::find(cross_correlation < 0.5 * max_value);

		arma::vec zeros(below_max.n_elem, arma::fill::zeros);
		v.elem(below_max) = zeros;

		double v_sum = arma::accu(v);

		double ux = std::round(arma::accu(v % x_mat) / v_sum);
		double uy = std::round(arma::accu(v % y_mat) / v_sum);

		//--------------------------------------------------------------------------------------

		int y_max_abs_value = (even_frames.n_rows - 1.0) - peak_index(0);
		int x_max_abs_value = (even_frames.n_cols - 1.0) - peak_index(1);

		int y_avg_cc = (even_frames.n_rows - 1.0) - uy;
		int x_avg_cc = (even_frames.n_cols - 1.0) - ux;
	
		
		int offset1 = std::round((y_max_abs_value + y_avg_cc) * 0.5);
		int offset2 = std::round((x_max_abs_value + x_avg_cc) * 0.5);

		DEBUG << "De-interlace: Using avereage cross correlation method. Max y was " << y_max_abs_value << " and y centroid was " << y_avg_cc;
		DEBUG << "De-interlace: Using avereage cross correlation method. Max x was " << x_max_abs_value << " and x centroid was " << x_avg_cc;
		DEBUG << "De-interlace: Calculated offset is y / x: " << offset1 << " / " << offset2;

		offsets << offset1 << offset2;
		
		break;
	}
	default:
		return std::vector<uint16_t>();
	}

	int row_shift = arma::sign(offsets(0)) * (std::abs(offsets(0)) % odd_frames.n_rows);
	int col_shift = arma::sign(offsets(1)) * (std::abs(offsets(1)) % odd_frames.n_cols);

	arma::mat temp = arma::shift(odd_frames, row_shift, 0);
	arma::mat  odd = arma::shift(temp, col_shift, 1);

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

	DEBUG << "De-interlace: Output frame 1st row and 1st column is " << std::to_string(out_frame_flat(0));

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

	arma::mat cc_mat = fast_fourier_transform(frame1_pad, frame2_pad);

	DEBUG << "De-interlace: Cross correlation matrix 1st row, 1st Column value is " << std::to_string(cc_mat(0, 0));
	
	return cc_mat;
}

arma::mat Deinterlace::fast_fourier_transform(arma::mat matrix1, arma::mat matrix2)
{
	//--------------------------------------------------------------------------------
	//FFT of matrix 1

	int r1 = matrix1.n_rows;
	int c1 = matrix1.n_cols;
	int N1 = r1 * c1;
	arma::vec matrix1_flat = arma::vectorise(matrix1.t());

	fftw_complex *out1;
	out1 = (fftw_complex*)fftw_malloc(N1 * sizeof(fftw_complex));

	std::vector<double>vector1_flat = arma::conv_to<std::vector<double>>::from(matrix1_flat);
	double *pt_vector1_flat = vector1_flat.data();

	/* forward Fourier transform, save the result in 'out' */
	fftw_plan plan1 = fftw_plan_dft_r2c_2d(matrix1.n_rows, matrix1.n_cols, pt_vector1_flat, out1, FFTW_ESTIMATE);
	fftw_execute(plan1);
		
	//--------------------------------------------------------------------------------
	//FFT of matrix 2
	
	int r2 = matrix2.n_rows;
	int c2 = matrix2.n_cols;
	int N2 = r2 * c2;
	arma::vec matrix2_flat = arma::vectorise(matrix2.t());

	fftw_complex *out2;
	out2 = (fftw_complex*)fftw_malloc(N2 * sizeof(fftw_complex));

	std::vector<double>vector2_flat = arma::conv_to<std::vector<double>>::from(matrix2_flat);
	double *pt_vector2_flat = vector2_flat.data();

	// forward Fourier transform, save the result in 'out'
	fftw_plan plan2 = fftw_plan_dft_r2c_2d(matrix2.n_rows, matrix2.n_cols, pt_vector2_flat, out2, FFTW_ESTIMATE);
	fftw_execute(plan2);
	

	//--------------------------------------------------------------------------------
	//Inverse FFT of product

	fftw_complex *in3;
	in3 = (fftw_complex*)fftw_malloc(N1 * sizeof(fftw_complex));
	
	double *out3;
	out3 = (double*)malloc(sizeof(double) * N2);
	fftw_plan inverse = fftw_plan_dft_c2r_2d(r2, c2, in3, out3, FFTW_ESTIMATE);
	
	for (int j = 0; j < N2; j++)
	{
		in3[j][0] = out1[j][0] * out2[j][0] - out1[j][1] * out2[j][1];
		in3[j][1] = out1[j][1] * out2[j][0] + out1[j][0] * out2[j][1];
	}
		
	fftw_execute(inverse);
	
	std::vector<double> out_vector(out3, out3 + r2 * c2);
	arma::vec out_arma_vector(out_vector);
	arma::mat out_mat(out_arma_vector / N2);
	out_mat.reshape(c1, r1);
	out_mat = out_mat.t();

	//-------------------------------------------------------------------

	fftw_free(out1);
	fftw_free(out2);
	free(out3);
	fftw_free(in3);

	fftw_destroy_plan(plan1);
	fftw_destroy_plan(plan2);
	fftw_destroy_plan(inverse);
	
	return out_mat;
}

void Deinterlace::mesh_grid(arma::vec x_input, arma::vec y_input, arma::mat & x_mat, arma::mat & y_mat)
{
	int n_rows = x_mat.n_rows;
	int n_cols = x_mat.n_cols;

	if (n_rows != y_mat.n_rows)
		return;

	if (n_cols != y_mat.n_cols)
		return;

	if (n_rows != y_input.n_elem)
		return;

	if (n_cols != x_input.n_elem)
		return;

	for (int i = 0; i < n_rows; i++)
	{
		x_mat.row(i) = x_input.t();
		y_mat.col(i) = y_input;
	}
	for (int i = 0; i < n_rows; i++)
		x_mat.row(i) = x_input.t();

	for (int j = 0; j < n_cols; j++)
		y_mat.col(j) = y_input;

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

	DEBUG << "De-interlace: Original frame 2nd row, 1st column value is " << std::to_string(mat_frame(1, 0));

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

	DEBUG << "De-interlace: Odd frame 1st row, 1st column value is " << std::to_string(odd_frame(0, 0));
	DEBUG << "De-interlace: Even frame 1st row, 1st column value is " << std::to_string(even_frame(0, 0));

}

Deinterlace::~Deinterlace()
{
}

