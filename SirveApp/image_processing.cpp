#include "image_processing.h"
#include <iostream>
#include <fstream>

std::vector<std::vector<uint16_t>> MedianFilter::MedianFilterStandard(VideoDetails & original, int window_size, QProgressDialog & progress)
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

    return frames_out;

}

std::vector<std::vector<uint16_t>> Deinterlacing::CrossCorrelation(VideoDetails & original, QProgressDialog & progress)
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
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
    }

    return frames_out;

}

 arma::cx_mat Deinterlacing::xcorr2(arma::mat inFrame1, arma::mat inFrame2)
{
    inFrame1 = inFrame1 - arma::mean(inFrame1.as_col());
    inFrame1.elem( arma::find(inFrame1 < (3.0*arma::stddev(inFrame1.as_col()))) ).zeros();
	inFrame2 = inFrame2 - arma::mean(inFrame2.as_col());
    inFrame2.elem( arma::find(inFrame2 < (3.0*arma::stddev(inFrame2.as_col()))) ).zeros();
    arma::cx_mat F = arma::fft2( inFrame1 ) % arma::fft2( arma::flipud( arma::fliplr( inFrame2 ) ) );
    arma::cx_mat cc_mat = arma::ifft2(F);
	
	return cc_mat;
}

std::vector<std::vector<uint16_t>> CenterOnTracks::CenterOnOSM(VideoDetails & original, std::vector<TrackFrame> osmFrames, QProgressDialog & progress)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels, nRows2 = nRows/2;
    int nCols = original.x_pixels, nCols2 = nCols/2;

    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
    int yOffset, xOffset;
    progress.setWindowTitle("Centering... ");
    for (int framei = 0; framei < num_video_frames; framei++){
        progress.setValue(framei);
        frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();  
        if (osmFrames.size()>=framei){
            try {      
                yOffset = osmFrames[framei].tracks[3].centroid_y  + 1;
                xOffset = osmFrames[framei].tracks[3].centroid_x  + 1;
                output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
                frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
            }
            catch (const std::exception& e) {
            frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame.t().as_col()));
            }
        }
    }

    return frames_out;

}
