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
    emit SignalProgress(val);
}

void AutoTracking::CancelOperation()
{
    cancel_operation = true;
}

// leverage OpenCV to track objects of interest
arma::u32_mat AutoTracking::SingleTracker(u_int track_id, int frame0, int start_frame, int stop_frame, VideoDetails original, QString new_track_file_name)
{
    int num_video_frames = original.frames_16bit.size();
    int nrows = original.y_pixels;
    int ncols = original.x_pixels;
    u_int indx0 = start_frame - 1;
    arma::vec image_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[indx0]);

    double m0, s0, m, s;
    m0 = arma::mean(image_vector);
    s0 = arma::stddev(image_vector);
    image_vector.clamp(m0 - 3*s0, m0 + 3*s0);
    image_vector = image_vector - image_vector.min();
    image_vector = 255*image_vector/image_vector.max();

    cv::Mat frame_matrix = cv::Mat(nrows, ncols, CV_64FC1, image_vector.memptr());
    cv::Mat frame_matrix_8bit, frame_matrix_filtered_8bit, frame_matrix_filtered_8bit_color;
    cv::Mat imCrop, imCrop_gray;
    frame_matrix.convertTo(frame_matrix_8bit, CV_8UC1);

    // attenuate image noise of initial frame
    cv::GaussianBlur(frame_matrix_8bit, frame_matrix_filtered_8bit, cv::Size(5,5), 0);
    cv::cvtColor(frame_matrix_filtered_8bit, frame_matrix_filtered_8bit_color,cv::COLOR_GRAY2RGB);

    Ptr<Tracker> tracker = TrackerMIL::create();
    Rect ROI = cv::selectROI("ROI Selection", frame_matrix_filtered_8bit_color);
    tracker->init(frame_matrix_filtered_8bit_color,ROI);

    u_int indx, num_frames = stop_frame - start_frame + 1;
    arma::u32_mat output(num_frames, 4);
    cv::destroyWindow("ROI Selection");

    for (u_int i = 0; i < num_frames; i++) {

        if (cancel_operation)
		{
            cv::destroyAllWindows();
            return arma::u32_mat ();
		}

        UpdateProgressBar(i);
        indx = (indx0 + i);

        arma::vec image_vector_i = arma::conv_to<arma::vec>::from(original.frames_16bit[indx]);
        m = arma::mean(image_vector_i);
        s = arma::stddev(image_vector_i);
        image_vector_i.clamp(m - 3*s, m + 3*s);
        image_vector_i = image_vector_i - image_vector_i.min();
        image_vector_i = 255*image_vector_i / image_vector_i.max();

        cv::Mat frame_matrix_i = cv::Mat(nrows, ncols, CV_64FC1, image_vector_i.memptr());

        cv::Mat frame_matrix_i_8bit, frame_matrix_i_filtered_8bit, frame_matrix_i_filtered_8bit_color;

        frame_matrix_i.convertTo(frame_matrix_i_8bit, CV_8UC1);

        // attenuate noise of frame i
        cv::GaussianBlur(frame_matrix_i_8bit, frame_matrix_i_filtered_8bit, cv::Size(5,5), 0);

        // convert to RGB so we can add a blue tracking rectangle later
        cv::cvtColor(frame_matrix_i_filtered_8bit, frame_matrix_i_filtered_8bit_color,COLOR_GRAY2RGB);
        
        bool ok = tracker->update(frame_matrix_i_filtered_8bit_color, ROI);

        if (ok) {
            rectangle(frame_matrix_i_filtered_8bit_color, ROI, cv::Scalar( 0, 0, 255 ), 2);
            imshow("tracker", frame_matrix_i_filtered_8bit_color);
        }
        else {
            ROI = selectROI(frame_matrix_i_filtered_8bit_color);
            tracker->init(frame_matrix_i_filtered_8bit_color, ROI);
        }
        waitKey(1);

        imCrop = frame_matrix_i_filtered_8bit(ROI);

        cv::Mat thr;
        cv::threshold(imCrop, thr, 100, 255, cv::THRESH_OTSU);
        cv::Moments m = cv::moments(thr,false);
        cv::Point p(m.m10/m.m00, m.m01/m.m00);
        u_int centerX, centerY;

        if (p.x > 0 && p.y > 0){
            centerX = round(p.x + ROI.x + 1);
            centerY = round(p.y + ROI.y + 1);
        }
        else
        {
            centerX = round(ROI.x + 0.5 * ROI.width + 1);
            centerY = round(ROI.y + 0.5 * ROI.height + 1);
        }
        output.row(i) = {track_id, frame0 + i, centerX ,centerY };
    }
    cv::destroyAllWindows();
    return output;
}
