#include "image_processing.h"
#include <iostream>
#include <fstream>

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
    int nRows = original.y_pixels, nRows2 = nRows/2, nRows4 = nRows2/2;
    int nCols = original.x_pixels, nCols2 = nCols/2;

    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
   	arma::uvec odd_rows = arma::regspace<arma::uvec>(0, 2, nRows - 1);
	arma::uvec even_rows = arma::regspace<arma::uvec>(1, 2, nRows);

	//Setup odd / even video frames
    arma::mat odd_frame(nRows2,nCols);
    arma::mat even_frame(nRows2,nCols);
    arma::cx_mat cc_mat(nRows2,nCols);
    arma::uword i_max;
    arma::uvec peak_index;
    int yOffset, xOffset;
    progress.setWindowTitle("Deinterlacing... ");
    for (int framei = 0; framei < num_video_frames; framei++){
        progress.setValue(framei);
        frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();
        odd_frame = frame.rows(odd_rows);
	    even_frame = frame.rows(even_rows);    
        cc_mat = Deinterlacing::xcorr2(even_frame,odd_frame);
        i_max = arma::abs(cc_mat).index_max();
		peak_index = arma::ind2sub(arma::size(cc_mat), i_max);
        yOffset = peak_index(0) - nRows2 + 1;
        xOffset = peak_index(1) - nCols + 1;
        if (framei == 448){
            cc_mat.save("cc_mat448.bin",arma::arma_binary);
            peak_index.save("peak_index448.bin",arma::arma_binary);
            frame.save("frame448.bin",arma::arma_binary);
            odd_frame.save("odd_frame448.bin",arma::arma_binary);
            even_frame.save("even_frame448.bin",arma::arma_binary);
            std::ofstream myfile1("imax448.txt");
            myfile1 << i_max;
            myfile1.close();
            std::ofstream myfile2("off448.txt");
            myfile2 << xOffset << yOffset;
            myfile2.close();

        }
        if (framei == 277){
            cc_mat.save("cc_mat277.bin",arma::arma_binary);
            peak_index.save("peak_index277.bin",arma::arma_binary);
            frame.save("frame277.bin",arma::arma_binary);
            odd_frame.save("odd_frame277.bin",arma::arma_binary);
            even_frame.save("even_frame277.bin",arma::arma_binary);
            std::ofstream myfile3("imax277.txt");
            myfile3 << i_max;
            myfile3.close();
            std::ofstream myfile4("off277.txt");
            myfile4 << xOffset << yOffset;
            myfile4.close();
        }
        if(yOffset % nRows2 == 0){
            yOffset = 0;
        }
        if(xOffset % nCols == 0){
            xOffset = 0;
        }
        output.rows(odd_rows) = odd_frame;
        output.rows(even_rows) = even_frame;
        if(abs(xOffset) >= 2 || abs(yOffset) >= 2) {
            if(xOffset % nCols2 < 10 && yOffset % nRows4 < 10){
                output.rows(even_rows) = arma::shift(arma::shift(even_frame,-yOffset,0),-xOffset,1);
            }
        }
     
        // output.rows(even_rows) = even_frame;
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
    }

    // arma::mat test(frames_out[0].size(), frames_out.size());
    // for (int framei = 0; framei < frames_out.size(); framei++){
    //     test.col(framei) = arma::conv_to<arma::vec>::from(frames_out[framei]);
    // }
    // test.save("test.bin",arma::arma_binary);

    return frames_out;

}

 arma::cx_mat Deinterlacing::xcorr2(arma::mat inFrame1, arma::mat inFrame2)
{
	// inFrame1 = (inFrame1 - arma::mean(inFrame1.as_col()))/arma::stddev(inFrame1.as_col());
	// inFrame2 = (inFrame2 - arma::mean(inFrame2.as_col()))/arma::stddev(inFrame2.as_col());
    inFrame1 = inFrame1 - arma::mean(inFrame1.as_col());
    inFrame1.elem( arma::find(inFrame1 < (3.0*arma::stddev(inFrame1.as_col()))) ).zeros();
	inFrame2 = inFrame2 - arma::mean(inFrame2.as_col());
    inFrame2.elem( arma::find(inFrame2 < (3.0*arma::stddev(inFrame2.as_col()))) ).zeros();

	// arma::mat cc_mat = arma::conv2(inFrame1,arma::flipud(arma::fliplr(inFrame2)),"same");

    arma::cx_mat F = arma::fft2( inFrame1 ) % arma::fft2( arma::flipud( arma::fliplr( inFrame2 ) ) );
    arma::cx_mat cc_mat = arma::ifft2(F);
	
	return cc_mat;
}
