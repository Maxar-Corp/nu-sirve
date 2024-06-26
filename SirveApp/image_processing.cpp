#include "image_processing.h"
#include <iostream>
#include <fstream>

std::vector<std::vector<uint16_t>> ImageProcessing::MedianFilterStandard(VideoDetails & original, int window_size, QProgressDialog & progress)
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

std::vector<std::vector<uint16_t>>ImageProcessing::DeinterlaceCrossCorrelation(VideoDetails & original, QProgressDialog & progress)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels, nRows2 = nRows/2;
    int nCols = original.x_pixels;

    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
    arma::mat frame0(nRows, nCols);
   	arma::uvec odd_rows = arma::regspace<arma::uvec>(0, 2, nRows - 1);
    arma::uvec even_rows = arma::regspace<arma::uvec>(1, 2, nRows);

	arma::mat offsets(1,6);
    arma::mat odd_frame(nRows2,nCols);
    arma::mat even_frame(nRows2,nCols);
    arma::mat even_frame0(nRows2,nCols);
    arma::cx_mat cc_mat(nRows2,nCols);
    arma::uword i_max;
    arma::uvec peak_index;
    int yOffset, xOffset;
    arma::rowvec V(6);
    progress.setWindowTitle("Deinterlacing... ");

    int n_rows_new = pow(2, ceil(log(nRows2)/log(2))), n_rows_new2 = round(n_rows_new/2);  
    int n_cols_new = pow(2, ceil(log(nCols)/log(2))), n_cols_new2 = round(n_cols_new/2);
    // int n_rows_new = 2*nRows2 - 1, n_rows_new2 = round(n_rows_new/2);  
    // int n_cols_new = 2*nCols - 1, n_cols_new2 = round(n_cols_new/2);
    for (int framei = 0; framei < num_video_frames; framei++){
        progress.setValue(framei);
        frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();
        frame0 = frame;
        frame = frame - arma::mean(frame.as_col());
        odd_frame = frame.rows(odd_rows);
	    even_frame = frame.rows(even_rows);    
        even_frame0 = frame0.rows(even_rows);
        cc_mat = ImageProcessing::xcorr2(odd_frame,even_frame,n_rows_new,n_cols_new, framei);
        i_max = cc_mat.index_max();
		peak_index = arma::ind2sub(arma::size(cc_mat), i_max);
        int yp = peak_index(0);
        int xp = peak_index(1);
        yOffset = (yp < n_rows_new2)*(yp + 1) - (yp > n_rows_new2)*(n_rows_new - yp - 1);
        xOffset = (xp < n_cols_new2)*xp - (xp > n_cols_new2)*(n_cols_new - xp - 1);
        output = frame0;
        double d = sqrt(pow(xOffset,2) + pow(yOffset,2));
        V[0] = framei;
        V[1] = xp;
        V[2] = yp;
        V[3] = xOffset;
        V[4] = yOffset;
        V[5] = d;
        offsets.insert_rows(offsets.n_rows,V);
        if(d < 20 && d >1.5) {
                output.rows(even_rows) = arma::shift(arma::shift(even_frame0,yOffset,0),xOffset,1);
            }
        output = output - arma::min(output.as_col());
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
        if (framei == 151){
            frame0.save("frame0_152.bin",arma::arma_binary);
            frame.save("frame_152.bin",arma::arma_binary);
            odd_frame.save("odd_frame_152.bin",arma::arma_binary);
            even_frame.save("even_frame_152.bin",arma::arma_binary);
            cc_mat.save("cc_mat_152.bin",arma::arma_binary);
        }
        if (framei == 449){
            frame0.save("frame0_449.bin",arma::arma_binary);
            frame.save("frame_449.bin",arma::arma_binary);
            odd_frame.save("odd_frame_449.bin",arma::arma_binary);
            even_frame.save("even_frame_449.bin",arma::arma_binary);
            cc_mat.save("cc_mat_449.bin",arma::arma_binary);
        }
        if (framei == 427){
            frame0.save("frame0_427.bin",arma::arma_binary);
            frame.save("frame_427.bin",arma::arma_binary);
            odd_frame.save("odd_frame_427.bin",arma::arma_binary);
            even_frame.save("even_frame_427.bin",arma::arma_binary);
            cc_mat.save("cc_mat_427.bin",arma::arma_binary);
        }
    }
    offsets.shed_row(0);
    offsets.save("offsets.bin",arma::arma_binary);
    return frames_out;
}

 arma::cx_mat ImageProcessing::xcorr2(arma::mat inFrame1, arma::mat inFrame2, int nRows, int nCols, int framei)
{
    inFrame1.elem( arma::find(arma::abs(inFrame1) < (3.0*arma::stddev(inFrame1.as_col()))) ).zeros();
    inFrame2.elem( arma::find(arma::abs(inFrame2) < (3.0*arma::stddev(inFrame2.as_col()))) ).zeros();
    arma::mat inFrame1_pad(nRows,nCols);
    arma::mat inFrame2_pad(nRows,nCols);
    inFrame1_pad.zeros();
    inFrame2_pad.zeros();
    inFrame1_pad(0,0,arma::size(inFrame1)) = inFrame1;
    inFrame2_pad(0,0,arma::size(inFrame2)) = inFrame2;
    arma::cx_mat F = arma::fft2(inFrame1_pad);
    arma::mat G0 = arma::flipud(arma::fliplr(inFrame2_pad));
    arma::cx_mat G = arma::fft2(G0);
    arma::cx_mat FG = F % G;
    arma::cx_mat cc_mat = arma::ifft2(FG);
    // if (framei == 151){
    //     inFrame1.save("inFrame1_152.bin",arma::arma_binary);
    //     inFrame2.save("inFrame2_152.bin",arma::arma_binary);
    //     // inFrame1_pad.save("inFrame1_pad_152.bin",arma::arma_binary);
    //     // inFrame2_pad.save("inFrame2_pad_152.bin",arma::arma_binary);
    //     F.save("F_152.bin",arma::arma_binary);
    //     G0.save("G0_152.bin",arma::arma_binary);
    //     G.save("G_152.bin",arma::arma_binary);
    //     FG.save("FG_152.bin",arma::arma_binary);
    //     cc_mat.save("cc_mat_152.bin",arma::arma_binary);
    // }
    // if (framei == 449){
    //     inFrame1.save("inFrame1_449.bin",arma::arma_binary);
    //     inFrame2.save("inFrame2_449.bin",arma::arma_binary);
    //     // inFrame1_pad.save("inFrame1_pad_449.bin",arma::arma_binary);
    //     // inFrame2_pad.save("inFrame2_pad_449.bin",arma::arma_binary);
    //     F.save("F_449.bin",arma::arma_binary);
    //     F.save("F_ascii_449.txt",arma::arma_ascii);
    //     G0.save("G0_449.bin",arma::arma_binary);
    //     G.save("G_449.bin",arma::arma_binary);
    //     FG.save("FG_449.bin",arma::arma_binary);
    //     cc_mat.save("cc_mat_449.bin",arma::arma_binary);
    // }
    // if (framei == 427){
    //     inFrame1.save("inFrame1_427.bin",arma::arma_binary);
    //     inFrame2.save("inFrame2_427.bin",arma::arma_binary);
    //     inFrame1_pad.save("inFrame1_pad_427.bin",arma::arma_binary);
    //     inFrame2_pad.save("inFrame2_pad_427.bin",arma::arma_binary);
    //     arma::mat Fr = arma::real(F);
    //     arma::mat Fc = arma::imag(F);
    //     Fr.save("Fr_427.bin",arma::arma_binary);
    //     Fc.save("Fc_427.bin",arma::arma_binary);
    //     // F.save("F_ascii_427.txt",arma::arma_ascii);
    //     G0.save("G0_427.bin",arma::arma_binary);
    //     arma::mat Gr = arma::real(G);
    //     arma::mat Gc = arma::imag(G);
    //     // G.save("G_427.bin",arma::arma_binary);
    //     // FG.save("FG_427.bin",arma::arma_binary);
    //     cc_mat.save("cc_mat_427.bin",arma::arma_binary);
    //     cc_mat.save("cc_mat_427.txt",arma::arma_ascii);
    // }
	return cc_mat;
}

