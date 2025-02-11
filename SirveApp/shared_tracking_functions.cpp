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

void SharedTrackingFunctions::GetTrackPointData(string &trackFeature, int & threshold, cv::Mat & frame_crop, cv::Mat & raw_frame_crop, cv::Point & frame_point,double & peak_counts, cv::Scalar & sum_counts, cv::Scalar & sum_ROI_counts, uint & N_threshold_pixels,uint & Num_NonZero_ROI_Pixels)
{
    cv::Scalar raw_frame_crop_mean, raw_frame_crop_sigma;

    cv::Mat frame_crop_threshold, frame_crop_threshold_binary, raw_frame_crop_threshold;
    cv::minMaxLoc(frame_crop, NULL, NULL, NULL, & frame_point);
    sum_ROI_counts = cv::sum(raw_frame_crop);
    Num_NonZero_ROI_Pixels = cv::countNonZero(raw_frame_crop > 0);
    cv::minMaxLoc(raw_frame_crop, NULL, & peak_counts, NULL, NULL);

    raw_frame_crop.copyTo(raw_frame_crop_threshold, frame_crop_threshold_binary);

    sum_counts = cv::sum(raw_frame_crop_threshold);
    N_threshold_pixels = cv::countNonZero(raw_frame_crop_threshold > 0);

    if(trackFeature == "INTENSITY_WEIGHTED_CENTROID")
    {   
        cv::Moments frame_moments = cv::moments(frame_crop_threshold,false);
        cv::Point frame_temp_point(frame_moments.m10/frame_moments.m00, frame_moments.m01/frame_moments.m00);
        frame_point = frame_temp_point;
    }
    else if (trackFeature == "CENTROID")
    {
        cv::Moments frame_moments = cv::moments(frame_crop_threshold_binary,true);
        cv::Point frame_temp_point(frame_moments.m10/frame_moments.m00, frame_moments.m01/frame_moments.m00);
        frame_point = frame_temp_point;
    }
}

void SharedTrackingFunctions::CheckROI(cv::Rect & ROI, bool & valid_ROI)
{
    valid_ROI = (!ROI.empty() && !(ROI.width == 0 || ROI.height == 0));    
}

void SharedTrackingFunctions::GetPointXY(cv::Point input_point, cv::Rect ROI, u_int & centerX, u_int & centerY)
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

void SharedTrackingFunctions::GetFrameRepresentations(
                                                        uint & indx,
                                                        double & clamp_low_coeff,
                                                        double & clamp_high_coeff,
                                                        VideoDetails & current_processing_state,
                                                        VideoDetails & base_processing_details,
                                                        cv::Mat & frame,
                                                        string & prefilter,
                                                        cv::Mat & display_frame,
                                                        cv::Mat & clean_display_frame,
                                                        cv::Mat & raw_frame
                                                    )
{
    int numRows = SirveAppConstants::VideoDisplayHeight;
    int numCols = SirveAppConstants::VideoDisplayWidth; 
    cv::Scalar m, s;    
    std::vector<uint16_t> frame_vector = current_processing_state.frames_16bit[indx];
    cv::Mat tmp(numRows, numCols, CV_16UC1, frame_vector.data());
    tmp.convertTo(frame,CV_32FC1);
    cv::meanStdDev(frame, m, s);
    int clamp_low = m[0] - clamp_low_coeff*s[0];
    int clamp_high = m[0] + clamp_high_coeff*s[0];
    display_frame = cv::min(cv::max(frame, clamp_low), clamp_high);
    display_frame = display_frame - clamp_low;
    display_frame = 255*display_frame/(clamp_high - clamp_low);
    display_frame.convertTo(display_frame, cv::COLOR_GRAY2BGR);

    std::vector<uint16_t> raw_frame_vector = base_processing_details.frames_16bit[indx];
    cv::Mat tmp2(numRows, numCols, CV_16UC1, raw_frame_vector.data());
    tmp2.convertTo(raw_frame, CV_32FC1);  

    FilterImage(prefilter, display_frame, clean_display_frame);
}

void SharedTrackingFunctions::FilterImage(string & prefilter, cv::Mat & display_frame, cv::Mat & clean_display_frame)
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

void SharedTrackingFunctions::CreateOffsetMatrix(int start_frame, int stop_frame, processingState & state_details, arma::mat & offsets_matrix)
{
    arma::vec frame_indices = arma::regspace(start_frame,stop_frame);
    // std::vector<double> frame_indices_data(frame_indices.begin(), frame_indices.end());

    offsets_matrix.set_size(frame_indices.n_elem,3);
    offsets_matrix.fill(0);

    std::vector<std::vector<int>> offsets = state_details.offsets;

    arma::mat offsets_matrix_tmp(offsets.size(),3,arma::fill::zeros);
    for (int rowi = 0; rowi < offsets.size(); rowi++){
        offsets_matrix_tmp.row(rowi) = arma::conv_to<arma::rowvec>::from(offsets[rowi]);
    }
    // std::vector<double> offsets_matrix_tmp_data(offsets_matrix_tmp.begin(), offsets_matrix_tmp.end());

    for (int rowii = 0; rowii<frame_indices.size(); rowii++){
        arma::uvec kk = arma::find(offsets_matrix_tmp.col(0) == frame_indices(rowii) + 1,0,"first");
        // std::vector<uint> kk_data(kk.begin(), kk.end());

        if (!kk.is_empty()){
            offsets_matrix.row(rowii) = offsets_matrix_tmp.row(kk(0));
        }
    }
    offsets_matrix.shed_col(0);
    // std::vector<double> mat_data(offsets_matrix.begin(), offsets_matrix.end());
}