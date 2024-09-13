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
arma::u64_mat AutoTracking::SingleTracker(u_int track_id, double clamp_low, double clamp_high, int threshold, string prefilter, string trackFeature, uint frame0, int start_frame, int stop_frame, VideoDetails original, QString new_track_file_name)
{
    double irradiance;
    cv::Scalar filtered_meani, filtered_stdi, frame_crop_mean;
    u_int indx0 = start_frame;
    cv::Mat frame_0_matrix, frame_i_matrix, processed_frame_0_matrix, processed_frame_i_matrix, filtered_frame_0_matrix, filtered_frame_i_matrix, frame_matrix_filtered_8bit, filtered_frame_0_matrix_8bit_color, filtered_frame_i_matrix_8bit_color;
    cv::Mat frame_0_crop, frame_i_crop;
    arma::vec frame_0_vector, frame_i_vector;

    GetProcessedFrameMatrix(indx0, clamp_low, clamp_high, original, frame_0_vector, frame_0_matrix, processed_frame_0_matrix);

    // attenuate image noise of initial frame
    filtered_frame_0_matrix = processed_frame_0_matrix;
    FilterImage(prefilter, processed_frame_0_matrix, filtered_frame_0_matrix);

    cv::cvtColor(filtered_frame_0_matrix, filtered_frame_0_matrix_8bit_color,cv::COLOR_GRAY2RGB);

    Ptr<Tracker> tracker = TrackerMIL::create();
    Rect region_of_interest = cv::selectROI("ROI Selection", filtered_frame_0_matrix_8bit_color);

    frame_0_crop = frame_0_matrix(region_of_interest);
    cv::Point frame_0_point, frame_i_point;
    double peak_counts_0, peak_counts_old, peak_counts_i;
    cv::Scalar sum_counts_0, sum_ROI_counts_0, sum_counts_i, sum_ROI_counts_i;
    uint N_threshold_pixels_0, N_ROI_pixels_0, N_threshold_pixels_i, N_ROI_pixels_i;
    GetTrackFeatureData(trackFeature, threshold, frame_0_crop, frame_0_point, frame_crop_mean, peak_counts_0, sum_counts_0, sum_ROI_counts_0, N_threshold_pixels_0, N_ROI_pixels_0);
    peak_counts_i = peak_counts_old;

    u_int frame_0_x, frame_0_y, frame_i_x, frame_i_y;
    u_int indx, num_frames = stop_frame - start_frame + 1;
    arma::u64_mat output(num_frames, 10);

    GetPointXY(frame_0_point, region_of_interest,frame_0_x,frame_0_y);
    irradiance =  static_cast<uint32_t>(sum_counts_0[0] - frame_crop_mean[0]);
    output.row(0) = {track_id, frame0, frame_0_x, frame_0_y, static_cast<uint16_t>(peak_counts_0), static_cast<uint32_t>(sum_counts_0[0]), static_cast<uint32_t>(sum_ROI_counts_0[0]), N_threshold_pixels_0, N_ROI_pixels_0, static_cast<uint64_t>(irradiance)};

    tracker->init(filtered_frame_0_matrix_8bit_color,region_of_interest);

    cv::destroyWindow("ROI Selection");

    for (u_int i = 1; i < num_frames; i++) {

        if (cancel_operation)
        {
            cv::destroyAllWindows();
            return arma::u64_mat ();
        }

        UpdateProgressBar(i);
        indx = (indx0 + i);

        GetProcessedFrameMatrix(indx, clamp_low, clamp_high, original, frame_i_vector, frame_i_matrix, processed_frame_i_matrix);

        filtered_frame_i_matrix = processed_frame_i_matrix;
        FilterImage(prefilter, processed_frame_i_matrix, filtered_frame_i_matrix);

        cv::cvtColor(filtered_frame_i_matrix, filtered_frame_i_matrix_8bit_color,cv::COLOR_GRAY2RGB);

        bool ok = tracker->update(filtered_frame_i_matrix_8bit_color, region_of_interest);
        
        frame_i_crop = frame_i_matrix(region_of_interest);
   
        GetTrackFeatureData(trackFeature, threshold, frame_i_crop, frame_i_point, frame_crop_mean, peak_counts_i, sum_counts_i, sum_ROI_counts_i, N_threshold_pixels_i, N_ROI_pixels_i);

        if (ok && peak_counts_i >= .5 * peak_counts_old) {
            peak_counts_old = peak_counts_i;
            rectangle(filtered_frame_i_matrix_8bit_color, region_of_interest, cv::Scalar( 0, 0, 255 ), 2);
            imshow("Tracking... ", filtered_frame_i_matrix_8bit_color);
        }
        else {
            region_of_interest = selectROI("Track Lost. Select ROI again.", filtered_frame_i_matrix_8bit_color);
            tracker->init(filtered_frame_i_matrix_8bit_color, region_of_interest);
            cv::destroyWindow("Track Lost. Select ROI again.");
            frame_i_crop = frame_i_matrix(region_of_interest);
            GetTrackFeatureData(trackFeature, threshold, frame_i_crop, frame_i_point, frame_crop_mean, peak_counts_i, sum_counts_i, sum_ROI_counts_i, N_threshold_pixels_i, N_ROI_pixels_i);
            peak_counts_i = peak_counts_old;
        }
        
        GetPointXY(frame_i_point, region_of_interest, frame_i_x, frame_i_y);
        irradiance =  static_cast<uint32_t>(sum_counts_i[0] - frame_crop_mean[0]);
        output.row(i) = {track_id, frame0 + i, frame_i_x ,frame_i_y, static_cast<uint16_t>(peak_counts_i), static_cast<uint32_t>(sum_counts_i[0]),static_cast<uint32_t>(sum_ROI_counts_i[0]),N_threshold_pixels_i,N_ROI_pixels_i, static_cast<uint64_t>(irradiance)};
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
        N_threshold_pixels = cv::countNonZero(frame_crop_threshold > 0);
        cv::Moments frame_moments = cv::moments(frame_crop_threshold,true);
        cv::Point frame_temp_point(frame_moments.m10/frame_moments.m00, frame_moments.m01/frame_moments.m00);
        cv::minMaxLoc(frame_crop, NULL, &peak_counts, NULL, NULL);
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

void AutoTracking::GetProcessedFrameMatrix(int indx, double clamp_low, double clamp_high, VideoDetails original, arma::vec & frame_vector, cv::Mat & frame_matrix, cv::Mat & processed_frame_matrix)
{
    double m, s;
    int nrows = original.y_pixels;
    int ncols = original.x_pixels;      
    frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[indx]);
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
