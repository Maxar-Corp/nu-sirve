#include "shared_tracking_functions.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/photo.hpp>

std::array<double, 3> SharedTrackingFunctions::CalculateIrradiance(int indx, cv::Rect boundingBox, const VideoDetails& base_processing_state_details, double frame_integration_time, const CalibrationData&
                                                                   model)
{
    std::array<double, 3> measurements = {0,0,0};
    int nRows = SirveAppConstants::VideoDisplayHeight;
    int nCols = SirveAppConstants::VideoDisplayWidth;

    int row1 = std::max(boundingBox.y,0);
    int row2 = std::min(boundingBox.y + boundingBox.height-1, nCols);
    int col1 = std::max(boundingBox.x,0);
    int col2 = std::min(boundingBox.x + boundingBox.width-1, nRows);

    arma::mat original_mat_frame = arma::reshape(arma::conv_to<arma::vec>::from(base_processing_state_details.frames_16bit[indx]),nCols,nRows).t(); 

    bool valid_indices = ((col2>col1) && (row2>row1) && (col1>0) && (col2>0) && (row1>0) && (row2>0));

    if (valid_indices)
    {
        arma::mat counts = original_mat_frame.submat(row1, col1, row2, col2);
        measurements = model.MeasureIrradiance(row1, col1, row2, col2, counts, frame_integration_time);
    }

    return measurements;
}

double SharedTrackingFunctions::GetAdjustedCounts(int indx, cv::Rect boundingBox, const VideoDetails& base_processing_state_details)
{
    int number_median_frames = std::min((int)base_processing_state_details.frames_16bit.size(), 30);
    double sum_relative_counts = 0;
     
    int start_indx;
    start_indx = std::max(indx - number_median_frames,0);

    int nRows = SirveAppConstants::VideoDisplayHeight;
    int nCols = SirveAppConstants::VideoDisplayWidth;

    int row1 = std::max(boundingBox.y,0);
    int row2 = std::min(boundingBox.y + boundingBox.height-1, nCols);
    int col1 = std::max(boundingBox.x,0);
    int col2 = std::min(boundingBox.x + boundingBox.width-1, nRows);

    arma::cube data_cube(nCols, nRows, number_median_frames+1);
    bool valid_indices = ((col2>col1) && (row2>row1) && (col1>0) && (col2>0) && (row1>0) && (row2>0));
    if (valid_indices)
    {
        for (unsigned int k = 0; k < number_median_frames; ++k)
        {
            data_cube.slice(k) = arma::reshape(arma::conv_to<arma::vec>::from(base_processing_state_details.frames_16bit[start_indx+k]),nCols,nRows); 
        }

        arma::cube data_subcube = data_cube.tube(col1,row1,col2,row2);

        int nPix = data_subcube.n_rows*data_subcube.n_cols;
        arma::mat data_subcube_as_columns(nPix, number_median_frames);
        for(unsigned int k = 0; k < number_median_frames; ++k)
        {
            data_subcube_as_columns.col(k) = data_subcube.slice(k).as_col();
        }
        arma::vec data_subcube_as_columns_median = arma::median(data_subcube_as_columns,1);
        arma::vec current_frame_subcube_as_column = data_subcube.slice(number_median_frames).as_col();
        sum_relative_counts = std::round(arma::sum(current_frame_subcube_as_column - data_subcube_as_columns_median));
    }

    return std::max(sum_relative_counts,0.0);

}

