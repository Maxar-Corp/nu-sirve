#include "image_processing.h"
ImageProcessing::ImageProcessing()
{
    frameval = 0;
}

ImageProcessing::~ImageProcessing() {
}

std::vector<std::vector<uint16_t>> ImageProcessing::MedianFilterStandard(VideoDetails & original, int window_size)
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
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
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

std::vector<std::vector<uint16_t>>ImageProcessing::DeinterlaceCrossCorrelation(VideoDetails & original)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels, nRows2 = nRows/2;
    int nCols = original.x_pixels;
    int n_rows_new = pow(2, ceil(log(nRows2)/log(2))), n_rows_new2 = round(n_rows_new/2);  
    int n_cols_new = pow(2, ceil(log(nCols)/log(2))), n_cols_new2 = round(n_cols_new/2);
    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
    arma::mat frame0(nRows, nCols);
   	arma::uvec odd_rows = arma::regspace<arma::uvec>(0, 2, nRows - 1);
    arma::uvec even_rows = arma::regspace<arma::uvec>(1, 2, nRows);
    arma::mat odd_frame(nRows2,nCols);
    arma::mat even_frame(nRows2,nCols);
    arma::mat even_frame0(nRows2,nCols);
    arma::cx_mat cc_mat(n_rows_new,n_cols_new);
    arma::uword i_max;
    arma::uvec peak_index;
    int yOffset, xOffset;

    for (int framei = 0; framei < num_video_frames; framei++){
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
        frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();
        frame0 = frame;
        frame = frame - arma::mean(frame.as_col());
        odd_frame = frame.rows(odd_rows);
	    even_frame = frame.rows(even_rows);    
        even_frame0 = frame0.rows(even_rows);
        cc_mat = ImageProcessing::xcorr2(odd_frame,even_frame,n_rows_new,n_cols_new, framei);
        i_max = cc_mat.index_max();
		peak_index = arma::ind2sub(arma::size(cc_mat), i_max);
        yOffset = (peak_index(0) < n_rows_new2)*(peak_index(0) + 1) - (peak_index(0) > n_rows_new2)*(n_rows_new - peak_index(0) - 1);
        xOffset = (peak_index(1) < n_cols_new2)*peak_index(1) - (peak_index(1) > n_cols_new2)*(n_cols_new - peak_index(1) - 1);
        output = frame0;
        double d = sqrt(pow(xOffset,2) + pow(yOffset,2));
        if(d < 35 && d >1.5){
            output.rows(even_rows) = arma::shift(arma::shift(even_frame0,yOffset,0),xOffset,1);
        }
        output = output - arma::min(output.as_col());
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
    }
    return frames_out;
}


