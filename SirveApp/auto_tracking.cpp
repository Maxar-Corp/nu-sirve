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
    double peak_counts, S;

    cv::Scalar sum_counts, sum_ROI_counts;

    cv::Mat frame, raw_frame, display_frame, clean_display_frame;

    cv::Mat frame_crop, raw_frame_crop, display_frame_resize;

    arma::vec frame_vector;

    u_int indx, num_frames = stop_frame - start_frame + 1;

    arma::u64_mat output(num_frames, 14);
    
    arma::running_stat<double> stats;

    arma::mat offsets_matrix;

    cv::Point frame_point;

    uint N_threshold_pixels, Num_NonZero_ROI_Pixels;

    string choice;

    CreateOffsetMatrix(start_frame ,stop_frame, current_processing_state, offsets_matrix);

    cv::startWindowThread();

    Ptr<Tracker> tracker = TrackerMIL::create();

    bool valid_ROI, step_success;
    cv::Rect ROI, bbox;

    int i = 0;
    indx = (start_frame + i);

    InitializeTracking(
                        false,
                        i,
                        indx,
                        num_frames,
                        clamp_low_coeff,
                        clamp_high_coeff,
                        current_processing_state.details,
                        base_processing_state_details,
                        prefilter,
                        display_frame,
                        clean_display_frame,
                        ROI,
                        valid_ROI,
                        frame,
                        frame_crop,
                        raw_frame,
                        raw_frame_crop,
                        tracker,
                        choice,
                        stats
                    );


    if (!valid_ROI || choice == "Discard")
    {
        output = arma::u64_mat();
        return output;
    }

    while (i < num_frames)
    {

        UpdateProgressBar(i);

        TrackingStep(
                    i,
                    indx,
                    track_id,
                    frame0,
                    clamp_low_coeff,
                    clamp_high_coeff,
                    current_processing_state,
                    base_processing_state_details,
                    prefilter,
                    tracker,
                    trackFeature,
                    display_frame,
                    clean_display_frame,
                    threshold,
                    ROI,
                    frame,                           
                    frame_crop,
                    raw_frame,
                    raw_frame_crop,
                    frame_point,
                    stats,
                    step_success,
                    S,
                    peak_counts,
                    sum_counts,
                    sum_ROI_counts,
                    N_threshold_pixels,
                    Num_NonZero_ROI_Pixels,
                    offsets_matrix,
                    output
                    );

        
        i+=1;
        indx = (start_frame + i);
        if ((S>0 && !step_success) || (cancel_operation))
        {
            i -=1;
            indx -=1;
            InitializeTracking(
                                true,
                                i,
                                indx,
                                num_frames,
                                clamp_low_coeff,
                                clamp_high_coeff,
                                current_processing_state.details,
                                base_processing_state_details,
                                prefilter,
                                display_frame,
                                clean_display_frame,
                                ROI,
                                valid_ROI,
                                frame,
                                frame_crop,
                                raw_frame,
                                raw_frame_crop,
                                tracker,
                                choice,
                                stats
                            );
            CheckROI(ROI, valid_ROI);
            if (!valid_ROI || choice == "Discard")
            {
                output = arma::u64_mat();
                return output;
            }
            else if (choice == "Save")
            {
                output.shed_rows(i,num_frames-1);
                return output;
            }
        }
    }
    cv::destroyAllWindows();
    return output;
}

void AutoTracking::GetFrameRepresentations(
                                            int indx,
                                            double clamp_low_coeff,
                                            double clamp_high_coeff,
                                            VideoDetails & current_processing_state,
                                            VideoDetails & base_processing_details,
                                            cv::Mat & frame,
                                            string prefilter,
                                            cv::Mat & display_frame,
                                            cv::Mat & clean_display_frame,
                                            cv::Mat & raw_frame
                                        )
{
    cv::Scalar m, s;    
    std::vector<uint16_t> frame_vector = current_processing_state.frames_16bit[indx];
    cv::Mat tmp(nrows, ncols, CV_16UC1, frame_vector.data());
    tmp.convertTo(frame,CV_32FC1);
    cv::meanStdDev(frame, m, s);
    int clamp_low = m[0] - clamp_low_coeff*s[0];
    int clamp_high = m[0] + clamp_high_coeff*s[0];
    display_frame = cv::min(cv::max(frame, clamp_low), clamp_high);
    display_frame = display_frame - clamp_low;
    display_frame = 255*display_frame/(clamp_high - clamp_low);
    display_frame.convertTo(display_frame, cv::COLOR_GRAY2BGR);

    std::vector<uint16_t> raw_frame_vector = base_processing_details.frames_16bit[indx];
    cv::Mat tmp2(nrows, ncols, CV_16UC1, raw_frame_vector.data());
    tmp2.convertTo(raw_frame, CV_32FC1);  

    FilterImage(prefilter, display_frame, clean_display_frame);
}

