#include "shared_tracking_functions.h"

double SharedTrackingFunctions::GetAdjustedCounts(int indx, cv::Rect boundingBox, VideoDetails & base_processing_state_details)
{
    int number_median_frames = 30;
    double irradiance_val;
     
    int start_indx;
    start_indx = std::max(indx - number_median_frames,0);

    int nRows = SirveAppConstants::VideoDisplayHeight;
    int nCols = SirveAppConstants::VideoDisplayWidth;

    int row1 = std::max(boundingBox.y,0);
    int row2 = std::min(boundingBox.y + boundingBox.height, nCols);
    int col1 = std::max(boundingBox.x,0);
    int col2 = std::min(boundingBox.x + boundingBox.width, nRows);

    arma::cube data_cube(nCols, nRows, number_median_frames+1);

    for (unsigned int k = 0; k <= number_median_frames; ++k)
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
    arma::vec current_frame_subcube_as_column =  data_subcube.slice(number_median_frames).as_col();
    irradiance_val = std::round(arma::sum(current_frame_subcube_as_column - data_subcube_as_columns_median));

    return std::max(irradiance_val,0.0);

}

void SharedTrackingFunctions::FindTargetExtent(cv::Mat &  display_image, int threshold, cv::Rect & ROI, cv::Rect & bbox)
{
    int M = 20;

    cv::Mat mask;
    cv::Mat temp_image, output_image, output_image_resize, display_image_resize, display_image_threshold, display_image_threshold_resize;
    cv::Scalar display_image_mean, display_image_sigma;
    cv::Scalar m,s; 
    
    double  minVal, maxVal, minVal2, maxVal2, threshold_val;

    temp_image = display_image.clone();
    cv::meanStdDev(temp_image, m, s);
    int clamp_low = m[0] - 3*s[0];
    int clamp_high = m[0] + 3*s[0];
    temp_image = cv::min(cv::max(temp_image, clamp_low), clamp_high);
    temp_image = temp_image - clamp_low;
    temp_image = 255*temp_image/(clamp_high - clamp_low);
    temp_image.convertTo(temp_image, CV_8UC1);

    cv::minMaxLoc(temp_image, & minVal, & maxVal);
    threshold_val = maxVal * std::pow(10,-threshold/20.);
    cv::threshold(temp_image, display_image_threshold, threshold_val, 255, cv::THRESH_TOZERO);
    cv::resize(display_image_threshold, display_image_threshold_resize, cv::Size(10*display_image_threshold.cols, 10*display_image_threshold.rows));
    display_image_threshold_resize.convertTo(display_image_threshold_resize, cv::COLOR_GRAY2BGR);
    imshow("Thresholded ROI",display_image_threshold_resize);
    cv::moveWindow("Thresholded ROI", 700, 50);

    // Find contours of the blob
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(display_image_threshold, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

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
        output_image = temp_image.clone();
        cv::cvtColor(output_image, output_image, cv::COLOR_GRAY2BGR);
        cv::rectangle(output_image, bbox, cv::Scalar(0, 255, 0), 1);
          
        cv::resize(output_image, output_image_resize, cv::Size(M*output_image.cols, M*output_image.rows));
        cv::imshow("Blob Extent",output_image_resize);
        cv::moveWindow("Blob Extent", 1400, 50);

        bbox.x = ROI.x + bbox.x;
        bbox.y = ROI.y + bbox.y;

        // cv::waitKey(0);
    } else {
        std::cerr << "No contours found!" << std::endl;
        bbox = ROI;
    }
}