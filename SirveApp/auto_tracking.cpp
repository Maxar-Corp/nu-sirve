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
arma::u64_mat AutoTracking::SingleTracker(
                                u_int track_id,
                                double clamp_low_coeff,
                                double clamp_high_coeff,
                                int threshold,
                                string prefilter,
                                string trackFeature,
                                uint frame0,
                                int start_frame,
                                int stop_frame,
                                processingState & current_processing_state,
                                VideoDetails & base_processing_state_details,
                                QString new_track_file_name
                                )
{
    cv::Scalar filtered_mean_i, filtered_sigma_i;

    cv::Scalar sum_counts_0, sum_ROI_counts_0, sum_counts_i, sum_ROI_counts_i;

    cv::Mat frame_0, frame_i, base_frame_0, base_frame_i, processed_frame_0, processed_frame_i, filtered_frame_0,
            filtered_frame_i, frame_filtered_8bit, filtered_frame_0_8bit_color, filtered_frame_i_8bit_color;

    cv::Mat frame_0_crop, frame_i_crop, base_frame_0_crop, base_frame_i_crop, filtered_frame_0_8bit_color_resize, filtered_frame_i_8bit_color_resize;

    arma::vec frame_0_vector, frame_i_vector;

    arma::vec frame_indices = arma::regspace(start_frame,stop_frame);

    arma::mat offset2(frame_indices.n_elem,3,arma::fill::zeros);

    std::vector<std::vector<int>> offsets;

    u_int frame_0_x, frame_0_y, frame_i_x, frame_i_y;

    u_int indx, num_frames = stop_frame - start_frame + 1;

    arma::u64_mat output(num_frames, 14);

    arma::running_stat<double> stats;

    cv::Point frame_0_point, frame_i_point;

    double peak_counts_0, peak_counts_i, adjusted_integrated_counts_0, adjusted_integrated_counts_old, adjusted_integrated_counts_i;

    uint N_threshold_pixels_0, Num_NonZero_ROI_Pixels_0, N_threshold_pixels_i, Num_NonZero_ROI_Pixels_i;

     if (current_processing_state.offsets.size()>0){
        offsets = current_processing_state.offsets;
        arma::mat offset(offsets.size(),3,arma::fill::zeros);
        for (int rowi = 0; rowi < offsets.size(); rowi++){
            offset.row(rowi) = arma::conv_to<arma::rowvec>::from(offsets[rowi]);
        }
        for (int rowii = 0; rowii<frame_indices.size(); rowii++){
            arma::uvec kk = arma::find(offset.col(0) == frame_indices(rowii) + 1,0,"first");
            if (!kk.is_empty()){
                offset2.row(rowii) = offset.row(kk(0));
            }
        }
        offset2.shed_col(0);
    }

    cv::startWindowThread();
    Ptr<Tracker> tracker = TrackerMIL::create();

    cv::Rect ROI;
    int i = 0;
    indx = (start_frame + i);
    GetFrameRepresentations(start_frame, clamp_low_coeff, clamp_high_coeff, current_processing_state.details, base_processing_state_details, frame_0, processed_frame_0, base_frame_0);
    FilterImage(prefilter, processed_frame_0, filtered_frame_0_8bit_color); 
    InitializeTracking(false, i, indx, num_frames, output, ROI, filtered_frame_0_8bit_color, frame_0, frame_0_crop, base_frame_0, base_frame_0_crop, tracker);
    GetTrackFeatureData(trackFeature, threshold, frame_0_crop, base_frame_0_crop, frame_0_point, peak_counts_0, sum_counts_0, sum_ROI_counts_0, N_threshold_pixels_0, Num_NonZero_ROI_Pixels_0);
    GetPointXY(frame_0_point, ROI, frame_0_x, frame_0_y);
    adjusted_integrated_counts_0 = IrradianceCountsCalc::ComputeIrradiance(start_frame, ROI.height/2, ROI.width/2, frame_0_x + offset2(0,0), frame_0_y + offset2(0,1), base_processing_state_details);
    adjusted_integrated_counts_old = adjusted_integrated_counts_0;
    stats(adjusted_integrated_counts_old);

    output.row(0) = {track_id, frame0, frame_0_x, frame_0_y, static_cast<uint16_t>(peak_counts_0),static_cast<uint32_t>(sum_counts_0[0]), static_cast<uint32_t>(sum_ROI_counts_0[0]), N_threshold_pixels_0, Num_NonZero_ROI_Pixels_0, static_cast<uint64_t>(adjusted_integrated_counts_0), static_cast<uint16_t>(ROI.x),static_cast<uint16_t>(ROI.y),static_cast<uint16_t>(ROI.width),static_cast<uint16_t>(ROI.height)};

    bool step_success = false;

    string window_name_i = "Tracking... ";
 
    while (i < num_frames)
    {
        if (cancel_operation)
        {                                 
            InitializeTracking(true, i, indx, num_frames, output, ROI, filtered_frame_i_8bit_color, frame_i, frame_i_crop, base_frame_i, base_frame_i_crop, tracker);
            step_success = true;
        }

        UpdateProgressBar(i);

        indx = (start_frame + i);

        GetFrameRepresentations(indx, clamp_low_coeff, clamp_high_coeff, current_processing_state.details, base_processing_state_details, frame_i, processed_frame_i, base_frame_i);
        FilterImage(prefilter, processed_frame_i, filtered_frame_i_8bit_color);
        bool ok = tracker->update(filtered_frame_i_8bit_color, ROI);
        frame_i_crop = frame_i(ROI);
        base_frame_i_crop = base_frame_i(ROI);
        GetTrackFeatureData(trackFeature, threshold, frame_i_crop, base_frame_i_crop, frame_i_point, peak_counts_i, sum_counts_i, sum_ROI_counts_i, N_threshold_pixels_i, Num_NonZero_ROI_Pixels_i);
        GetPointXY(frame_i_point, ROI, frame_i_x, frame_i_y);
        adjusted_integrated_counts_i = IrradianceCountsCalc::ComputeIrradiance(indx, ROI.height/2, ROI.width/2, frame_i_x + offset2(i,0), frame_i_y+ offset2(i,1), base_processing_state_details);
        stats(adjusted_integrated_counts_old);
        double S = stats.stddev();
        step_success = (ok && abs((adjusted_integrated_counts_i - stats.mean())) <= 6*S);

        if (S>0 && !step_success)
        {
            InitializeTracking(true, i, indx, num_frames, output, ROI, filtered_frame_i_8bit_color, frame_i, frame_i_crop, base_frame_i, base_frame_i_crop, tracker);
            GetTrackFeatureData(trackFeature, threshold, frame_i_crop, base_frame_i_crop, frame_i_point, peak_counts_i, sum_counts_i, sum_ROI_counts_i, N_threshold_pixels_i, Num_NonZero_ROI_Pixels_i);
            GetPointXY(frame_i_point, ROI, frame_i_x, frame_i_y);  
            adjusted_integrated_counts_i = IrradianceCountsCalc::ComputeIrradiance(indx, ROI.height/2, ROI.width/2, frame_i_x + offset2(i,0), frame_i_y + offset2(i,1), base_processing_state_details);       
            adjusted_integrated_counts_old = adjusted_integrated_counts_i;
            stats(adjusted_integrated_counts_old);

            step_success = true;
        }

        rectangle(filtered_frame_i_8bit_color, ROI, cv::Scalar( 0, 0, 255 ), 2);
        cv::namedWindow(window_name_i, cv::WINDOW_AUTOSIZE);
        cv::moveWindow(window_name_i, 1000, 50);  // Move window to (1000,50) coordinates
        cv::imshow(window_name_i, filtered_frame_i_8bit_color);
        output.row(i) = {track_id, frame0 + i, frame_i_x ,frame_i_y, static_cast<uint16_t>(peak_counts_i),static_cast<uint32_t>(sum_counts_i[0]),static_cast<uint32_t>(sum_ROI_counts_i[0]),N_threshold_pixels_i,Num_NonZero_ROI_Pixels_i, static_cast<uint64_t>(adjusted_integrated_counts_i), static_cast<uint16_t>(ROI.x),static_cast<uint16_t>(ROI.y),static_cast<uint16_t>(ROI.width),static_cast<uint16_t>(ROI.height)};
         
        cv::waitKey(1);
        i+=1;
    }
    cv::destroyAllWindows();
    return output;
}