void AutoTracking::FilterImage(string prefilter, cv::Mat & display_frame, cv::Mat & clean_display_frame)
{
  if(prefilter=="GAUSSIAN"){
        cv::GaussianBlur(display_frame, display_frame, cv::Size(5,5), 0);
    }
    else if(prefilter=="MEDIAN"){
        cv::medianBlur(display_frame, display_frame, 5);      
    }
    else if(prefilter=="NLMEANS"){
        cv::fastNlMeansDenoising(display_frame, display_frame);
    }
    cv::cvtColor(display_frame, display_frame,cv::COLOR_GRAY2RGB);
    clean_display_frame = display_frame.clone();
}

void AutoTracking::InitializeTracking(
                                    bool isRestart,
                                    u_int i,
                                    u_int indx,
                                    u_int num_frames,
                                    double clamp_low_coeff,
                                    double clamp_high_coeff,
                                    VideoDetails & current_processing_state,
                                    VideoDetails & base_processing_details,
                                    string prefilter,
                                    cv::Mat & display_frame,
                                    cv::Mat & clean_display_frame,
                                    cv::Rect & ROI,
                                    bool & valid_ROI,
                                    cv::Mat & frame,
                                    cv::Mat & frame_crop,
                                    cv::Mat & raw_frame,
                                    cv::Mat & raw_frame_crop,
                                    Ptr<Tracker> tracker,
                                    string & choice,
                                    arma::running_stat<double> & stats
                                    )
{
    cv::Mat display_frame_resize;

    GetFrameRepresentations(
                        indx,
                        clamp_low_coeff,
                        clamp_high_coeff,
                        current_processing_state,
                        base_processing_details,
                        frame,
                        prefilter,
                        display_frame,
                        clean_display_frame,
                        raw_frame
                    );

    if (!isRestart)
    {
        cv::resize(display_frame, display_frame_resize, cv::Size(N*ncols, N*nrows));
        string ROI_window_name = "Region of Interest (ROI) Selection - Press Escape twice to Cancel, or Select ROI then Hit Enter twice to Continue.";
        GetROI(ROI_window_name, ROI, display_frame_resize);
        choice = "Continue";
    }
    else
    {
        cv::destroyAllWindows();
        QDialog dialog;
        dialog.setWindowTitle("Track Paused or Lost");
        QMessageBox::StandardButton response = QMessageBox::question(&dialog, "Track Paused or Lost... ", "Save available track data and exit, continue tracking, or discard and exit?",
                                                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Retry);
        if (response == QMessageBox::Save)
        {
            if (i>1)
            {
                choice = "Save";
            }
            else
            {
                QtHelpers::LaunchMessageBox("Empty Track", "The track is empty and nothing was saved.");
                choice = "Discard";
            }
        }
        else if (response == QMessageBox::Discard)
        {
            choice = "Discard";
        }
        else
        {
            string window_name_lost = "Track Paused or Lost. " + std::to_string(indx) + " Select ROI again.";
            cv::resize(display_frame, display_frame_resize, cv::Size(N*ncols, N*nrows));
            GetROI(window_name_lost, ROI, display_frame_resize);
            choice = "Continue";
        }
    }
    CheckROI(ROI, valid_ROI);
    if (valid_ROI)
    {
        ROI.x /= N;
        ROI.y /= N;
        ROI.width /= N;
        ROI.height /= N;
        frame_crop = frame(ROI);
        raw_frame_crop = raw_frame(ROI);
        tracker->init(display_frame,ROI);
        cancel_operation = false;
        stats.reset();
    }

}

void AutoTracking::GetROI(string window_name, cv::Rect & ROI, cv::Mat & display_frame_resize)
{
    // cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    ROI = cv::selectROI(window_name, display_frame_resize);  
    cv::moveWindow(window_name, 50, 50);  
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
            break;
        } else if (cv::getWindowProperty(window_name, cv::WND_PROP_VISIBLE) < 1) {
            // Window has been closed
            ROI.width = 0;
            ROI.height = 0;
            break;
        }
    }
}


