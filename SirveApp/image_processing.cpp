#include "image_processing.h"
ImageProcessing::ImageProcessing()
{
    frameval = 0;

    disk_avg_kernel = {
            {0.0, 0.0, 0.0, 0.0012, 0.0050, 0.0063, 0.0050, 0.0012, 0.0, 0.0, 0.0},
            {0.0, 0.0000, 0.0062, 0.0124, 0.0127, 0.0127, 0.0127, 0.0124, 0.0062, 0.0000, 0.0},
            {0.0, 0.0062, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0062, 0.0},
            {0.0012, 0.0124, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0124, 0.0012},
            {0.0050, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0050},
            {0.0063, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0063},
            {0.0050, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0050},
            {0.0012, 0.0124, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0124, 0.0012},
            {0.0, 0.0062, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0127, 0.0062, 0.0},
            {0.0, 0.0000, 0.0062, 0.0124, 0.0127, 0.0127, 0.0127, 0.0124, 0.0062, 0.0000, 0.0},
            {0.0, 0.0, 0.0, 0.0012, 0.0050, 0.0063, 0.0050, 0.0012, 0.0, 0.0, 0.0}
        };
}

ImageProcessing::~ImageProcessing() {
}

std::vector<std::vector<uint16_t>> ImageProcessing::FixedNoiseSuppression(QString image_path, QString path_video_file, int start_frame, int end_frame, double version, VideoDetails & original)
{
	// Initialize output
	std::vector<std::vector<uint16_t>> frames_out;

	int num_video_frames = original.frames_16bit.size();
	int number_avg_frames;
	int index_first_frame, index_last_frame;
	index_first_frame = start_frame - 1;

	ABIRDataResult abir_result;
	int compare = QString::compare(path_video_file, image_path, Qt::CaseInsensitive);
	if (compare!=0){ 
			QByteArray array = image_path.toLocal8Bit();
			char* buffer = array.data();
			abir_result = abir_data.GetFrames(buffer, start_frame, end_frame, version, false);
			if (abir_result.had_error) {
				return frames_out;
			}
		 number_avg_frames = abir_result.video_frames_16bit.size();
	}
	else{
		abir_result.video_frames_16bit = original.frames_16bit;
		number_avg_frames = end_frame - start_frame + 1;
	}
	index_last_frame = index_first_frame + number_avg_frames - 1;
	int num_pixels = abir_result.video_frames_16bit[0].size();
	// Create an Armadillo matrix for submatrix average
    arma::mat window_data(num_pixels, number_avg_frames);

    // Fill the Armadillo matrix from the std::vector
	if (compare!=0){ 
		for (int i = 0; i < number_avg_frames; i++){
			window_data.col(i) = arma::conv_to<arma::vec>::from(abir_result.video_frames_16bit[i]);
		}
	}
	else{
		int k = 0;
		for (int i = index_first_frame; i < index_last_frame; i++){
			window_data.col(k) = arma::conv_to<arma::vec>::from(abir_result.video_frames_16bit[i]);
			k += 1;
		}
    }
	// Take the mean of each row
	arma::vec mean_frame = arma::mean(window_data, 1);
	double M;
	arma::vec frame_vector(num_pixels, 1);
	//Loop through frames to subtract mean
	for (int i = 0; i < num_video_frames; i++){
		UpdateProgressBar(i);
		QCoreApplication::processEvents();
		frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
		M = frame_vector.max();
		frame_vector -= mean_frame;
		frame_vector -= frame_vector.min();
		frame_vector = M * frame_vector / frame_vector.max();
		frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector));
    }
	return frames_out;
}

