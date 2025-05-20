#include "auto_tracking.h"

AutoTracking::AutoTracking(ABPFileType file_type)
{
    if (file_type == ABPFileType::ABP_D)
    {
        nRows = 720;
        nCols = 1280;
        image_scale_factor = 1;
    }
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
arma::s64_mat AutoTracking::SingleTracker(
    QSize screenResolution,
    QPoint appPos,
    u_int track_id,
    double clamp_low_coeff,
    double clamp_high_coeff,
    int threshold,
    int bbox_buffer_pixels,
    string prefilter,
    string trackFeature,
    uint frame0,
    uint start_frame,
    uint stop_frame,
    const ProcessingState& current_processing_state,
    const VideoDetails& base_processing_state_details,
    const std::vector<ABIRFrameHeader>& input_frame_header,
    const CalibrationData& calibration_model
)
{
    double peak_counts, SIGMA, sum_relative_counts__old = 0, mean_counts;

    cv::Scalar sum_counts;

    cv::Mat frame, raw_frame, display_frame, clean_display_frame;

    cv::Mat frame_crop, frame_bbox, raw_frame_bbox, display_frame_resize;

    arma::vec frame_vector;

    u_int indx, num_frames = stop_frame - start_frame + 1;

    arma::s64_mat output(num_frames, 18);
    
    arma::running_stat<double> stats;

    arma::mat offsets_matrix;

    cv::Point frame_point;

    uint number_pixels;

    string choice;

    SirveApp_x = appPos.x();
    SirveApp_y = appPos.y();
    Display_res_x = screenResolution.width();
    Display_res_y = screenResolution.height();

    ROI_window_y = SirveApp_y;
    ROI_window_x = 10;
    tracking_window_y =  SirveApp_y;
    tracking_window_x = 10;
    extent_window_x = 10; 
    extent_window_y = std::min(tracking_window_y, Display_res_y - nRows);
    if (Display_res_x > 1920)
    {
        ROI_window_x = std::max(SirveApp_x - image_scale_factor*(nCols) - 10,ROI_window_x);
        tracking_window_x = std::max(SirveApp_x - nCols - 500, tracking_window_x);
        extent_window_x = tracking_window_x + nCols + 10;
    }

    raw_window_x = tracking_window_x;
    raw_window_y = std::min(tracking_window_y + nRows + 10, Display_res_y - nRows);

    SharedTrackingFunctions::CreateOffsetMatrix(start_frame, stop_frame, current_processing_state, offsets_matrix);

    cv::startWindowThread();

    Ptr<Tracker> tracker = TrackerMIL::create();

    bool valid_ROI, step_success;
    cv::Rect ROI;

    int i = 0;
    indx = (start_frame + i);

    InitializeTracking(
                        false,
                        i,
                        indx,
                        frame0,
                        clamp_low_coeff,
                        clamp_high_coeff,
                        current_processing_state.details,
                        base_processing_state_details,
                        prefilter,
                        display_frame,
                        raw_display_frame,
                        clean_display_frame,
                        ROI,
                        valid_ROI,
                        frame,
                        frame_crop,
                        raw_frame,
                        tracker,
                        choice,
                        stats
                    );


    if (!valid_ROI || choice == "Discard")
    {
        output = arma::s64_mat();
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
                    input_frame_header,
                    prefilter,
                    tracker,
                    trackFeature,
                    display_frame,
                    raw_display_frame,
                    clean_display_frame,
                    threshold,
                    bbox_buffer_pixels,
                    ROI,
                    frame,                           
                    raw_frame,
                    raw_frame_bbox,
                    frame_point,
                    stats,
                    step_success,
                    SIGMA,
                    peak_counts,
                    mean_counts,
                    sum_counts,
                    number_pixels,
                    offsets_matrix,
                    output,
                    sum_relative_counts__old,
                    calibration_model
                    );

        
        i+=1;
        indx = (start_frame + i);
        if ((SIGMA>0 && !step_success) || (cancel_operation))
        {
            i -= 1;
            indx -= 1;
            InitializeTracking(
                                true,
                                i,
                                indx,
                                frame0,
                                clamp_low_coeff,
                                clamp_high_coeff,
                                current_processing_state.details,
                                base_processing_state_details,
                                prefilter,
                                display_frame,
                                clean_display_frame,
                                raw_display_frame,
                                ROI,
                                valid_ROI,
                                frame,
                                frame_crop,
                                raw_frame,
                                tracker,
                                choice,
                                stats
                            );
            SharedTrackingFunctions::CheckROI(ROI, valid_ROI);
            if (!valid_ROI || choice == "Discard")
            {
                output = arma::s64_mat();
                return output;
            }
            else if (choice == "Save")
            {
                i+=1;
                output.shed_rows(i,output.n_rows-1);
                return output;
            }
        }
    }
    cv::destroyAllWindows();
    return output;
}