void AutoTracking::TrackingStep(
                                int i,
                                int indx,
                                uint track_id,
                                uint frame0,
                                double clamp_low_coeff,
                                double clamp_high_coeff,
                                processingState & current_processing_state,
                                VideoDetails & base_processing_state_details,
                                string & prefilter,
                                Ptr<Tracker> & tracker,
                                string trackFeature,
                                cv::Mat & display_frame,
                                cv::Mat & clean_display_frame,
                                int threshold,
                                cv::Rect & ROI,
                                cv::Mat & frame,                           
                                cv::Mat & frame_crop,
                                cv::Mat & raw_frame,
                                cv::Mat & raw_frame_crop,
                                cv::Point & frame_point,
                                arma::running_stat<double> &stats,
                                bool & step_success,
                                double & S,
                                double & peak_counts,
                                cv::Scalar & sum_counts,
                                cv::Scalar & sum_ROI_counts,
                                uint & N_threshold_pixels,
                                uint & Num_NonZero_ROI_Pixels,
                                arma::mat & offsets_matrix,
                                arma::u64_mat & output
                                )
{
    u_int frame_x, frame_y, peak_count;
    double adjusted_integrated_counts, adjusted_integrated_counts_old = 0.0;
    
    cv::Rect bbox;

    GetFrameRepresentations(
        indx,
        clamp_low_coeff,
        clamp_high_coeff,
        current_processing_state.details,
        base_processing_state_details,
        frame,
        prefilter,
        display_frame,
        clean_display_frame,
        raw_frame
    );

    bool ok = tracker->update(display_frame, ROI);
    frame_crop = frame(ROI);
    raw_frame_crop = raw_frame(ROI);
    GetTrackFeatureData(trackFeature, threshold, frame_crop, raw_frame_crop, frame_point, peak_counts, sum_counts, sum_ROI_counts, N_threshold_pixels, Num_NonZero_ROI_Pixels);
    GetPointXY(frame_point, ROI, frame_x, frame_y);
    arma::mat offsets = offsets_matrix.row(i);
    FindBlobExtent(frame_crop, ROI, bbox, offsets);
    adjusted_integrated_counts = IrradianceCountsCalc::ComputeIrradiance(indx, bbox, base_processing_state_details);
    stats(adjusted_integrated_counts_old);
    S = stats.stddev();
    step_success = (ok && abs((adjusted_integrated_counts - stats.mean())) <= 6*S);

    string window_name = "Tracking... ";
    rectangle(display_frame, ROI, cv::Scalar( 0, 0, 255 ), 1);
    rectangle(display_frame, bbox, cv::Scalar( 255, 255, 0 ), 1);
    // cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
    cv::imshow(window_name, display_frame);     
    cv::moveWindow(window_name, 50, 50); 
    cv::waitKey(1);
    output.row(i) = {track_id, frame0 + i, frame_x ,frame_y, static_cast<uint16_t>(peak_counts),static_cast<uint32_t>(sum_counts[0]),static_cast<uint32_t>(sum_ROI_counts[0]),N_threshold_pixels,Num_NonZero_ROI_Pixels, static_cast<uint64_t>(adjusted_integrated_counts), static_cast<uint16_t>(ROI.x),static_cast<uint16_t>(ROI.y),static_cast<uint16_t>(ROI.width),static_cast<uint16_t>(ROI.height)};
}