std::vector<std::vector<uint16_t>> ImageProcessing::CenterOnOSM(VideoDetails & original, int track_id, std::vector<TrackFrame> osmFrames, std::vector<std::vector<int>> & OSM_centered_offsets)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels;
    int nCols = original.x_pixels;

    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
    int yOffset, xOffset;

    if (track_id>0){
        for (int framei = 0; framei < num_video_frames; framei++){
            UpdateProgressBar(framei);
            QCoreApplication::processEvents();
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
            UpdateProgressBar(framei);
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

std::vector<std::vector<uint16_t>> ImageProcessing::CenterOnManual(VideoDetails & original, int track_id, std::vector<TrackFrame> manualFrames, std::vector<std::vector<int>> & manual_centered_offsets)
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
            UpdateProgressBar(framei);
            QCoreApplication::processEvents();
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
            UpdateProgressBar(framei);
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


std::vector<std::vector<uint16_t>> ImageProcessing::CenterOnBrightest(VideoDetails & original, std::vector<std::vector<int>> & brightest_centered_offsets)
{
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels, nRows2 = nRows/2;
    int nCols = original.x_pixels, nCols2 = nCols/2;
    int yOffset0, xOffset0, yOffset, xOffset, i_max, test1a, test2a;
    int n_rows_new = pow(2, ceil(log(nRows)/log(2))), n_rows_new2 = round(n_rows_new/2);  
    int n_cols_new = pow(2, ceil(log(nCols)/log(2))), n_cols_new2 = round(n_cols_new/2);
    double ds = .5;
    int N = 12;
    arma::uvec peak_index; 
    arma::mat output(nRows, nCols);
    arma::mat frame1(nRows, nCols);
    arma::mat frame2(nRows, nCols);
    arma::mat frame1_00(nRows, nCols);
    arma::mat frame2_00(nRows, nCols);
    arma::cx_mat cc_mat(n_rows_new,n_cols_new);
    frame1 = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[0]),nCols,nRows).t(); 
    frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame1.t().as_col()));
    for (int framei = 1; framei < num_video_frames; framei++){
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
        frame1 = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei-1]),nCols,nRows).t();  
        frame1_00 = frame1;
        frame1 = frame1 - arma::mean(frame1.as_col());
        i_max = frame1.index_max();
		peak_index = arma::ind2sub(arma::size(frame1), i_max);
        yOffset0 = nRows2 - peak_index(0);
        xOffset0 = nCols2 - peak_index(1);
        double d = sqrt(pow(yOffset0,2) + pow(xOffset0,2));
        if (d >1.5){
            frame1 = arma::shift(arma::shift(frame1,yOffset0,0),xOffset0,1);
        }
        frame2 = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();
        frame2_00 = frame2;
        frame2 = frame2 - arma::mean(frame2.as_col());
        int ii = 0;
        arma::umat test1 = (arma::abs(frame1) > (N*arma::stddev(frame1.as_col())));
        arma::umat test2 = (arma::abs(frame2) > (N*arma::stddev(frame2.as_col())));
        int test1a = arma::sum(test1.as_col());
        int test2a = arma::sum(test2.as_col());
        while (ii<20 && (test1a < 10 && test2a < 10)) {
            test1 = arma::abs(frame1) > (N - (ds*ii))*arma::stddev(frame1.as_col());
            test2 = arma::abs(frame2) > (N - (ds*ii))*arma::stddev(frame2.as_col());
            test1a = arma::sum(test1.as_col());
            test2a = arma::sum(test2.as_col());
            ii += 1;
        }
        frame1.elem(arma::find(test1==0)).zeros();
        frame2.elem(arma::find(test2==0)).zeros();
        cc_mat = ImageProcessing::xcorr2(frame1,frame2,n_rows_new,n_cols_new, framei);
        i_max = cc_mat.index_max();
		peak_index = arma::ind2sub(arma::size(cc_mat), i_max);
        yOffset = (peak_index(0) < n_rows_new2)*(peak_index(0) + 1) - (peak_index(0) > n_rows_new2)*(n_rows_new - peak_index(0) - 1);
        xOffset = (peak_index(1) < n_cols_new2)*peak_index(1) - (peak_index(1) > n_cols_new2)*(n_cols_new - peak_index(1) - 1);
        output = frame2_00;
        if (framei == 0){
            output = frame1_00;
        }
        d = sqrt(pow(xOffset,2) + pow(yOffset,2));
        // if(d < 35 && d >1.5){
            output = arma::shift(arma::shift(frame2_00,yOffset,0),xOffset,1);
        // }
        brightest_centered_offsets.push_back({framei,-xOffset,-yOffset});
        output = output - arma::min(output.as_col());
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
    }
    return frames_out;     
}

 arma::cx_mat ImageProcessing::xcorr2(arma::mat inFrame1, arma::mat inFrame2, int nRows, int nCols, int framei)
{
    int N = 12;
    double ds = .25;
    int kk = 0;
    arma::umat test1 = (arma::abs(inFrame1) > (N*arma::stddev(inFrame1.as_col())));
    arma::umat test2 = (arma::abs(inFrame2) > (N*arma::stddev(inFrame2.as_col())));
    int test1a = arma::sum(test1.as_col());
    int test2a = arma::sum(test2.as_col());
    while(kk <46 && (test1a < 10 && test2a < 10) ){
        kk +=1;
        test1 = (arma::abs(inFrame1) > ((N-kk*ds)*arma::stddev(inFrame1.as_col())));
        test2 = (arma::abs(inFrame2) > ((N-kk*ds)*arma::stddev(inFrame2.as_col())));
        test1a = arma::sum(test1.as_col());
        test2a = arma::sum(test2.as_col());
    }
    inFrame1.elem(arma::find(test1==0)).zeros();
    inFrame2.elem(arma::find(test2==0)).zeros();
    arma::mat inFrame1_pad(nRows,nCols);
    arma::mat inFrame2_pad(nRows,nCols);
    inFrame1_pad.zeros();
    inFrame2_pad.zeros();
    inFrame1_pad(0,0,arma::size(inFrame1)) = inFrame1;
    inFrame2_pad(0,0,arma::size(inFrame2)) = inFrame2;;
    arma::cx_mat FG = arma::fft2(inFrame1_pad) % arma::fft2(arma::flipud(arma::fliplr(inFrame2_pad)));
    arma::cx_mat cc_mat = arma::ifft2(FG);
	return cc_mat;
}

void ImageProcessing::UpdateProgressBar(unsigned int val) {

    emit SignalProgress(val);
}
