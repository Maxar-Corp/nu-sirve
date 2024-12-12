#include "auto_tracking.h"

AutoTracking::AutoTracking()
{
    cancel_operation = false;
}

AutoTracking::~AutoTracking()
{
}

void AutoTracking::UpdateProgressBar(unsigned int val)
{
    emit signalProgress(val);
}

void AutoTracking::CancelOperation()
{
    cancel_operation = true;
}

// leverage OpenCV to track objects of interest
arma::u64_mat AutoTracking::SingleTracker(u_int track_id, double clamp_low, double clamp_high, int threshold, string prefilter, string trackFeature, uint frame0, int start_frame, int stop_frame, VideoDetails current_processing_state, VideoDetails base_processing_state, QString new_track_file_name)
{
    double irradiance;

    int nrows = current_processing_state.y_pixels;
    int ncols = current_processing_state.x_pixels;

    cv::Scalar filtered_meani, filtered_stdi, frame_crop_mean;
    cv::Mat frame_0_matrix, frame_i_matrix, processed_frame_0_matrix, processed_frame_i_matrix, filtered_frame_0_matrix, filtered_frame_i_matrix, frame_matrix_filtered_8bit, filtered_frame_0_matrix_8bit_color, filtered_frame_i_matrix_8bit_color;
    cv::Mat frame_0_crop, frame_i_crop, filtered_frame_0_matrix_8bit_color_resize, filtered_frame_i_matrix_8bit_color_resize;
    arma::vec frame_0_vector, frame_i_vector;

    GetProcessedFrameMatrix(start_frame, clamp_low, clamp_high, current_processing_state, frame_0_vector, frame_0_matrix, processed_frame_0_matrix);

    // attenuate image noise of initial frame
    filtered_frame_0_matrix = processed_frame_0_matrix;
    FilterImage(prefilter, processed_frame_0_matrix, filtered_frame_0_matrix);

    cv::cvtColor(filtered_frame_0_matrix, filtered_frame_0_matrix_8bit_color,cv::COLOR_GRAY2RGB);

    Ptr<Tracker> tracker = TrackerMIL::create();

    cv::startWindowThread();
    cv::resize(filtered_frame_0_matrix_8bit_color, filtered_frame_0_matrix_8bit_color_resize, cv::Size(N*ncols, N*nrows));
    string ROI_window_name = "Region of Interest (ROI) Selection - Press Escape twice to Cancel, or Select ROI then Hit Enter twice to Continue.";
    cv::Rect ROI = cv::selectROI(ROI_window_name, filtered_frame_0_matrix_8bit_color_resize);
    while (true) {
        int key = cv::pollKey();
        if (key == 13 && !ROI.empty()){ //Enter ket
            cv::destroyWindow(ROI_window_name);
            break;
        }
        else if(key == 27) { // Check for Esc key press
            ROI.width = 0;
            ROI.height = 0;
            cv::destroyAllWindows();
            QtHelpers::LaunchMessageBox("Canceled", "ROI selection canceled. Exiting without saving.");
            return arma::u64_mat ();
        } else if (cv::getWindowProperty(ROI_window_name, cv::WND_PROP_VISIBLE) < 1) {
            // Window has been closed
            ROI.width = 0;
            ROI.height = 0;
            break;
        }
    }
    
    if (ROI.width == 0 || ROI.height == 0)
    {
        QtHelpers::LaunchMessageBox("Invalid ROI", "There was an error in the ROI selection. Exiting without saving.");
        cv::destroyAllWindows();
        return arma::u64_mat ();
    }
    ROI.x /= N;
    ROI.y /= N;
    ROI.width /= N;
    ROI.height /= N;

    frame_0_crop = frame_0_matrix(ROI);

    cv::Point frame_0_point, frame_i_point;
    double peak_counts_0, peak_counts_old, peak_counts_i;
    peak_counts_old = 0;
    cv::Scalar sum_counts_0, sum_ROI_counts_0, sum_counts_i, sum_ROI_counts_i;
    uint N_threshold_pixels_0, N_ROI_pixels_0, N_threshold_pixels_i, N_ROI_pixels_i;
    GetTrackFeatureData(trackFeature, threshold, frame_0_crop, frame_0_point, frame_crop_mean, peak_counts_0, sum_counts_0, sum_ROI_counts_0, N_threshold_pixels_0, N_ROI_pixels_0);
    peak_counts_i = peak_counts_old;

    u_int frame_0_x, frame_0_y, frame_i_x, frame_i_y;
    u_int indx, num_frames = stop_frame - start_frame + 1;
    arma::u64_mat output(num_frames, 14);

    GetPointXY(frame_0_point, ROI,frame_0_x,frame_0_y);

    irradiance = ComputeIrradiance(start_frame, ROI, base_processing_state);          

    output.row(0) = {track_id, frame0, frame_0_x, frame_0_y, static_cast<uint16_t>(peak_counts_0),\
     static_cast<uint32_t>(sum_counts_0[0]), static_cast<uint32_t>(sum_ROI_counts_0[0]), N_threshold_pixels_0, N_ROI_pixels_0, static_cast<uint64_t>(irradiance),\
     static_cast<uint16_t>(ROI.x),static_cast<uint16_t>(ROI.y),static_cast<uint16_t>(ROI.width),static_cast<uint16_t>(ROI.height)};

    tracker->init(filtered_frame_0_matrix_8bit_color,ROI);

    for (u_int i = 1; i < num_frames; i++)
    {

        if (cancel_operation)
        {
            cv::destroyAllWindows();
            return arma::u64_mat ();
        }

        UpdateProgressBar(i);
        indx = (start_frame + i);
        string window_name_i = "Tracking... ";
        GetProcessedFrameMatrix(indx, clamp_low, clamp_high, current_processing_state, frame_i_vector, frame_i_matrix, processed_frame_i_matrix);

        filtered_frame_i_matrix = processed_frame_i_matrix;
        FilterImage(prefilter, processed_frame_i_matrix, filtered_frame_i_matrix);

        cv::cvtColor(filtered_frame_i_matrix, filtered_frame_i_matrix_8bit_color,cv::COLOR_GRAY2RGB);

        cv::resize(filtered_frame_i_matrix_8bit_color, filtered_frame_i_matrix_8bit_color_resize, cv::Size(N*ncols, N*nrows));
        bool ok = tracker->update(filtered_frame_i_matrix_8bit_color, ROI);
        frame_i_crop = frame_i_matrix(ROI);
   
        GetTrackFeatureData(trackFeature, threshold, frame_i_crop, frame_i_point, frame_crop_mean, peak_counts_i, sum_counts_i, sum_ROI_counts_i, N_threshold_pixels_i, N_ROI_pixels_i);

        if (ok && peak_counts_i >= tracking_peak_success_threshold * peak_counts_old)
        {
            peak_counts_old = peak_counts_i;
            rectangle(filtered_frame_i_matrix_8bit_color, ROI, cv::Scalar( 0, 0, 255 ), 2);
            imshow(window_name_i, filtered_frame_i_matrix_8bit_color);
        }
        else
        {
            cv::destroyAllWindows();
            QDialog dialog;
            dialog.setWindowTitle("Track Lost");

            QMessageBox::StandardButton response = QMessageBox::question(&dialog, "Track Lost... ", "Save available track data and exit, try to continue tracking, or discard and exit?",
                                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Retry);
            
            if (response == QMessageBox::Save)
            {
                if (i>1)
                {
                    output.shed_rows(i,num_frames-1);
                    return output;
                }
                else
                {
                    QtHelpers::LaunchMessageBox("Empty Track", "The track is empty and nothing was saved.");
                    return arma::u64_mat ();
                }
            }
            else if (response == QMessageBox::Discard)
                {
                    return arma::u64_mat ();
                }
            else
            {
                window_name_i = "Track Lost. " + std::to_string(indx) + " Select ROI again.";
                ROI = selectROI(window_name_i, filtered_frame_i_matrix_8bit_color_resize);

                while (true) {
                    int key = cv::pollKey();
                    if (key == 13 && !ROI.empty()){ //Enter ket
                        cv::destroyWindow(window_name_i);
                        break;
                    }
                    else if(key == 27) { // Check for Esc key press
                        ROI.width = 0;
                        ROI.height = 0;
                        cv::destroyAllWindows();
                        QtHelpers::LaunchMessageBox("Canceled", "ROI selection canceled. Exiting without saving.");
                        return arma::u64_mat ();
                    } else if (cv::getWindowProperty(window_name_i, cv::WND_PROP_VISIBLE) < 1) {
                        // Window has been closed
                        ROI.width = 0;
                        ROI.height = 0;
                        break;
                    }
                }
    
                if (ROI.width == 0 || ROI.height == 0)
                {
                    QtHelpers::LaunchMessageBox("Invalid ROI", "There was an error in the ROI selection. Exiting without saving.");
                    cv::destroyAllWindows();
                    return arma::u64_mat ();
                }

                ROI.x /= N;
                ROI.y /= N;
                ROI.width /= N;
                ROI.height /= N;
                tracker->init(filtered_frame_i_matrix_8bit_color, ROI);
                frame_i_crop = frame_i_matrix(ROI);
               
                GetTrackFeatureData(trackFeature, threshold, frame_i_crop, frame_i_point, frame_crop_mean, peak_counts_i, sum_counts_i, sum_ROI_counts_i, N_threshold_pixels_i, N_ROI_pixels_i);
                peak_counts_old = peak_counts_i;
            }
        }

        GetPointXY(frame_i_point, ROI, frame_i_x, frame_i_y);
        irradiance = ComputeIrradiance(indx, ROI, base_processing_state);   

        output.row(i) = {track_id, frame0 + i, frame_i_x ,frame_i_y, static_cast<uint16_t>(peak_counts_i),\
         static_cast<uint32_t>(sum_counts_i[0]),static_cast<uint32_t>(sum_ROI_counts_i[0]),N_threshold_pixels_i,N_ROI_pixels_i, static_cast<uint64_t>(irradiance),\
         static_cast<uint16_t>(ROI.x),static_cast<uint16_t>(ROI.y),static_cast<uint16_t>(ROI.width),static_cast<uint16_t>(ROI.height)};
        waitKey(1);
    }
    cv::destroyAllWindows();
    return output;
}