void AutoTracking::InitializeTracking(
    bool isRestart,
    u_int i,
    u_int indx,
    u_int frame0,
    double clamp_low_coeff,
    double clamp_high_coeff,
    const VideoDetails& current_processing_state,
    const VideoDetails& base_processing_details,
    string prefilter,
    cv::Mat & display_frame,
    cv::Mat & raw_display_frame,
    cv::Mat & clean_display_frame,
    cv::Rect & ROI,
    bool & valid_ROI,
    cv::Mat & frame,
    cv::Mat & frame_crop,
    cv::Mat & raw_frame,
    Ptr<Tracker> tracker,
    string & choice,
    arma::running_stat<double> & stats
)
{
    cv::Mat display_frame_resize;

    SharedTrackingFunctions::GetFrameRepresentations(
        indx,
        clamp_low_coeff,
        clamp_high_coeff,
        current_processing_state,
        base_processing_details,
        frame,
        prefilter,
        display_frame,
        raw_display_frame,
        clean_display_frame,
        raw_frame
    );

    if (!isRestart)
    {
        cv::resize(display_frame, display_frame_resize, cv::Size(image_scale_factor*nCols, image_scale_factor*nRows));
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
            cv::resize(display_frame, display_frame_resize, cv::Size(image_scale_factor*nCols, image_scale_factor*nRows));
            string window_name_lost = "Track Paused or Lost at Frame " + std::to_string(indx+frame0) + " Select ROI again.";
            GetROI(window_name_lost, ROI, display_frame_resize);
            choice = "Continue";
        }
    }

    SharedTrackingFunctions::CheckROI(ROI, valid_ROI);

    if (valid_ROI)
    {
        ROI.x /= image_scale_factor;
        ROI.y /= image_scale_factor;
        ROI.width /= image_scale_factor;
        ROI.height /= image_scale_factor;
        frame_crop = frame(ROI);
        tracker->init(display_frame,ROI);
        cancel_operation = false;
        stats.reset();
    }

}