std::vector<std::vector<uint16_t>> ImageProcessing::CenterOnOSM(VideoDetails & original, int track_id, std::vector<TrackFrame> osmFrames, std::vector<std::vector<int>> & OSM_centered_offsets, QProgressDialog & progress)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels;
    int nCols = original.x_pixels;

    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
    int yOffset, xOffset;

    progress.setWindowTitle("Centering... ");
    if (track_id>0){
        for (int framei = 0; framei < num_video_frames; framei++){
            progress.setValue(framei);
            frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();  
            if (osmFrames[framei].tracks.find(track_id) != osmFrames[framei].tracks.end()) {      
                yOffset = osmFrames[framei].tracks[track_id].centroid_y;
                xOffset = osmFrames[framei].tracks[track_id].centroid_x;
                output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
                frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
                OSM_centered_offsets.push_back({framei,xOffset,yOffset});
            }
            else {
                frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame.t().as_col()));
            }

        }
    }
    else{
         for (int framei = 0; framei < num_video_frames; framei++){
            progress.setValue(framei);
            frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();  
            output = frame;
            bool cont_search = true;
            int i = 0;
            while (cont_search && i < osmFrames[framei].tracks.size()){
                if (osmFrames[framei].tracks[i].centroid_x != NULL){
                    yOffset = osmFrames[framei].tracks[i].centroid_y;
                    xOffset = osmFrames[framei].tracks[i].centroid_x;
                    output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
                    OSM_centered_offsets.push_back({framei,xOffset,yOffset});
                    cont_search = false;
                }
                else{
                    i+=1;
                } 
            }
            frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));                  
        }
    }

    return frames_out;

}

