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
arma::u32_mat AutoTracking::SingleTracker(u_int track_id, double clamp_low, double clamp_high, int threshold, string prefilter, string trackFeature, uint frame0, int start_frame, int stop_frame, VideoDetails original, QString new_track_file_name)
{
    int nrows = original.y_pixels;
    int ncols = original.x_pixels;
    double m0, s0, mi, si;
    cv::Scalar filtered_mean0, filtered_std0, filtered_meani, filtered_stdi;
    u_int indx0 = start_frame - 1;
    cv::Mat frame_matrix_filtered, frame_matrix_filtered_8bit, frame_matrix_filtered_8bit_color;
    cv::Mat imCrop0, imCrop;

    arma::vec image_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[indx0]);

    m0 = arma::mean(image_vector);
    s0 = arma::stddev(image_vector);
    image_vector.clamp(m0 - clamp_low*s0, m0 + clamp_high*s0);
    image_vector = image_vector - image_vector.min();
    image_vector = 255*image_vector/image_vector.max();
    cv::Mat frame_matrix = cv::Mat(nrows, ncols, CV_64FC1, image_vector.memptr());
    frame_matrix.convertTo(frame_matrix, CV_8UC1);

    // attenuate image noise of initial frame
    frame_matrix_filtered = frame_matrix;
    if(prefilter=="GAUSSIAN"){
        cv::GaussianBlur(frame_matrix, frame_matrix_filtered, cv::Size(5,5), 0);
    }
    else if(prefilter=="MEDIAN"){
        cv::medianBlur(frame_matrix, frame_matrix_filtered, 5);      
    }
    else if(prefilter=="NLMEANS"){
        cv::fastNlMeansDenoising(frame_matrix, frame_matrix_filtered);
    }

    cv::cvtColor(frame_matrix_filtered, frame_matrix_filtered_8bit_color,cv::COLOR_GRAY2RGB);

    Ptr<Tracker> tracker = TrackerMIL::create();
    Rect ROI = cv::selectROI("ROI Selection", frame_matrix_filtered_8bit_color);

    imCrop0 = frame_matrix_filtered(ROI);

    cv::Point pout0;
    if(trackFeature == "INTENSITY_WEIGHTED_CENTROID"){
        cv::Mat thr0;
        cv::meanStdDev(imCrop0,filtered_mean0, filtered_std0);
        cv::threshold(imCrop0, thr0, filtered_mean0[0]+threshold*filtered_std0[0], 255, cv::THRESH_TOZERO);
        cv::Moments mom0 = cv::moments(thr0,false);
        cv::Point p0(mom0.m10/mom0.m00, mom0.m01/mom0.m00);
        cv::cvtColor(thr0,thr0, cv::COLOR_GRAY2RGB);
        // cv::circle(thr0,p0,3, cv::Scalar(255,0,0),-1);
        // cv::imshow("thr0",thr0);
        pout0 = p0;
    }
    else if (trackFeature == "CENTROID"){
        cv::Mat thr0;
        cv::meanStdDev(imCrop0,filtered_mean0, filtered_std0);
        cv::threshold(imCrop0, thr0, filtered_mean0[0]+threshold*filtered_std0[0], 255, cv::THRESH_BINARY);
        cv::Moments mom0 = cv::moments(thr0,true);
        cv::Point p0(mom0.m10/mom0.m00, mom0.m01/mom0.m00);
        cv::cvtColor(thr0, thr0,cv::COLOR_GRAY2RGB);
        // cv::circle(thr0,p0,3,cv::Scalar(255,0,0),-1);
        // cv::imshow("THR0",thr0);
        pout0 = p0;
    }
    else{;
        cv::Point p0;
        cv::Mat thr0;
        cv::meanStdDev(imCrop0,filtered_mean0, filtered_std0);
        cv::threshold(imCrop0, thr0, filtered_mean0[0]+threshold*filtered_std0[0], 255, cv::THRESH_TOZERO);
        cv::minMaxLoc(thr0, NULL, NULL, NULL, &p0);
        cv::cvtColor(thr0, thr0,cv::COLOR_GRAY2RGB);
        // cv::circle(thr0,p0,3, cv::Scalar(255,0,0),-1);
        // cv::imshow("thr0",thr0);
        pout0 = p0;
    }

    u_int centerX0, centerY0;
    u_int indx, num_frames = stop_frame - start_frame + 1;
    arma::u32_mat output(num_frames, 4);

    if (pout0.x > 0 && pout0.y > 0){
        centerX0 = round(pout0.x + ROI.x);
        centerY0 = round(pout0.y + ROI.y);
    }
    else
    {
        centerX0 = round(ROI.x + 0.5 * ROI.width);
        centerY0 = round(ROI.y + 0.5 * ROI.height);
    }

    output.row(0) = {track_id, frame0, centerX0 ,centerY0};

    tracker->init(frame_matrix_filtered_8bit_color,ROI);

    cv::destroyWindow("ROI Selection");

    for (u_int i = 1; i < num_frames; i++) {

        if (cancel_operation)
		{
            cv::destroyAllWindows();
            return arma::u32_mat ();
		}

        UpdateProgressBar(i);
        indx = (indx0 + i);

        arma::vec image_vector_i = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
        mi = arma::mean(image_vector_i);
        si = arma::stddev(image_vector_i);
        image_vector_i.clamp(mi - clamp_low*si, mi + clamp_high*si);
        image_vector_i = image_vector_i - image_vector_i.min();
        image_vector_i = 255*image_vector_i/image_vector_i.max();
        cv::Mat frame_matrix_i = cv::Mat(nrows, ncols, CV_64FC1, image_vector_i.memptr());
        frame_matrix_i.convertTo(frame_matrix_i, CV_8UC1);
        cv::Mat frame_matrix_i_filtered, frame_matrix_i_filtered_8bit_color;

        // attenuate noise of frame i
        frame_matrix_i_filtered = frame_matrix_i;
        if(prefilter=="GAUSSIAN"){
            cv::GaussianBlur(frame_matrix_i, frame_matrix_i_filtered, cv::Size(5,5), 0);
        }
        else if(prefilter=="MEDIAN"){
            cv::medianBlur(frame_matrix_i, frame_matrix_i_filtered, 5);      
        }
        else if(prefilter=="NLMEANS"){
            cv::fastNlMeansDenoising(frame_matrix_i, frame_matrix_i_filtered);
        }

        // convert to RGB so we can add a blue tracking rectangle later
        cv::cvtColor(frame_matrix_i_filtered, frame_matrix_i_filtered_8bit_color,COLOR_GRAY2RGB);
        
        bool ok = tracker->update(frame_matrix_i_filtered_8bit_color, ROI);

        if (ok) {
            rectangle(frame_matrix_i_filtered_8bit_color, ROI, cv::Scalar( 0, 0, 255 ), 2);
            imshow("tracker", frame_matrix_i_filtered_8bit_color);
        }
        else {
            ROI = selectROI(frame_matrix_i_filtered_8bit_color);
            tracker->init(frame_matrix_i_filtered_8bit_color, ROI);
        }

        imCrop = frame_matrix_i_filtered(ROI);

    cv::Point pout;
    if(trackFeature == "INTENSITY_WEIGHTED_CENTROID"){
        cv::Mat thr;
        cv::meanStdDev(imCrop,filtered_meani, filtered_stdi);
        cv::threshold(imCrop, thr, filtered_meani[0]+threshold*filtered_stdi[0], 255, cv::THRESH_TOZERO);
        cv::Moments mom = cv::moments(thr,false);
        cv::Point p(mom.m10/mom.m00, mom.m01/mom.m00);
        cv::cvtColor(thr, thr, cv::COLOR_GRAY2RGB);
        // cv::circle(thr,p,3, cv::Scalar(255,0,0),-1);
        // cv::imshow("thr",thr);
        pout = p;
    }
    else if (trackFeature == "CENTROID"){
        cv::Mat thr;
        cv::meanStdDev(imCrop,filtered_meani, filtered_stdi);
        cv::threshold(imCrop, thr, filtered_meani[0]+threshold*filtered_stdi[0], 255, cv::THRESH_BINARY);
        cv::Moments mom = cv::moments(thr,true);
        cv::Point p(mom.m10/mom.m00, mom.m01/mom.m00);
        cv::cvtColor(thr, thr,cv::COLOR_GRAY2RGB);
        // cv::circle(thr,p,3,cv::Scalar(255,0,0),-1);
        // cv::imshow("THRi",thr);
        pout = p;
    }
    else{
        cv::Point p;
        cv::Mat thr;
        cv::meanStdDev(imCrop,filtered_meani, filtered_stdi);
        cv::threshold(imCrop, thr, filtered_meani[0]+threshold*filtered_stdi[0], 255, cv::THRESH_TOZERO);
        cv::minMaxLoc(thr, NULL, NULL, NULL, &p);
        cv::cvtColor(thr, thr,cv::COLOR_GRAY2RGB);
        // cv::circle(thr,p,3, cv::Scalar(255,0,0),-1);
        // cv::imshow("thr",thr);
        pout = p;
    }

        u_int centerX, centerY;

        if (pout.x > 0 && pout.y > 0){
            centerX = round(pout.x + ROI.x);
            centerY = round(pout.y + ROI.y);
        }
        else
        {
            centerX = round(ROI.x + 0.5 * ROI.width);
            centerY = round(ROI.y + 0.5 * ROI.height);
        }
        output.row(i) = {track_id, frame0 + i, centerX ,centerY};
        // waitKey(-1);
        waitKey(1);
    }
    cv::destroyAllWindows();
    return output;
}