void SharedTrackingFunctions::FindTargetExtent(int i, double & clamp_low_coeff, double & clamp_high_coeff, cv::Mat & frame, int threshold, int bbox_buffer_pixels, cv::Mat & frame_crop_threshold, cv::Rect & ROI, cv::Rect & bbox, arma::mat & offsets_matrix, cv::Rect & bbox_uncentered, int & extent_window_x, int & extent_window_y)
{
    int resize_factor = 10;
    int nRows = SirveAppConstants::VideoDisplayHeight;
    int nCols = SirveAppConstants::VideoDisplayWidth;

    cv::Mat mask;
    cv::Mat temp_image, output_image, output_image_resize, frame_crop_resize, frame_crop_threshold_resize;
    cv::Scalar mean, sigma; 
    
    double threshold_val;

    cv::Mat frame_crop = frame(ROI);

    cv::meanStdDev(frame_crop, mean, sigma);
    int clamp_low = mean[0] - clamp_low_coeff*sigma[0];
    int clamp_high = mean[0] + clamp_high_coeff*sigma[0];
    temp_image = cv::min(cv::max(frame, clamp_low), clamp_high);
    cv::normalize(temp_image, temp_image, 0, 255, cv::NORM_MINMAX, CV_8U);

    temp_image = temp_image(ROI);
    threshold_val = 255. * std::pow(10,-threshold/20.);
    frame_crop_threshold = temp_image;
    cv::threshold(temp_image, frame_crop_threshold, threshold_val, 255, cv::THRESH_TOZERO);   
    frame_crop_threshold.convertTo(frame_crop_threshold,CV_8U);

    cv::resize(frame_crop_threshold, frame_crop_threshold_resize, cv::Size(1.5*resize_factor*frame_crop_threshold.cols, 1.5*resize_factor*frame_crop_threshold.rows));
    frame_crop_threshold_resize.convertTo(frame_crop_threshold_resize, cv::COLOR_GRAY2BGR);
    imshow("Thresholded ROI",frame_crop_threshold_resize);
    cv::moveWindow("Thresholded ROI", extent_window_x, extent_window_y);

    // Find contours of the blob
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(frame_crop_threshold, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

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

        bbox.x = std::max(bbox.x - bbox_buffer_pixels, 0);
        bbox.width = std::min((bbox.width + 2*bbox_buffer_pixels), ROI.width - bbox.x);
        bbox.y = std::max(bbox.y - bbox_buffer_pixels, 0);
        bbox.height = std::min((bbox.height + 2*bbox_buffer_pixels), ROI.height - bbox.y);

        // Draw the bounding rectangle
        output_image = temp_image.clone();
        cv::cvtColor(output_image, output_image, cv::COLOR_GRAY2BGR);
        cv::rectangle(output_image, bbox, cv::Scalar(0, 255, 0), 1);
          
        cv::resize(output_image, output_image_resize, cv::Size(resize_factor*output_image.cols, resize_factor*output_image.rows));
        cv::namedWindow("Target Extent", cv::WINDOW_NORMAL);
        cv::imshow("Target Extent",output_image_resize);
        cv::moveWindow("Target Extent", extent_window_x, extent_window_y + 1.5*resize_factor*frame_crop_threshold.rows + 50);
        frame_crop_threshold = frame_crop_threshold(bbox);
        bbox.x = ROI.x + bbox.x;
        bbox.y = ROI.y + bbox.y;

        // cv::waitKey(0);
    } else {
        std::cerr << "No contours found!" << std::endl;
        bbox = ROI;
    }
    //If frame is centered, finds the corresponding bbox in the original frame
    bbox_uncentered = bbox;
    bbox_uncentered.x += (offsets_matrix(i,0));
    if (bbox_uncentered.x<0)
    {
        bbox_uncentered.x += nCols;
    }
    if (bbox_uncentered.x>nCols)
    {
        bbox_uncentered.x -= nCols;
    }
    bbox_uncentered.y += (offsets_matrix(i,1));
    if (bbox_uncentered.y<0)
    {
        bbox_uncentered.y += nRows;
    }
    if (bbox_uncentered.y>nRows)
    {
        bbox_uncentered.y -= nRows;
    }
    if (bbox_uncentered.x + bbox_uncentered.width >= nCols)
    {
        bbox_uncentered.width = std::max(nCols - bbox_uncentered.x,1);
    }
    if (bbox_uncentered.y + bbox_uncentered.height >= nRows)
    {
        bbox_uncentered.height = std::max(nRows - bbox_uncentered.y,1);
    }
}