void AutoTracking::GetFrameRepresentations(int indx, double clamp_low_coeff, double clamp_high_coeff, VideoDetails & current_processing_state, VideoDetails & base_processing_details, cv::Mat & frame, cv::Mat & processed_frame, cv::Mat & base_frame)
{
    cv::Scalar m, s;    
    std::vector<uint16_t> frame_vector = current_processing_state.frames_16bit[indx];
    cv::Mat tmp(nrows, ncols, CV_16UC1, frame_vector.data());
    tmp.convertTo(frame,CV_32FC1);
    cv::meanStdDev(frame, m, s);
    int clamp_low = m[0] - clamp_low_coeff*s[0];
    int clamp_high = m[0] + clamp_high_coeff*s[0];
    processed_frame = cv::min(cv::max(frame, clamp_low), clamp_high);
    processed_frame = processed_frame - clamp_low;
    processed_frame = 255*processed_frame/(clamp_high - clamp_low);
    processed_frame.convertTo(processed_frame, CV_8UC1);

    std::vector<uint16_t> base_frame_vector = base_processing_details.frames_16bit[indx];
    cv::Mat tmp2(nrows, ncols, CV_16UC1, base_frame_vector.data());
    tmp2.convertTo(base_frame, CV_32FC1);  
}

void AutoTracking::FilterImage(string prefilter, cv::Mat & input_frame, cv::Mat & output_frame)
{
  output_frame = input_frame;
  if(prefilter=="GAUSSIAN"){
        cv::GaussianBlur(input_frame, output_frame, cv::Size(5,5), 0);
    }
    else if(prefilter=="MEDIAN"){
        cv::medianBlur(input_frame, output_frame, 5);      
    }
    else if(prefilter=="NLMEANS"){
        cv::fastNlMeansDenoising(input_frame, output_frame);
    }
    cv::cvtColor(output_frame, output_frame,cv::COLOR_GRAY2RGB);
}

