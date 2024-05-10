#include "image_processing.h"

std::vector<std::vector<uint16_t>> MedianFilter::median_filter_standard(video_details & original, int window_size, QProgressDialog & progress)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    arma::mat window(window_size,window_size);
    int nRows = original.y_pixels;
    int nCols = original.x_pixels;
    arma::mat output(nRows, nCols);
    arma::mat frame(nCols, nRows);
    arma::mat paddedInput(nCols + window_size - 1, nRows + window_size - 1);
    for (int framei = 0; framei < num_video_frames; framei++){
        progress.setValue(framei);
        frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows);
        paddedInput.zeros();
        paddedInput.submat(window_size / 2, window_size / 2, nCols - 1 + window_size / 2, nRows - 1 + window_size / 2) = frame;

        // Create an output matrix
        output.zeros();

        // Iterate over the input matrix
        for (arma::uword i = 0; i < nCols; i++) {
            for (arma::uword j = 0; j < nRows; j++) {
            // Get the window of pixels around the current pixel
            window = paddedInput.submat(i, j, i + window_size - 1, j + window_size - 1);
            output(j, i) = arma::median(window.as_col());
            }
        }

        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.as_row()));
    }

    // arma::mat test(frames_out[0].size(), frames_out.size());
    // for (int framei = 0; framei < frames_out.size(); framei++){
    //     test.col(framei) = arma::conv_to<arma::vec>::from(frames_out[framei]);
    // }
    // test.save("test.bin",arma::arma_binary);

    return frames_out;

}

std::vector<std::vector<uint16_t>> Deinterlacing::cross_correlation(video_details & original, QProgressDialog & progress)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels;
    int nCols = original.x_pixels;

    arma::mat output(nRows, nCols);
    arma::mat frame(nCols, nRows);
   
   	arma::uvec odd_rows = arma::regspace<arma::uvec>(1, 2, nCols);
	arma::uvec even_rows = arma::regspace<arma::uvec>(2, 2, nCols);

	// Set index values to start counting from zero
	even_rows = even_rows - 1;
	odd_rows = odd_rows - 1;

	//Setup odd / even video frames
    arma::mat odd_frame(even_rows.size(),nRows);
    arma::mat even_frame(odd_rows.size(),nRows);
    arma::mat cc_mat(even_rows.size(),nRows);
    for (int framei = 0; framei < num_video_frames; framei++){
        progress.setValue(framei);
        frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows);
        odd_frame = frame.rows(odd_rows);
	    even_frame = frame.rows(even_rows);    
        cc_mat = Deinterlacing::xcorr2(even_frame,odd_frame);
        arma::uword i_max = arma::abs(cc_mat).index_max();
		arma::uvec peak_index = arma::ind2sub(arma::size(cc_mat), i_max);
        output = arma::shift(frame,peak_index(0) - nCols,0);
        output = arma::shift(output,peak_index(1) - nRows,1);
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.as_row()));
    }

    // arma::mat test(frames_out[0].size(), frames_out.size());
    // for (int framei = 0; framei < frames_out.size(); framei++){
    //     test.col(framei) = arma::conv_to<arma::vec>::from(frames_out[framei]);
    // }
    // test.save("test.bin",arma::arma_binary);

    return frames_out;

}

 arma::mat Deinterlacing::xcorr2(arma::mat inFrame1, arma::mat inFrame2)
{
	inFrame1 = (inFrame1 - arma::mean(inFrame1.as_col()))/arma::stddev(inFrame1.as_col());
	inFrame2 = (inFrame2 - arma::mean(inFrame2.as_col()))/arma::stddev(inFrame2.as_col());

	arma::mat cc_mat = arma::conv2(inFrame1,arma::flipud(arma::fliplr(inFrame2)),"same");
	
	return cc_mat;
}