void AutoTracking::FilterImage(string prefilter, cv::Mat & processed_frame_0_matrix, cv::Mat & filtered_frame_0_matrix)
{
  if(prefilter=="GAUSSIAN"){
        cv::GaussianBlur(processed_frame_0_matrix, filtered_frame_0_matrix, cv::Size(5,5), 0);
    }
    else if(prefilter=="MEDIAN"){
        cv::medianBlur(processed_frame_0_matrix, filtered_frame_0_matrix, 5);      
    }
    else if(prefilter=="NLMEANS"){
        cv::fastNlMeansDenoising(processed_frame_0_matrix, filtered_frame_0_matrix);
    }
}

void  AutoTracking::GetTrackFeatureData(string trackFeature, int threshold, cv::Mat frame_crop, cv::Point & frame_point, cv::Scalar frame_crop_mean,\
  double & peak_counts, cv::Scalar & sum_counts, cv::Scalar & sum_ROI_counts, uint & N_threshold_pixels,  uint & N_ROI_pixels)
{
    cv::Mat frame_crop_threshold;
    cv::Scalar frame_crop_sigma;
    cv::meanStdDev(frame_crop, frame_crop_mean, frame_crop_sigma);
    sum_ROI_counts = cv::sum(frame_crop);
    N_ROI_pixels = cv::countNonZero(frame_crop > 0);
    if(trackFeature == "INTENSITY_WEIGHTED_CENTROID"){
        cv::threshold(frame_crop, frame_crop_threshold, frame_crop_mean[0]+threshold*frame_crop_sigma[0], NULL, cv::THRESH_TOZERO);
        sum_counts = cv::sum(frame_crop_threshold);
        N_threshold_pixels = cv::countNonZero(frame_crop_threshold > 0);
        cv::Moments frame_moments = cv::moments(frame_crop_threshold,false);
        cv::Point frame_temp_point(frame_moments.m10/frame_moments.m00, frame_moments.m01/frame_moments.m00);
        cv::minMaxLoc(frame_crop_threshold, NULL, &peak_counts, NULL, NULL);
        frame_point = frame_temp_point;
    }
    else if (trackFeature == "CENTROID"){
        cv::threshold(frame_crop, frame_crop_threshold, frame_crop_mean[0]+threshold*frame_crop_sigma[0], 1, cv::THRESH_BINARY);
        cv::Mat result;
        cv::multiply(frame_crop_threshold,frame_crop,result);
        sum_counts = cv::sum(result);
        N_threshold_pixels = cv::countNonZero(frame_crop_threshold > 0);
        cv::Moments frame_moments = cv::moments(frame_crop_threshold,true);
        cv::Point frame_temp_point(frame_moments.m10/frame_moments.m00, frame_moments.m01/frame_moments.m00);
        cv::minMaxLoc(result, NULL, &peak_counts, NULL, NULL);
        frame_point = frame_temp_point;
    }
    else{
        cv::Point frame_temp_point;
        cv::threshold(frame_crop, frame_crop_threshold, frame_crop_mean[0]+threshold*frame_crop_sigma[0], NULL, cv::THRESH_TOZERO);
        sum_counts = cv::sum(frame_crop_threshold);
        N_threshold_pixels = cv::countNonZero(frame_crop_threshold > 0);
        cv::minMaxLoc(frame_crop_threshold, NULL, &peak_counts, NULL, &frame_temp_point);
        frame_point = frame_temp_point;
    }

}