void AutoTracking::InitializeTracking(
                                    bool isRestart,
                                    u_int i,
                                    u_int indx,
                                    u_int num_frames,
                                    arma::u64_mat & output,
                                    cv::Rect & ROI,
                                    cv::Mat & filtered_frame_8bit_color,
                                    cv::Mat & frame,
                                    cv::Mat & frame_crop,
                                    cv::Mat & base_frame,
                                    cv::Mat & base_frame_crop,
                                    Ptr<Tracker> tracker
                                    )
{
    cv::Mat filtered_frame_8bit_color_resize;
    if (!isRestart)
    {
        cv::resize(filtered_frame_8bit_color, filtered_frame_8bit_color_resize, cv::Size(N*ncols, N*nrows));
        string ROI_window_name = "Region of Interest (ROI) Selection - Press Escape twice to Cancel, or Select ROI then Hit Enter twice to Continue.";
        GetValidROI(ROI_window_name, ROI, filtered_frame_8bit_color_resize, output);
    }
    else
    {
        cv::destroyAllWindows();
        QDialog dialog;
        dialog.setWindowTitle("Tracking Paused");
        QMessageBox::StandardButton response = QMessageBox::question(&dialog, "Track Paused or Lost... ", "Save available track data and exit, try to continue tracking, or discard and exit?",
                                                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Retry);
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
            string window_name_lost = "Track Paused or Lost. " + std::to_string(indx) + " Select ROI again.";
            cv::resize(filtered_frame_8bit_color, filtered_frame_8bit_color_resize, cv::Size(N*ncols, N*nrows));
            GetValidROI(window_name_lost, ROI, filtered_frame_8bit_color_resize, output);
        }
    }
    if (!ROI.empty() && !(ROI.width == 0 || ROI.height == 0))
    {
        ROI.x /= N;
        ROI.y /= N;
        ROI.width /= N;
        ROI.height /= N;
        frame_crop = frame(ROI);
        base_frame_crop = base_frame(ROI);
        tracker->init(filtered_frame_8bit_color,ROI);
        cancel_operation = false;
    }
    else
    {
        QtHelpers::LaunchMessageBox("Invalid ROI", "There was an error in the ROI selection. Exiting without saving.");
        cv::destroyAllWindows();
        cancel_operation = true;
        output = arma::u64_mat();
        return;
    }
}