void  AutoTracking::GetTrackFeatureData(
                                        string trackFeature,
                                        int threshold,
                                        cv::Mat & frame_crop,
                                        cv::Mat & raw_frame_crop,
                                        cv::Point & frame_point,
                                        double & peak_counts,
                                        cv::Scalar & sum_counts,
                                        cv::Scalar & sum_ROI_counts,
                                        uint & N_threshold_pixels,
                                        uint & Num_NonZero_ROI_Pixels
                                        )
{
    cv::Scalar frame_crop_mean, frame_crop_sigma, raw_frame_crop_mean, raw_frame_crop_sigma;
 
    cv::meanStdDev(frame_crop, frame_crop_mean, frame_crop_sigma);

    cv::Mat frame_crop_threshold, frame_crop_threshold_resize, frame_crop_threshold_binary, raw_frame_crop_threshold;

    sum_ROI_counts = cv::sum(raw_frame_crop);
    Num_NonZero_ROI_Pixels = cv::countNonZero(raw_frame_crop > 0);
    cv::minMaxLoc(raw_frame_crop, NULL, & peak_counts, NULL, NULL);
    cv::minMaxLoc(frame_crop, NULL, NULL, NULL, & frame_point);
    cv::threshold(frame_crop, frame_crop_threshold, frame_crop_mean[0]+threshold*frame_crop_sigma[0], NULL, cv::THRESH_TOZERO);
    frame_crop_threshold.convertTo(frame_crop_threshold_binary, CV_8U);
    raw_frame_crop.copyTo(raw_frame_crop_threshold, frame_crop_threshold_binary);
    
    sum_counts = cv::sum(raw_frame_crop_threshold);
    N_threshold_pixels = cv::countNonZero(raw_frame_crop_threshold > 0);
    
    double maxval;
    cv::minMaxLoc(frame_crop_threshold, NULL, & maxval, NULL, NULL);
    frame_crop_threshold /= maxval;
    frame_crop_threshold *= 255;
    cv::resize(frame_crop_threshold, frame_crop_threshold_resize, cv::Size(10*frame_crop_threshold.cols, 10*frame_crop_threshold.rows));
    frame_crop_threshold_resize.convertTo(frame_crop_threshold_resize, cv::COLOR_GRAY2BGR);
    imshow("Thresholded ROI",frame_crop_threshold_resize);
    cv::moveWindow("Thresholded ROI", 700, 50);
    
    if(trackFeature == "INTENSITY_WEIGHTED_CENTROID"){   
        // frame_crop.copyTo(frame_crop_threshold,frame_crop_threshold_binary);
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

void AutoTracking::CheckROI(cv::Rect & ROI, bool & valid_ROI)
{
    valid_ROI = (!ROI.empty() && !(ROI.width == 0 || ROI.height == 0));    
}

void AutoTracking::FindBlobExtent(cv::Mat & input_image, cv::Rect & ROI, cv::Rect & bbox, arma::mat & offsets)
{
    int M = 20;

     // Create binary mask where intensity drops below threshold
    cv::Mat temp_image, mask;
    cv::Mat output, output_resize, output_resize2;
    cv::Scalar m,s; 
    input_image.convertTo(temp_image,CV_32FC1);
    cv::meanStdDev(temp_image, m, s);
    int clamp_low = m[0] - 3*s[0];
    int clamp_high = m[0] + 3*s[0];
    temp_image = cv::min(cv::max(temp_image, clamp_low), clamp_high);
    temp_image = temp_image - clamp_low;
    temp_image = 255*temp_image/(clamp_high - clamp_low);
    temp_image.convertTo(temp_image, CV_8UC1);

    double minVal, maxVal;
    cv::minMaxLoc(temp_image, &minVal, &maxVal);
    double thresholdVal = maxVal * 0.5;
    cv::threshold(temp_image, mask, thresholdVal, 255, cv::THRESH_BINARY);
    mask.convertTo(mask, CV_8U);
    
    // Find contours of the blob
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Ensure at least one contour is found
    if (!contours.empty()) {
        // Find the largest outer contour by area
        std::vector<cv::Point> largestContour = contours[0];
        for (const auto& contour : contours) {
            if (cv::contourArea(contour) > cv::contourArea(largestContour)) {
                largestContour = contour;
            }
        }

        // Get the bounding rectangle
        bbox = cv::boundingRect(largestContour);

        // Draw the bounding rectangle
        output = temp_image.clone();
        cv::cvtColor(output, output, cv::COLOR_GRAY2BGR);
        cv::rectangle(output, bbox, cv::Scalar(0, 255, 0), 1);
          
        cv::resize(output, output_resize2, cv::Size(M*output.cols, M*output.rows));
        cv::imshow("Blob Extent", output_resize2);
        cv::moveWindow("Blob Extent", 1400, 50);
        bbox.x = ROI.x + bbox.x + offsets(0,0);
        bbox.y = ROI.y + bbox.y + offsets(0,1);

        // cv::waitKey(0);
    } else {
        std::cerr << "No contours found!" << std::endl;
        bbox = ROI;
        bbox.x += offsets(0,0);
        bbox.y += offsets(0,1);
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

void AutoTracking::CreateOffsetMatrix(int start_frame, int stop_frame, processingState & state_details, arma::mat & offsets_matrix)
{
    arma::vec frame_indices = arma::regspace(start_frame,stop_frame);

    offsets_matrix.set_size(frame_indices.n_elem,3);
    offsets_matrix.fill(0);

    std::vector<std::vector<int>> offsets = state_details.offsets;

    arma::mat offsets_matrix_tmp(offsets.size(),3,arma::fill::zeros);
    for (int rowi = 0; rowi < offsets.size(); rowi++){
        offsets_matrix_tmp.row(rowi) = arma::conv_to<arma::rowvec>::from(offsets[rowi]);
    }
    for (int rowii = 0; rowii<frame_indices.size(); rowii++){
        arma::uvec kk = arma::find(offsets_matrix_tmp.col(0) == frame_indices(rowii) + 1,0,"first");
        if (!kk.is_empty()){
            offsets_matrix.row(rowii) = offsets_matrix_tmp.row(kk(0));
        }
    }
    offsets_matrix.shed_col(0);
}