void AutoTracking::GetPointXY(cv::Point input_point, cv::Rect ROI, u_int & centerX, u_int & centerY)
{
    if (input_point.x > 0 && input_point.y > 0){
        centerX = round(input_point.x + ROI.x);
        centerY = round(input_point.y + ROI.y);
    }
    else
    {
        centerX = round(ROI.x + 0.5 * ROI.width);
        centerY = round(ROI.y + 0.5 * ROI.height);
    }
}

void AutoTracking::GetProcessedFrameMatrix(int indx, double clamp_low, double clamp_high, VideoDetails current_processing_state, arma::vec & frame_vector, cv::Mat & frame_matrix, cv::Mat & processed_frame_matrix)
{
    double m, s;
    int nrows = current_processing_state.y_pixels;
    int ncols = current_processing_state.x_pixels;      
    frame_vector = arma::conv_to<arma::vec>::from(current_processing_state.frames_16bit[indx]);
    arma::vec processed_frame_vector = frame_vector;
    m = arma::mean(frame_vector);
    s = arma::stddev(frame_vector);
    processed_frame_vector.clamp(m - clamp_low*s, m + clamp_high*s);
    processed_frame_vector = processed_frame_vector - processed_frame_vector.min();
    processed_frame_vector = 255*processed_frame_vector/processed_frame_vector.max();
    processed_frame_matrix = cv::Mat(nrows, ncols, CV_64FC1, processed_frame_vector.memptr());
    processed_frame_matrix.convertTo(processed_frame_matrix, CV_8UC1);
    frame_matrix = cv::Mat(nrows, ncols, CV_64FC1, frame_vector.memptr());
}