std::vector<std::vector<uint16_t>> ImageProcessing::AdaptiveNoiseSuppressionByFrame(int start_frame, int num_of_averaging_frames, int NThresh, VideoDetails & original,  QString & hide_shadow_choice)
{

	int num_video_frames = original.frames_16bit.size();
	int num_pixels = original.frames_16bit[0].size();
	int nRows = original.y_pixels;
    int nCols = original.x_pixels;
	int index_first_frame, index_last_frame, index_frame, abs_start_frame;
	double M;
	abs_start_frame = std::abs(start_frame);
	std::vector<std::vector<uint16_t>> frames_out;
  	arma::mat window_data(num_pixels,num_of_averaging_frames);
	arma::mat adjusted_window_data(num_pixels,1);
	adjusted_window_data.fill(0.0);
	arma::vec moving_mean(num_pixels, 1);
	arma::vec frame_vector(num_pixels,1);
	arma::vec frame_vector_out(num_pixels,1);

	for (int j = 0; j < num_of_averaging_frames - 1; j++) { 
		index_frame = std::max(start_frame + j,0);
		window_data.col(j) = arma::conv_to<arma::vec>::from(original.frames_16bit[index_frame]);
	}
    for (int i = 0; i < num_video_frames; i++) {
	 	UpdateProgressBar(i);
		QCoreApplication::processEvents();
		frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);

		index_first_frame = std::max(i + start_frame,0);
        index_last_frame = std::min(index_first_frame + num_of_averaging_frames,num_video_frames - 1);
		window_data.insert_cols(window_data.n_cols,arma::conv_to<arma::vec>::from(original.frames_16bit[index_last_frame]));
		window_data.shed_col(0);

		moving_mean = arma::mean(window_data,1);

		M = frame_vector.max();
		frame_vector -= moving_mean;

		if (hide_shadow_choice == "Hide Shadow"){
			ImageProcessing::remove_shadow(nRows, nCols, frame_vector, adjusted_window_data, NThresh, num_of_averaging_frames);
			if(i == 0){
				adjusted_window_data.insert_cols(0,frame_vector);
				adjusted_window_data.shed_col(0);
			}
			else{
				adjusted_window_data.insert_cols(adjusted_window_data.n_cols,frame_vector);
				if(adjusted_window_data.n_cols>abs_start_frame){
					adjusted_window_data.shed_col(0);	
				}		
			}
		}
		else
		{
			frame_vector -= frame_vector.min();
		}

		frame_vector_out = M * frame_vector / frame_vector.max();
		frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector_out));
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
    arma::uword i_max, i_max_even, i_max_odd;
    arma::uvec peak_index,peak_index_even,peak_index_odd;
    int yOffset, xOffset;

    for (int framei = 0; framei < num_video_frames; framei++){
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
        frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();
        frame0 = frame;
        output = frame0;
        frame = frame - arma::mean(frame.as_col());
        odd_frame = frame.rows(odd_rows);
	    even_frame = frame.rows(even_rows);
        i_max_even = even_frame.index_max();  
        i_max_odd = odd_frame.index_max(); 
        peak_index_even = arma::ind2sub(arma::size(even_frame),i_max_even); 
        peak_index_odd = arma::ind2sub(arma::size(even_frame),i_max_odd);
        if(sqrt(pow(peak_index_even(0)-peak_index_odd(0),2)+pow(peak_index_even(1)-peak_index_odd(1),2))>=4){
            even_frame0 = frame0.rows(even_rows);
            cc_mat = ImageProcessing::xcorr2(odd_frame,even_frame,n_rows_new,n_cols_new);
            i_max = cc_mat.index_max();
            peak_index = arma::ind2sub(arma::size(cc_mat), i_max);
            yOffset = (peak_index(0) < n_rows_new2)*(peak_index(0) + 1) - (peak_index(0) > n_rows_new2)*(n_rows_new - peak_index(0) - 1);
            xOffset = (peak_index(1) < n_cols_new2)*peak_index(1) - (peak_index(1) > n_cols_new2)*(n_cols_new - peak_index(1) - 1);
            double d = sqrt(pow(xOffset,2) + pow(yOffset,2));
            if(d < 35 && d >1.5){
                output.rows(even_rows) = arma::shift(arma::shift(even_frame0,yOffset,0),xOffset,1);
            }
            output = output - arma::min(output.as_col());
        }
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
    }
    return frames_out;
}


