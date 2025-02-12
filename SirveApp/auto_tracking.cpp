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
                                uint start_frame,
                                uint stop_frame,
                                processingState & current_processing_state,
                                VideoDetails & base_processing_state_details,
                                QString new_track_file_name
                                )
{
    double peak_counts, S, adjusted_integrated_counts_old = 0;

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
                    output,
                    adjusted_integrated_counts_old
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
            SharedTrackingFunctions::CheckROI(ROI, valid_ROI);
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

    SharedTrackingFunctions::GetFrameRepresentations(
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
    SharedTrackingFunctions::CheckROI(ROI, valid_ROI);
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
                                int & i,
                                uint & indx,
                                uint & track_id,
                                uint & frame0,
                                double & clamp_low_coeff,
                                double & clamp_high_coeff,
                                processingState & current_processing_state,
                                VideoDetails & base_processing_state_details,
                                string & prefilter,
                                Ptr<Tracker> & tracker,
                                string & trackFeature,
                                cv::Mat & display_frame,
                                cv::Mat & clean_display_frame,
                                int & threshold,
                                cv::Rect & ROI,
                                cv::Mat & frame,                           
                                cv::Mat & frame_crop,
                                cv::Mat & raw_frame,
                                cv::Mat & raw_frame_crop,
                                cv::Point & frame_point,
                                arma::running_stat<double> & stats,
                                bool & step_success,
                                double & S,
                                double & peak_counts,
                                cv::Scalar & sum_counts,
                                cv::Scalar & sum_ROI_counts,
                                uint & N_threshold_pixels,
                                uint & Num_NonZero_ROI_Pixels,
                                arma::mat & offsets_matrix,
                                arma::u64_mat & output,
                                double & adjusted_integrated_counts_old
                                )
{
    u_int frame_x, frame_y;
    double adjusted_integrated_counts;
   
    SharedTrackingFunctions::GetFrameRepresentations(indx, clamp_low_coeff, clamp_high_coeff, current_processing_state.details, base_processing_state_details, frame, prefilter, display_frame, clean_display_frame, raw_frame);

    bool ok = tracker->update(display_frame, ROI);

    cv::Mat frame_crop_threshold;
    cv::Rect bbox = ROI;
    cv::Rect bbox_uncentered = bbox;
    SharedTrackingFunctions::FindTargetExtent(i, clamp_low_coeff, clamp_high_coeff, frame, threshold, frame_crop_threshold, ROI, bbox, offsets_matrix, bbox_uncentered); //Returns absolute position of bbox within frame

    frame_crop = frame(bbox);
    raw_frame_crop = raw_frame(bbox_uncentered);

    SharedTrackingFunctions::GetTrackPointData(trackFeature, threshold, frame_crop, raw_frame_crop, frame_crop_threshold, frame_point, peak_counts, sum_counts, sum_ROI_counts, N_threshold_pixels, Num_NonZero_ROI_Pixels);
    SharedTrackingFunctions::GetPointXY(frame_point, bbox, frame_x, frame_y);
    
    adjusted_integrated_counts = SharedTrackingFunctions::GetAdjustedCounts(indx, bbox_uncentered, base_processing_state_details);
    adjusted_integrated_counts_old = adjusted_integrated_counts;
    stats(adjusted_integrated_counts_old);
    S = stats.stddev();
    step_success = (ok && abs((adjusted_integrated_counts - stats.mean())) <= 3*S);

    string window_name = "Tracking... ";
    rectangle(display_frame, ROI, cv::Scalar( 0, 0, 255 ), 1);
    rectangle(display_frame, bbox, cv::Scalar( 255, 255, 0 ), 1);
    cv::Point point(frame_x, frame_y);
    cv::circle(display_frame, point, 2, cv::Scalar(0, 0, 255), -1); // Red color, filled

    cv::imshow(window_name, display_frame);     
    cv::moveWindow(window_name, 50, 50); 
    cv::waitKey(1);
    output.row(i) = {track_id, frame0 + i, frame_x ,frame_y, static_cast<uint16_t>(peak_counts),static_cast<uint32_t>(sum_counts[0]),static_cast<uint32_t>(sum_ROI_counts[0]),N_threshold_pixels,Num_NonZero_ROI_Pixels, static_cast<uint64_t>(adjusted_integrated_counts), static_cast<uint16_t>(ROI.x),static_cast<uint16_t>(ROI.y),static_cast<uint16_t>(ROI.width),static_cast<uint16_t>(ROI.height)};
}

