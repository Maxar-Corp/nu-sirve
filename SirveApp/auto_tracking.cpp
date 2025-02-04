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
arma::u64_mat AutoTracking::SingleTracker(u_int track_id, double clamp_low, double clamp_high, int threshold, string prefilter, string trackFeature, uint frame0, int start_frame, int stop_frame,\
 processingState & current_processing_state, VideoDetails & base_processing_state_details, QString new_track_file_name)
{
    double irradiance;

    cv::Scalar filtered_meani, filtered_stdi, frame_crop_mean;
    cv::Mat frame_0_matrix, frame_i_matrix, processed_frame_0_matrix, processed_frame_i_matrix, filtered_frame_0_matrix, filtered_frame_i_matrix, frame_matrix_filtered_8bit, filtered_frame_0_matrix_8bit_color, filtered_frame_i_matrix_8bit_color;
    cv::Mat frame_0_crop, frame_i_crop, filtered_frame_0_matrix_8bit_color_resize, filtered_frame_i_matrix_8bit_color_resize;
    arma::vec frame_0_vector, frame_i_vector;
    arma::vec framei = arma::regspace(start_frame,stop_frame);
    arma::mat offset_matrix2(framei.n_elem,3,arma::fill::zeros);
    std::vector<std::vector<int>> offsets;
    u_int frame_0_x, frame_0_y, frame_i_x, frame_i_y;
    u_int indx, num_frames = stop_frame - start_frame + 1;
    arma::u64_mat output(num_frames, 14);
    arma::running_stat<double> stats;
    cv::Point frame_0_point, frame_i_point;
    double peak_counts_0, peak_counts_i, irradiance_counts_0, irradiance_counts_old, irradiance_counts_i;
    cv::Scalar sum_counts_0, sum_ROI_counts_0, sum_counts_i, sum_ROI_counts_i;

    GetProcessedFrameMatrix(start_frame, clamp_low, clamp_high, current_processing_state.details, frame_0_vector, frame_0_matrix, processed_frame_0_matrix);

    // attenuate image noise of initial frame
    filtered_frame_0_matrix = processed_frame_0_matrix;
    FilterImage(prefilter, processed_frame_0_matrix, filtered_frame_0_matrix);

    cv::cvtColor(filtered_frame_0_matrix, filtered_frame_0_matrix_8bit_color,cv::COLOR_GRAY2RGB);

    if (current_processing_state.offsets.size()>0){
        offsets = current_processing_state.offsets;
        arma::mat offset_matrix(offsets.size(),3,arma::fill::zeros);
        for (int rowi = 0; rowi < offsets.size(); rowi++){
            offset_matrix.row(rowi) = arma::conv_to<arma::rowvec>::from(offsets[rowi]);
        }
        for (int rowii = 0; rowii<framei.size(); rowii++){
            arma::uvec kk = arma::find(offset_matrix.col(0) == framei(rowii) + 1,0,"first");
            if (!kk.is_empty()){
                offset_matrix2.row(rowii) = offset_matrix.row(kk(0));
            }
        }
        offset_matrix2.shed_col(0);
    }

    uint N_threshold_pixels_0, N_ROI_pixels_0, N_threshold_pixels_i, N_ROI_pixels_i;

    cv::startWindowThread();

    Ptr<Tracker> tracker = TrackerMIL::create();

    cv::resize(filtered_frame_0_matrix_8bit_color, filtered_frame_0_matrix_8bit_color_resize, cv::Size(N*ncols, N*nrows));
    string ROI_window_name = "Region of Interest (ROI) Selection - Press Escape twice to Cancel, or Select ROI then Hit Enter twice to Continue.";
    cv::Rect ROI = cv::selectROI(ROI_window_name, filtered_frame_0_matrix_8bit_color_resize);

    while (true)
    {
        int key = cv::pollKey();
        if (key == 13 && !ROI.empty()){ //Enter key
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
    GetTrackFeatureData(trackFeature, threshold, frame_0_crop, frame_0_point, frame_crop_mean, peak_counts_0, sum_counts_0, sum_ROI_counts_0, N_threshold_pixels_0, N_ROI_pixels_0);
    GetPointXY(frame_0_point, ROI, frame_0_x, frame_0_y);
    irradiance_counts_0 = IrradianceCountsCalc::ComputeIrradiance(start_frame, ROI.height/2, ROI.width/2, frame_0_x + offset_matrix2(0,0), frame_0_y + offset_matrix2(0,1), base_processing_state_details);
    irradiance_counts_old = irradiance_counts_0;
    stats(irradiance_counts_old);
    tracker->init(filtered_frame_0_matrix_8bit_color,ROI);

    output.row(0) = {track_id, frame0, frame_0_x, frame_0_y, static_cast<uint16_t>(peak_counts_0),\
     static_cast<uint32_t>(sum_counts_0[0]), static_cast<uint32_t>(sum_ROI_counts_0[0]), N_threshold_pixels_0, N_ROI_pixels_0, static_cast<uint64_t>(irradiance_counts_0),\
     static_cast<uint16_t>(ROI.x),static_cast<uint16_t>(ROI.y),static_cast<uint16_t>(ROI.width),static_cast<uint16_t>(ROI.height)};

    bool step_sucess = false;
    int i = 1;

    string window_name_i = "Tracking... ";
 
    while (i < num_frames)
    {
        if (cancel_operation)
        {
            cv::destroyAllWindows();
            QDialog dialog;
            dialog.setWindowTitle("Tracking Paused");
            QMessageBox::StandardButton response = QMessageBox::question(&dialog, "Track Paused... ", "Save available track data and exit, try to continue tracking, or discard and exit?",
                                                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Retry);
                                                    
            HandleInterruption(response, i, indx, num_frames, output, ROI, filtered_frame_i_matrix_8bit_color);
            if ((response == QMessageBox::Save) || (response == QMessageBox::Discard) || output.n_elem<1)
            {
                return output;
            }

            ROI.x /= N;
            ROI.y /= N;
            ROI.width /= N;
            ROI.height /= N;
            frame_i_crop = frame_i_matrix(ROI);  
            GetTrackFeatureData(trackFeature, threshold, frame_i_crop, frame_i_point, frame_crop_mean, peak_counts_i, sum_counts_i, sum_ROI_counts_i, N_threshold_pixels_i, N_ROI_pixels_i);
            GetPointXY(frame_i_point, ROI, frame_i_x, frame_i_y);  
            irradiance_counts_i = IrradianceCountsCalc::ComputeIrradiance(indx, ROI.height/2, ROI.width/2, frame_i_x + offset_matrix2(i,0), frame_i_y + offset_matrix2(i,1), base_processing_state_details);       
            irradiance_counts_old = irradiance_counts_i;
            stats(irradiance_counts_old);
            tracker->init(filtered_frame_i_matrix_8bit_color, ROI);

            step_sucess = true;
            cancel_operation = false;
        }

        UpdateProgressBar(i);
        indx = (start_frame + i);

        GetProcessedFrameMatrix(indx, clamp_low, clamp_high, current_processing_state.details, frame_i_vector, frame_i_matrix, processed_frame_i_matrix);

        filtered_frame_i_matrix = processed_frame_i_matrix;
        FilterImage(prefilter, processed_frame_i_matrix, filtered_frame_i_matrix);

        cv::cvtColor(filtered_frame_i_matrix, filtered_frame_i_matrix_8bit_color,cv::COLOR_GRAY2RGB);

        bool ok = tracker->update(filtered_frame_i_matrix_8bit_color, ROI);

        frame_i_crop = frame_i_matrix(ROI);
   
        GetTrackFeatureData(trackFeature, threshold, frame_i_crop, frame_i_point, frame_crop_mean, peak_counts_i, sum_counts_i, sum_ROI_counts_i, N_threshold_pixels_i, N_ROI_pixels_i);
        GetPointXY(frame_i_point, ROI, frame_i_x, frame_i_y);
        irradiance_counts_i = IrradianceCountsCalc::ComputeIrradiance(indx, ROI.height/2, ROI.width/2, frame_i_x + offset_matrix2(i,0), frame_i_y+ offset_matrix2(i,1), base_processing_state_details);
        stats(irradiance_counts_old);
        double S = stats.stddev();
        step_sucess = (ok && abs((irradiance_counts_i - stats.mean())) <= 6*S);

        if (S>0 && !step_sucess)
        {
            cv::destroyAllWindows();
            QDialog dialog;
            dialog.setWindowTitle("Track Lost");

            QMessageBox::StandardButton response = QMessageBox::question(&dialog, "Track Lost... ", "Save available track data and exit, try to continue tracking, or discard and exit?",
                                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Retry);
            
            HandleInterruption(response, i, indx, num_frames, output, ROI, filtered_frame_i_matrix_8bit_color);

            if ((response == QMessageBox::Save) || (response == QMessageBox::Discard) || output.n_elem<1)
            {
                return output;
            }

            ROI.x /= N;
            ROI.y /= N;
            ROI.width /= N;
            ROI.height /= N;
            frame_i_crop = frame_i_matrix(ROI);  
            GetTrackFeatureData(trackFeature, threshold, frame_i_crop, frame_i_point, frame_crop_mean, peak_counts_i, sum_counts_i, sum_ROI_counts_i, N_threshold_pixels_i, N_ROI_pixels_i);
            GetPointXY(frame_i_point, ROI, frame_i_x, frame_i_y);  
            irradiance_counts_i = IrradianceCountsCalc::ComputeIrradiance(indx, ROI.height/2, ROI.width/2, frame_i_x + offset_matrix2(i,0), frame_i_y + offset_matrix2(i,1), base_processing_state_details);       
            irradiance_counts_old = irradiance_counts_i;
            stats(irradiance_counts_old);
            tracker->init(filtered_frame_i_matrix_8bit_color, ROI);

            step_sucess = true;
        }

        rectangle(filtered_frame_i_matrix_8bit_color, ROI, cv::Scalar( 0, 0, 255 ), 2);
        cv::imshow(window_name_i, filtered_frame_i_matrix_8bit_color);
        output.row(i) = {track_id, frame0 + i, frame_i_x ,frame_i_y, static_cast<uint16_t>(peak_counts_i),\
         static_cast<uint32_t>(sum_counts_i[0]),static_cast<uint32_t>(sum_ROI_counts_i[0]),N_threshold_pixels_i,N_ROI_pixels_i, static_cast<uint64_t>(irradiance_counts_i),\
         static_cast<uint16_t>(ROI.x),static_cast<uint16_t>(ROI.y),static_cast<uint16_t>(ROI.width),static_cast<uint16_t>(ROI.height)};
         
        cv::waitKey(1);
        i+=1;
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
    if (input_point.x > 0 && input_point.y > 0)
    {
        centerX = ceil((input_point.x + ROI.x)*1000.)/1000.;
        centerY = ceil((input_point.y + ROI.y)*1000.)/1000.;
    }
    else
    {
        centerX = ceil((ROI.x + 0.5 * ROI.width)*1000.)/1000.;
        centerY = ceil((ROI.y + 0.5 * ROI.height)*1000.)/1000.;
    }
}

void AutoTracking::GetProcessedFrameMatrix(int indx, double clamp_low, double clamp_high, VideoDetails & current_processing_state, arma::vec & frame_vector, cv::Mat & frame_matrix, cv::Mat & processed_frame_matrix)
{
    double m, s;      
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

void AutoTracking::HandleInterruption(QMessageBox::StandardButton &response, u_int i, u_int indx, u_int num_frames, arma::u64_mat & output, cv::Rect & ROI, cv::Mat &filtered_frame_matrix_8bit_color)
{
    
    if (response == QMessageBox::Save)
    {
        if (i>1)
        {
            output.shed_rows(i,num_frames-1);
        }
        else
        {
            QtHelpers::LaunchMessageBox("Empty Track", "The track is empty and nothing was saved.");
            output = arma::u64_mat();
        }
    }
    else if (response == QMessageBox::Discard)
    {
        output = arma::u64_mat();
    }
    else
    {
        cv::Mat filtered_frame_matrix_8bit_color_resize;
        string window_name_lost = "Track Lost or Paused. " + std::to_string(indx) + " Select ROI again.";
        cv::resize(filtered_frame_matrix_8bit_color, filtered_frame_matrix_8bit_color_resize, cv::Size(N*ncols, N*nrows));
        ROI = selectROI(window_name_lost, filtered_frame_matrix_8bit_color_resize);

        while (true)
        {
            int key = cv::pollKey();
            if (key == 13 && !ROI.empty()){ //Enter key
                cv::destroyWindow(window_name_lost);
                break;
            }
            else if(key == 27) { // Check for Esc key press
                ROI.width = 0;
                ROI.height = 0;
                cv::destroyAllWindows();
                QtHelpers::LaunchMessageBox("Canceled", "ROI selection canceled. Exiting without saving.");
                output = arma::u64_mat();
            } else if (cv::getWindowProperty(window_name_lost, cv::WND_PROP_VISIBLE) < 1) {
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
            output = arma::u64_mat();
        }
    }
}