std::vector<std::vector<uint16_t>> ImageProcessing::CenterOnManual(VideoDetails & original, int track_id, std::vector<TrackFrame> manualFrames, std::vector<std::vector<int>> & manual_centered_offsets, QProgressDialog & progress)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels, nRows2 = nRows/2;
    int nCols = original.x_pixels, nCols2 = nCols/2;

    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
    int yOffset, xOffset;

    if (track_id>0){
        for (int framei = 0; framei < num_video_frames; framei++){
            progress.setValue(framei);
            frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();  
            if (manualFrames[framei].tracks.find(track_id) != manualFrames[framei].tracks.end()) {      
                yOffset = manualFrames[framei].tracks[track_id].centroid_y - nRows2;
                xOffset = manualFrames[framei].tracks[track_id].centroid_x - nCols2;
                output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
                frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
                manual_centered_offsets.push_back({framei,xOffset,yOffset});
            }
            else {
                frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame.t().as_col()));
            }

        }
    }
    else{
         for (int framei = 0; framei < num_video_frames; framei++){
            progress.setValue(framei);
            frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();  
            output = frame;
            bool cont_search = true;
            int i = 0;
            while (cont_search && i < manualFrames[framei].tracks.size()){
                if (manualFrames[framei].tracks[i].centroid_x != NULL){
                    yOffset = manualFrames[framei].tracks[i].centroid_y - nRows2;
                    xOffset = manualFrames[framei].tracks[i].centroid_x - nCols2;
                    output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
                    manual_centered_offsets.push_back({framei,xOffset,yOffset});
                    cont_search = false;
                }
                else{
                    i+=1;
                } 
            }
            frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));                  
        }
    }
    return frames_out;
}