void AutoTracking::GetROI(string window_name, cv::Rect & ROI, cv::Mat & display_frame_resize)
{
    cv::namedWindow(window_name,cv::WINDOW_AUTOSIZE);
    cv::moveWindow(window_name,ROI_window_x, ROI_window_y);
    cv::cvtColor(display_frame_resize, display_frame_resize,cv::COLOR_GRAY2RGB);
    ROI = cv::selectROI(window_name, display_frame_resize);   

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
    int & i,
    uint & indx,
    uint & track_id,
    uint & frame0,
    double & clamp_low_coeff,
    double & clamp_high_coeff,
    const ProcessingState& current_processing_state,
    const VideoDetails& base_processing_state_details,
    const std::vector<ABIRFrameHeader>& input_frame_header,
    string & prefilter,
    Ptr<Tracker> & tracker,
    string & trackFeature,
    cv::Mat & display_frame,
    cv::Mat & raw_display_frame,
    cv::Mat & clean_display_frame,
    int & threshold,
    int & bbox_buffer_pixels,
    cv::Rect & ROI,
    cv::Mat & frame,
    cv::Mat & raw_frame,
    cv::Mat & raw_frame_bbox,
    cv::Point & frame_point,
    arma::running_stat<double> & stats,
    bool & step_success,
    double & SIGMA,
    double & peak_counts,
    double & mean_counts,
    cv::Scalar & sum_counts,
    uint & number_pixels,
    arma::mat & offsets_matrix,
    arma::s64_mat & output,
    double & sum_relative_counts__old,
    const CalibrationData& calibration_model
)
{
    int frame_x, frame_y;

    double sum_relative_counts;

    SharedTrackingFunctions::GetFrameRepresentations(indx, clamp_low_coeff, clamp_high_coeff, current_processing_state.details, base_processing_state_details, frame, prefilter, display_frame, raw_display_frame, clean_display_frame, raw_frame);

    bool ok = tracker->update(clean_display_frame, ROI);

    cv::Mat frame_crop_threshold;
    cv::Rect bbox = ROI;
    cv::Rect bbox_uncentered = bbox;
    SharedTrackingFunctions::FindTargetExtent(nRows, nCols, i, clamp_low_coeff, clamp_high_coeff, clean_display_frame, threshold, bbox_buffer_pixels, frame_crop_threshold, ROI, bbox, offsets_matrix, bbox_uncentered, extent_window_x, extent_window_y); //Returns absolute position of bbox within frame

    cv::Mat frame_bbox = frame(bbox);
    raw_frame_bbox = raw_frame(bbox_uncentered);
    if (bbox_uncentered.size() == bbox.size())
    {

        SharedTrackingFunctions::GetTrackPointData(trackFeature, frame_bbox, raw_frame_bbox, frame_crop_threshold, frame_point, peak_counts, mean_counts, sum_counts, number_pixels);

        SharedTrackingFunctions::GetPointXY(frame_point, bbox, frame_x, frame_y);

        double frame_integration_time = input_frame_header[indx].int_time;

        sum_relative_counts = SharedTrackingFunctions::GetAdjustedCounts(indx+1, bbox_uncentered, base_processing_state_details);
        sum_relative_counts__old = sum_relative_counts;
        stats(sum_relative_counts__old);
        SIGMA = stats.stddev();
        step_success = (ok && abs((sum_relative_counts - stats.mean())) <= step_success_coefficient*SIGMA);

        std::array<double, 3> measurements = {0,0,0};
        if (calibration_model.calibration_available)
        {
            measurements =  SharedTrackingFunctions::CalculateIrradiance(indx, bbox_uncentered,base_processing_state_details,frame_integration_time, calibration_model);
        }
        cv::cvtColor(clean_display_frame, clean_display_frame,cv::COLOR_GRAY2RGB);
        string window_name = "Tracking... ";
        rectangle(clean_display_frame, ROI, cv::Scalar( 0, 0, 255 ), 1);
        rectangle(clean_display_frame, bbox, cv::Scalar( 255, 255, 0 ), 1);
        cv::Point point(frame_x, frame_y);
        cv::circle(clean_display_frame, point, 2, cv::Scalar(0, 0, 255), -1); // Red color, filled
        cv::imshow(window_name, clean_display_frame);    
        if (i==0)
        {
            cv::moveWindow(window_name, tracking_window_x, tracking_window_y);           
        }
        
        string raw_window_name = "Raw Data Tracking... ";
        bool bbox_uncentered_valid = (bbox_uncentered.x>0 && (bbox_uncentered.x+bbox_uncentered.width)<nCols && bbox_uncentered.y>0 && (bbox_uncentered.y+bbox_uncentered.height)<nRows); 
        if (bbox_uncentered_valid)
        {
            rectangle(raw_display_frame, bbox_uncentered, cv::Scalar( 0, 0, 255 ), 1);
        }
        cv::imshow(raw_window_name, raw_display_frame); 
        if (i==0)
        {
            cv::moveWindow(raw_window_name, raw_window_x, raw_window_y); 
        }
        cv::waitKey(1);

        output.row(i) =  {
                            static_cast<uint16_t>(track_id),
                            static_cast<uint16_t>(frame0 + i),
                            frame_x - nRows/2,
                            frame_y - nCols/2,
                            frame_x,
                            frame_y,
                            static_cast<int32_t>(number_pixels),
                            static_cast<uint16_t>(peak_counts),
                            static_cast<int32_t>(mean_counts),
                            static_cast<int32_t>(sum_counts[0]),
                            static_cast<int64_t>(sum_relative_counts),
                            static_cast<int32_t>(measurements[0]),
                            static_cast<int32_t>(measurements[1]),
                            static_cast<int32_t>(measurements[2]),
                            static_cast<uint16_t>(bbox_uncentered.x),
                            static_cast<uint16_t>(bbox_uncentered.y),
                            static_cast<uint16_t>(bbox_uncentered.width),
                            static_cast<uint16_t>(bbox_uncentered.height)
                        };
    }
    else
    {
        step_success = false;   
        output.row(i) =  {
            static_cast<uint16_t>(track_id),
            static_cast<uint16_t>(frame0 + i),
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0
        };
    }
}

void AutoTracking::SetCalibrationModel(CalibrationData input)
{
    model = input;
}
