#include "auto_tracking.h"
#include <iostream>
#include <cstring>


AutoTracking::AutoTracking()
{
    cancel_operation = false;
}

AutoTracking::~AutoTracking() {
}


void AutoTracking::UpdateProgressBar(unsigned int val) {

    emit SignalProgress(val);
}

void AutoTracking::CancelOperation()
{
    cancel_operation = true;
}


void AutoTracking::SingleTracker(int frame0, int start_frame, int stop_frame, VideoDetails original)
{

    cv::Mat template_image;
    template_image.release();

    int nrows = original.y_pixels;
    int ncols = original.x_pixels;
   
    arma::vec image_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[start_frame - 1]);
    image_vector = image_vector - arma::mean(image_vector);
    image_vector.elem((arma::find(image_vector<0))).zeros();
    int num_video_frames = original.frames_16bit.size();
    cv::Mat frame_matrix = cv::Mat(nrows,ncols,CV_64FC1,image_vector.memptr());
    cv::Mat frame_matrix_blurred;
    // cv::GaussianBlur(frame_matrix, frame_matrix_blurred, cv::Size(5,5), 0);
    // cv:: Mat dst00;
    // cv::cvtColor(frame_matrix,dst00,cv::COLOR_GRAY2RGB);
    cv:: Mat dst00;
    frame_matrix.convertTo(dst00, CV_32FC1);
    cv::bilateralFilter (dst00, frame_matrix_blurred, 9, 75, 75 );
    cv::Mat dst;
    frame_matrix_blurred.convertTo(dst, CV_8UC1);
    Ptr<Tracker> tracker = TrackerMIL::create();
    cv:: Mat dst1;
    cv::cvtColor(dst,dst1,cv::COLOR_GRAY2RGB);
    Rect ROI = cv::selectROI(dst1);
    tracker->init(dst1,ROI);
    std::vector<TrackFrame> track_frames_from_auto;
    int num_frames = stop_frame - start_frame + 1;
    arma::u32_mat output(num_frames,4);
    for (u_int i = 0; i < num_frames; i++) {
        UpdateProgressBar(i);
        arma::vec image_vector_i = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
        image_vector_i = image_vector_i - arma::mean(image_vector_i);
        image_vector_i.elem((arma::find(image_vector_i<0))).zeros();
        cv::Mat frame_matrix_i = cv::Mat(nrows,ncols,CV_64FC1,image_vector_i.memptr());
        cv::Mat frame_matrix_i_blurred;
        cv::GaussianBlur(frame_matrix_i, frame_matrix_i_blurred, cv::Size(5,5), 0);
        cv:: Mat dst00i;
        frame_matrix_i.convertTo(dst00i, CV_32FC1);
        // cv::bilateralFilter (dst00i, frame_matrix_i_blurred, 9, 75, 75);
        cv::Mat dsti;
        frame_matrix_i_blurred.convertTo(dsti, CV_8UC1);
        // cv::bilateralFilter (frame_matrix_i, frame_matrix_i_blurred, 11, 11*2, 11/2 );
        cv:: Mat frame_matrix_ii;
        cv::cvtColor(dsti,frame_matrix_ii,cv::COLOR_GRAY2RGB);
        // Mat frame_matrix_i = Mat(nrows,ncols,CV_64FC1,out_vector_i.data());
        // cv::imshow("Frame", frame_matrix_i);
        bool ok = tracker->update(frame_matrix_ii,ROI);
        if(ok){
            rectangle(frame_matrix_ii, ROI, cv::Scalar( 0, 0, 255 ), 2);
            imshow("tracker",frame_matrix_ii);
        }
        else{
            ROI = selectROI(frame_matrix_ii);
            tracker->init(frame_matrix_ii,ROI);  
        }
        waitKey(1);
        u_int centerX = ROI.x + 0.5 * ROI.width;
        u_int centerY = ROI.y + 0.5 * ROI.height;
        output.row(i) = {99, i + frame0 , centerX ,centerY };
    }
    output.save("auto_track99.csv", arma::csv_ascii);
    cv::destroyAllWindows();

}