std::vector<std::vector<uint16_t>> ImageProcessing::CenterOnTracks(QString trackTypePriority, VideoDetails & original, int track_id, std::vector<TrackFrame> osmFrames, std::vector<TrackFrame> manualFrames, boolean findAnyTrack, std::vector<std::vector<int>> & track_centered_offsets)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels, yOffset_correction = nRows/2;
    int nCols = original.x_pixels, xOffset_correction = nCols/2;
    int yOffset, xOffset;
    int OSMPriority = QString::compare(trackTypePriority,"OSM",Qt::CaseInsensitive);
    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
    std::vector<TrackFrame> trackFrames = manualFrames;
    

    if (track_id>0){
        for (int framei = 0; framei < num_video_frames; framei++){
            UpdateProgressBar(framei);
            QCoreApplication::processEvents();
            frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();  
            if (OSMPriority==0){
                trackFrames = osmFrames;
                yOffset_correction = 0;
                xOffset_correction = 0;
            }
            if (trackFrames[framei].tracks.find(track_id) != trackFrames[framei].tracks.end()) {      
                yOffset = trackFrames[framei].tracks[track_id].centroid_y - yOffset_correction;
                xOffset = trackFrames[framei].tracks[track_id].centroid_x - xOffset_correction;
                output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
                frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
                track_centered_offsets.push_back({framei,xOffset,yOffset});
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
            if(OSMPriority==0){
                if(osmFrames[framei].tracks.size()>0){
                    while (cont_search && i < osmFrames[framei].tracks.size()){
                        if (osmFrames[framei].tracks[i].centroid_x != NULL){
                            yOffset = osmFrames[framei].tracks[i].centroid_y;
                            xOffset = osmFrames[framei].tracks[i].centroid_x;
                            output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
                            track_centered_offsets.push_back({framei,xOffset,yOffset});
                            cont_search = false;
                        }
                        else{
                            i+=1;
                        }
                    }
                }
                if(cont_search && manualFrames[framei].tracks.size()>0 && findAnyTrack){
                    i = 0;
                    while (cont_search && i < manualFrames[framei].tracks.size()){
                        if (manualFrames[framei].tracks[i].centroid_x != NULL){
                            yOffset = manualFrames[framei].tracks[i].centroid_y - yOffset_correction;
                            xOffset = manualFrames[framei].tracks[i].centroid_x - xOffset_correction;
                            output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
                            track_centered_offsets.push_back({framei,xOffset,yOffset});
                            cont_search = false;
                        }
                        else{
                            i+=1;
                        }
                    }
                }
            }
            else{
               if(manualFrames[framei].tracks.size()>0){
                    while (cont_search && i < manualFrames[framei].tracks.size()){
                        if (manualFrames[framei].tracks[i].centroid_x != NULL){
                            yOffset = manualFrames[framei].tracks[i].centroid_y - yOffset_correction;
                            xOffset = manualFrames[framei].tracks[i].centroid_x - xOffset_correction;
                            output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
                            track_centered_offsets.push_back({framei,xOffset,yOffset});
                            cont_search = false;
                        }
                        else{
                            i+=1;
                        }
                    }
                }
                if(cont_search && osmFrames[framei].tracks.size()>0 && findAnyTrack){
                    i = 0;
                    while (cont_search && i < osmFrames[framei].tracks.size()){
                        if (osmFrames[framei].tracks[i].centroid_x != NULL){
                            yOffset = osmFrames[framei].tracks[i].centroid_y;
                            xOffset = osmFrames[framei].tracks[i].centroid_x;
                            output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
                            track_centered_offsets.push_back({framei,xOffset,yOffset});
                            cont_search = false;
                        }
                        else{
                            i+=1;
                        }
                    }
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
    int yOffset0, xOffset0, i_max;
    arma::uvec peak_index; 
    arma::mat output(nRows, nCols);
    arma::mat frame1(nRows, nCols);
    for (int framei = 0; framei < num_video_frames; framei++){
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
        frame1 = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();  
        frame1 = frame1 - arma::mean(frame1.as_col());
        i_max = frame1.index_max();
		peak_index = arma::ind2sub(arma::size(frame1), i_max);
        yOffset0 = nRows2 - peak_index(0);
        xOffset0 = nCols2 - peak_index(1);
        double d = sqrt(pow(yOffset0,2) + pow(xOffset0,2));
        if (d >1.5){
            frame1 = arma::shift(arma::shift(frame1,yOffset0,0),xOffset0,1);
        } 
        brightest_centered_offsets.push_back({framei,-xOffset0,-yOffset0});
        output = frame1 - arma::min(frame1.as_col());
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
    }
    return frames_out;     
}


 std::vector<std::vector<uint16_t>> ImageProcessing::FrameStacking(int num_of_averaging_frames, VideoDetails & original)
 {
    int num_video_frames = original.frames_16bit.size();
	int num_pixels = original.frames_16bit[0].size();
	int nRows = original.y_pixels;
    int nCols = original.x_pixels;
	int index_first_frame, index_last_frame;
	double R;
	std::vector<std::vector<uint16_t>> frames_out;
  	arma::mat window_data(num_pixels,num_of_averaging_frames);
    window_data.fill(0.0);
	arma::vec moving_mean(num_pixels, 1);
	arma::vec frame_vector(num_pixels,1);
	arma::vec frame_vector_out(num_pixels,1);

	for (int j = 0; j < num_of_averaging_frames - 1; j++) { 
		window_data.col(j) = arma::conv_to<arma::vec>::from(original.frames_16bit[j]);
	}
    for (int i = 0; i < num_video_frames; i++) {
	 	UpdateProgressBar(i);
		QCoreApplication::processEvents();
		frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
        index_last_frame = std::min(i + num_of_averaging_frames - 1,num_video_frames - 1);
        if(i >num_of_averaging_frames){
		    window_data.insert_cols(window_data.n_cols,arma::conv_to<arma::vec>::from(original.frames_16bit[index_last_frame]));
		    window_data.shed_col(0);
        }
		moving_mean = arma::mean(window_data,1);
        R = arma::range(frame_vector);
		frame_vector -= moving_mean;
        frame_vector -= frame_vector.min();
        frame_vector_out = R * frame_vector / frame_vector.max();
		frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector_out));
    }
	return frames_out;
 }

 arma::cx_mat ImageProcessing::xcorr2(arma::mat inFrame1, arma::mat inFrame2, int nRows, int nCols)
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
    inFrame2_pad(0,0,arma::size(inFrame2)) = inFrame2;
    arma::cx_mat FG = arma::fft2(inFrame1_pad) % arma::fft2(arma::flipud(arma::fliplr(inFrame2_pad)));
    arma::cx_mat cc_mat = arma::ifft2(FG);
	return cc_mat;
}

void ImageProcessing::remove_shadow(int nRows, int nCols, arma::vec & frame_vector, arma::mat window_data, int NThresh, int num_of_averaging_frames)
{	
	int n_rows_new = pow(2, ceil(log(nRows)/log(2)));  
    int n_cols_new = pow(2, ceil(log(nCols)/log(2)));
	arma::mat frame_matrix = arma::reshape(frame_vector,nCols,nRows).t();
	// arma::mat frame_matrix_blurred = arma::conv2(frame_matrix,kernel,"same");
	// arma::cx_mat frame_matrix_blurred = AdaptiveNoiseSuppression::conv2fft(frame_matrix,kernel,n_rows_new,n_cols_new);
	arma::mat frame_matrix_blurred = frame_matrix;
	frame_matrix_blurred -= arma::mean(frame_matrix_blurred.as_col());
	arma::uvec index_negative = arma::find(frame_matrix_blurred.t() < -NThresh*arma::stddev(frame_matrix_blurred.as_col()));
	// arma::uvec index_negative = arma::find(arma::real(frame_matrix_blurred.t()) < -NThresh*arma::stddev(frame_matrix_blurred.as_col()));
	arma::uvec index_change;
	arma::vec old_frame_sum;
	int numCols = window_data.n_cols;
	int maxNcols = std::min(num_of_averaging_frames-1,numCols-1);
	double MEAN, SIGMA;
	if(index_negative.n_elem>0){
		if(maxNcols>0){
			old_frame_sum = arma::sum(window_data.cols(0,maxNcols),1);
		}
		else{
			old_frame_sum = window_data.col(0);
		}
		arma::mat old_frame_sum_mat = arma::reshape(old_frame_sum,nCols,nRows).t();
		arma::mat old_frame_sum_mat_blurred = arma::conv2(old_frame_sum_mat,disk_avg_kernel,"same");
		// arma::cx_mat old_frame_sum_mat_blurred = AdaptiveNoiseSuppression::conv2fft(old_frame_sum_mat,kernel,n_rows_new,n_cols_new);
		// arma::mat old_frame_sum_mat_blurred = old_frame_sum_mat;
		old_frame_sum_mat_blurred -= arma::mean(old_frame_sum_mat_blurred.as_col());
		arma::uvec index_positive = arma::find(old_frame_sum_mat_blurred.t() > NThresh*arma::stddev(old_frame_sum_mat_blurred.as_col()));
		// arma::uvec index_positive = arma::find(arma::real(old_frame_sum_mat_blurred.t()) > NThresh*arma::stddev(old_frame_sum_mat_blurred.as_col()));
		index_change = arma::intersect(index_negative,index_positive);
		if(index_change.n_elem>0){
			arma::uvec index_other = arma::find(arma::abs(frame_vector) <= 3.*arma::stddev(frame_vector));
			if(index_other.n_elem>0){
				MEAN = arma::mean(frame_vector.elem(index_other));
				SIGMA = arma::stddev(frame_vector.elem(index_other));
			}
			else{
				MEAN = arma::mean(frame_vector);
				SIGMA = arma::stddev(frame_vector);
			}
			if (SIGMA!=0){	
				frame_vector.elem(index_change) = arma::randn<arma::vec>(index_change.size(),arma::distr_param(MEAN,SIGMA));
			}
		}
	}

	frame_vector -= frame_vector.min();
}

void ImageProcessing::UpdateProgressBar(unsigned int val) {

    emit SignalProgress(val);
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