void AutoTracking::GetValidROI(string window_name, cv::Rect & ROI, cv::Mat & filtered_frame_8bit_color_resize, arma::u64_mat & output)
{
    cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    cv::moveWindow(window_name, 50, 50);  // Move window to (50,50) coordinates
    ROI = cv::selectROI(window_name, filtered_frame_8bit_color_resize);

    while (true)
    {
        int key = cv::pollKey();
        if (key == 13 && !ROI.empty()){ //Enter key
            cv::destroyWindow(window_name);
            break;
        }
        else if(key == 27) { // Check for Esc key press
            ROI.width = 0;
            ROI.height = 0;
            cv::destroyAllWindows();
            QtHelpers::LaunchMessageBox("Cancelled", "ROI selection Cancelled. Exiting without saving.");
            cancel_operation = true;
            output = arma::u64_mat();
        } else if (cv::getWindowProperty(window_name, cv::WND_PROP_VISIBLE) < 1) {
            // Window has been closed
            ROI.width = 0;
            ROI.height = 0;
            break;
        }
    }
    return;

}

void  AutoTracking::GetTrackFeatureData(
                                        string trackFeature,
                                        int threshold,
                                        cv::Mat & frame_crop,
                                        cv::Mat & base_frame_crop,
                                        cv::Point & frame_point,
                                        double & peak_counts,
                                        cv::Scalar & sum_counts,
                                        cv::Scalar & sum_ROI_counts,
                                        uint & N_threshold_pixels,
                                        uint & Num_NonZero_ROI_Pixels
                                        )
{
    cv::Scalar frame_crop_mean, frame_crop_sigma, base_frame_crop_mean, base_frame_crop_sigma;
 
    cv::meanStdDev(frame_crop, frame_crop_mean, frame_crop_sigma);

    cv::Mat frame_crop_threshold, frame_crop_threshold_binary, base_frame_crop_threshold;

    sum_ROI_counts = cv::sum(base_frame_crop);
    Num_NonZero_ROI_Pixels = cv::countNonZero(base_frame_crop > 0);
    cv::minMaxLoc(base_frame_crop, NULL, &peak_counts, NULL, & frame_point);
    cv::inRange(frame_crop, 0, frame_crop_mean[0]+threshold*frame_crop_sigma[0],frame_crop_threshold_binary);
    base_frame_crop.copyTo(base_frame_crop_threshold, frame_crop_threshold_binary);
    sum_counts = cv::sum(base_frame_crop_threshold);
    N_threshold_pixels = cv::countNonZero(base_frame_crop_threshold > 0);

    if(trackFeature == "INTENSITY_WEIGHTED_CENTROID"){   
        frame_crop.copyTo(frame_crop_threshold,frame_crop_threshold_binary);
        cv::Moments frame_moments = cv::moments(frame_crop_threshold,false);
        cv::Point frame_temp_point(frame_moments.m10/frame_moments.m00, frame_moments.m01/frame_moments.m00);
        frame_point = frame_temp_point;
    }
    else if (trackFeature == "CENTROID"){
        cv::Moments frame_moments = cv::moments(frame_crop_threshold_binary,true);
        cv::Point frame_temp_point(frame_moments.m10/frame_moments.m00, frame_moments.m01/frame_moments.m00);
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