double AutoTracking::ComputeIrradiance(int indx, cv::Rect ROI, VideoDetails base_processing_state)
{
    double irradiance_val;
    // std::cout << "Indx: "<< indx << std::endl;
    
    int start_indx, stop_indx;
    start_indx = std::max(indx-number_median_frames,0);
    stop_indx = std::max(indx-1,0);

    // std::cout << "start_indx, stop_indx: "<< start_indx << " "<< stop_indx << std::endl;

    int nRows = base_processing_state.y_pixels;
    int nCols = base_processing_state.x_pixels;
    int nFrames = base_processing_state.frames_16bit.size();

    arma::cube data_cube(nRows,nCols,nFrames);

    for (unsigned int k = 0; k < nFrames; ++k) {
        data_cube.slice(k) = arma::reshape(arma::conv_to<arma::vec>::from(base_processing_state.frames_16bit[k]),nCols,nRows).t();   
    }

    int row1 = ROI.y;
    int row2 = ROI.y + ROI.height;
    int col1 = ROI.x;
    int col2 = ROI.x + ROI.width;

    // std::cout << "row1, row2, col1, col2: "<< row1 << " "<< row2 <<" "<< col1 << " "<< col2 << std::endl;

    arma::cube data_subcube = data_cube.subcube(row1,col1,start_indx,row2,col2,stop_indx);
    arma::mat data_subcube_mean = arma::mean(data_subcube,2);
    arma::mat current_frame = data_cube.slice(indx);
    arma::mat current_subframe = current_frame.submat(row1,col1,row2,col2);
    // std::cout << "Current Subframe:" << std::endl;
    // current_subframe.print();
    arma::mat counts_minus_mean =  current_subframe - data_subcube_mean;
    // std::cout << "counts_minus_mean:" << std::endl;
    // counts_minus_mean.print();
    irradiance_val = arma::sum(counts_minus_mean.as_col());
    return irradiance_val;

}