void SharedTrackingFunctions::GetTrackPointData(std::string & trackFeature, cv::Mat & frame_bbox, cv::Mat & raw_frame_bbox, cv::Mat & frame_bbox_threshold, cv::Point & frame_point, double & peak_counts, double & mean_counts, cv::Scalar & sum_counts, uint32_t & number_pixels)
{
    cv::Scalar mean, sigma;
    cv::Mat frame_bbox_threshold_binary, raw_frame_bbox_threshold;

    cv::minMaxLoc(frame_bbox, NULL, NULL, NULL, & frame_point); //Gets location of peak point in processed cropped frame
    
    cv::minMaxLoc(raw_frame_bbox, NULL, & peak_counts, NULL, NULL); //Get max value in original raw data

    if (cv::countNonZero(frame_bbox_threshold)>0)
    {
        raw_frame_bbox.copyTo(raw_frame_bbox_threshold, frame_bbox_threshold); //Copies the region defined by the thresholded processed frame from the raw frame to raw_frame_bbox_threshold
    }
    else
    {
        raw_frame_bbox_threshold = raw_frame_bbox.clone();
    }
    sum_counts = cv::sum(raw_frame_bbox_threshold);
    number_pixels = cv::countNonZero(raw_frame_bbox_threshold > 0);
    cv::meanStdDev(raw_frame_bbox_threshold, mean, sigma);
    mean_counts = mean[0];

    frame_bbox_threshold_binary.setTo(255,raw_frame_bbox_threshold !=0);

    if(trackFeature == "INTENSITY_WEIGHTED_CENTROID")
    {   
        cv::Moments frame_moments = cv::moments(raw_frame_bbox_threshold,false);
        cv::Point frame_temp_point(frame_moments.m10/frame_moments.m00, frame_moments.m01/frame_moments.m00);
        frame_point = frame_temp_point;
    }
    else if (trackFeature == "CENTROID")
    {
        cv::Moments frame_moments = cv::moments(frame_bbox_threshold_binary,true);
        cv::Point frame_temp_point(frame_moments.m10/frame_moments.m00, frame_moments.m01/frame_moments.m00);
        frame_point = frame_temp_point;
    }
    //Returns the frame point in the cropped image frame
}

void SharedTrackingFunctions::CheckROI(cv::Rect & ROI, bool & valid_ROI)
{
    valid_ROI = (!ROI.empty() && !(ROI.width == 0 || ROI.height == 0));    
}

void SharedTrackingFunctions::GetPointXY(cv::Point input_point, cv::Rect ROI,int & centerX,int & centerY)
{
    if (input_point.x > 0 && input_point.y > 0)
    {
        centerX = round(1000.0 *(input_point.x + ROI.x)/1000.0);
        centerY = round(1000.0 *(input_point.y + ROI.y)/1000.0);
    }
    else
    {
        centerX = round(1000.0 *(ROI.x + 0.5 * ROI.width)/1000.0);
        centerY = round(1000.0 *(ROI.y + 0.5 * ROI.height)/1000.0);
    }
}

void SharedTrackingFunctions::GetFrameRepresentations(
                                                       uint & indx,
                                                       double clamp_low_coeff,
                                                       double clamp_high_coeff,
                                                       const VideoDetails & current_processing_state,
                                                       const VideoDetails & base_processing_details,
                                                       cv::Mat & frame,
                                                       std::string & prefilter,
                                                       cv::Mat & display_frame,
                                                       cv::Mat & raw_display_frame,
                                                       cv::Mat & clean_display_frame,
                                                       cv::Mat & raw_frame
                                                   )
{
    int numRows = SirveAppConstants::VideoDisplayHeight;
    int numCols = SirveAppConstants::VideoDisplayWidth; 
    cv::Scalar mean, sigma;    
    std::vector<uint16_t> frame_vector = current_processing_state.frames_16bit[indx];
    cv::Mat tmp(numRows, numCols, CV_16UC1, frame_vector.data()); 
    tmp.convertTo(frame,CV_32FC1);

    cv::meanStdDev(frame, mean, sigma);
    int clamp_low = mean[0] - clamp_low_coeff*sigma[0];
    int clamp_high = mean[0] + clamp_high_coeff*sigma[0];
    display_frame = cv::min(cv::max(frame, clamp_low), clamp_high);
    cv::normalize(display_frame, display_frame, 0, 255, cv::NORM_MINMAX, CV_8U);
    display_frame.convertTo(display_frame, cv::COLOR_GRAY2BGR);

    std::vector<uint16_t> raw_frame_vector = base_processing_details.frames_16bit[indx];
    cv::Mat tmp2(numRows, numCols, CV_16UC1, raw_frame_vector.data());
    tmp2.convertTo(raw_frame, CV_32FC1);  

    cv::meanStdDev(raw_frame, mean, sigma);
    clamp_low = mean[0] - clamp_low_coeff*sigma[0];
    clamp_high = mean[0] + clamp_high_coeff*sigma[0];
    raw_display_frame = cv::min(cv::max(raw_frame, clamp_low), clamp_high);
    cv::normalize(raw_display_frame, raw_display_frame, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::cvtColor(raw_display_frame, raw_display_frame, cv::COLOR_GRAY2RGB);

    FilterImage(prefilter, display_frame, clean_display_frame);
}

void SharedTrackingFunctions::FilterImage(std::string & prefilter, cv::Mat & display_frame, cv::Mat & clean_display_frame)
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

void SharedTrackingFunctions::CreateOffsetMatrix(int start_frame, int stop_frame, const ProcessingState & state_details, arma::mat & offsets_matrix)
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
