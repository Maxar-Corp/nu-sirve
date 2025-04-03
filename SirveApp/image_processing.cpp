#include "image_processing.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

ImageProcessing::ImageProcessing()
{
    frameval = 0;

    cancel_operation = false;

    min_deinterlace_dist = 1.5;
    max_deinterlace_dist = 40;
    deinterlace_kernel_size = 3;
}

ImageProcessing::~ImageProcessing() {
}

void ImageProcessing::ReplacePixelsWithNeighbors(std::vector<std::vector<uint16_t>> & original_pixels, const std::vector<unsigned int>& bad_pixel_indices, int width_pixels)
{
    //For each frame, replaces any bad pixels with the mean of the value of 2 pixels above, below, left, and right
    //Other bad pixels are exempted from the calculation

    //Creating a set (for faster lookup) of bad pixel indices simplifies/speeds up some of this code
    std::set<int> bad_pixel_indices_set(bad_pixel_indices.begin(), bad_pixel_indices.end());

    for (auto frame = 0; frame < original_pixels.size(); frame++)
    {
        UpdateProgressBar(frame);
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return;
        }
        for (auto i = 0; i < bad_pixel_indices.size(); i++)
        {
            int bad_pixel_index = bad_pixel_indices[i];
            std::vector<int> neighbor_pixels;

            //Grab up to two pixels from the left, if possible
            int x_loc = bad_pixel_index % width_pixels;
            if (x_loc > 0)
            {
                neighbor_pixels.push_back(bad_pixel_index - 1);
                if (x_loc > 1)
                {
                    neighbor_pixels.push_back(bad_pixel_index - 2);
                }
            }

            //Grab up to two pixels from the right, if possible
            if (x_loc < width_pixels - 1)
            {
                neighbor_pixels.push_back(bad_pixel_index + 1);
                if (x_loc < width_pixels - 2)
                {
                    neighbor_pixels.push_back(bad_pixel_index + 2);
                }
            }

            //Grab up to two pixels from above, if possible
            if (bad_pixel_index - width_pixels >= 0)
            {
                neighbor_pixels.push_back(bad_pixel_index - width_pixels);
                if (bad_pixel_index - 2*width_pixels >= 0)
                {
                    neighbor_pixels.push_back(bad_pixel_index - 2*width_pixels);
                }
            }

            //Grab up to two pixels from below, if possible
            if (bad_pixel_index + width_pixels < original_pixels[frame].size())
            {
                neighbor_pixels.push_back(bad_pixel_index + width_pixels);
                if (bad_pixel_index + 2*width_pixels < original_pixels[frame].size())
                {
                    neighbor_pixels.push_back(bad_pixel_index + 2*width_pixels);
                }
            }

            //Determine the neighboring pixels' mean value to replace the pixel with
            int running_sum = 0;
            int other_bad_pixels_to_skip = 0;

            for (auto j = 0; j < neighbor_pixels.size(); j++)
            {
                if (bad_pixel_indices_set.count(neighbor_pixels[j]))
                {
                    other_bad_pixels_to_skip += 1;
                }
                else
                {
                    running_sum += original_pixels[frame][neighbor_pixels[j]];
                }
            }

            //The mean neighboring value is {sum of neighbor values}/{count of neighbors}
            //We have to remember that we skipped any neighbors that are also bad pixels
            double mean_value = 1.0 * running_sum / (neighbor_pixels.size() - other_bad_pixels_to_skip);
            int mean = static_cast<int>(std::round(mean_value));

            //Replace the bad pixel
            original_pixels[frame][bad_pixel_index] = mean;
        }
    }
}

arma::uvec ImageProcessing::IdentifyBadPixelsMedian(double N, const std::vector<std::vector<uint16_t>>& input_pixels)
{
    double c = 1.4826;

    int num_video_frames = input_pixels.size();

    int num_pixels = input_pixels[0].size();
    num_video_frames = std::min(num_video_frames,500);

    // Create an Armadillo matrix
    arma::mat frame_data(num_pixels, num_video_frames);
    arma::uvec index_outlier;
    index_outlier.reset();

    // Fill the Armadillo matrix from the std::vector
    for (int i = 0; i < num_video_frames; i++) {
        if (cancel_operation)
        {
            return index_outlier;
        }
        frame_data.col(i) = arma::conv_to<arma::vec>::from(input_pixels[i]);
    }
    UpdateProgressBar(round(num_video_frames/4));
    arma::vec med_frame = arma::median(frame_data,1);
    arma::mat med_frame_M = arma::repmat(med_frame,1,frame_data.n_cols);

    UpdateProgressBar(round(num_video_frames/2));

    arma::mat diff_from_med = arma::abs(frame_data - med_frame_M);
    arma::vec MAD = c*arma::median(diff_from_med,1);
    arma::mat MADM = arma::repmat(MAD,1,frame_data.n_cols);
    arma::umat OUTL = diff_from_med > 3*MADM;
    arma::uvec SUMB = arma::sum(OUTL,1);

    UpdateProgressBar(round(3*num_video_frames/4));

    arma::vec P = arma::conv_to<arma::vec>::from(SUMB);
    index_outlier = arma::find(arma::abs(P - arma::mean(P)) > N*arma::stddev(P));

    UpdateProgressBar(num_video_frames);

    return index_outlier;
}

arma::uvec ImageProcessing::IdentifyBadPixelsMovingMedian(int half_window_length, double N, const std::vector<std::vector<uint16_t>>& input_pixels)
{
    int start_frame_index, stop_frame_index;
    int num_video_frames = input_pixels.size();
    int num_pixels = input_pixels[0].size();
    double c = 1.4826;

    // Create an Armadillo matrix
    arma::mat frame_data(num_pixels, num_video_frames);

    // Fill the Armadillo matrix from the std::vector
    for (int i = 0; i < num_video_frames; i++) {
        frame_data.col(i) = arma::conv_to<arma::vec>::from(input_pixels[i]);
    }

    arma::mat MAD(num_pixels, num_video_frames);
    arma::mat moving_median(num_pixels, num_video_frames);
    arma::umat OUTL(num_pixels, num_video_frames);

    arma::uvec index_outlier;
    index_outlier.reset();
    for (int i = 0; i < num_video_frames; i++)
    {
        UpdateProgressBar(i);
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return index_outlier;
        }
        start_frame_index = std::max(i - (half_window_length),0);
        stop_frame_index = std::min(i + (half_window_length),num_video_frames - 1);
        moving_median.col(i) = arma::median(frame_data.cols(start_frame_index,stop_frame_index),1);
        MAD.col(i) = c*arma::median(arma::abs(frame_data.cols(start_frame_index,stop_frame_index).each_col() - moving_median.col(i)),1);
    }

    OUTL = arma::abs(frame_data - moving_median) > 3*MAD;
    arma::uvec SUMB = arma::sum(OUTL,1);

    arma::vec P = arma::conv_to<arma::vec>::from(SUMB);
    index_outlier = arma::find(arma::abs(P - arma::mean(P)) > N*arma::stddev(P));

    return index_outlier;
}

arma::uvec ImageProcessing::FindDeadBadscalePixels(const std::vector<std::vector<uint16_t>>& input_pixels)
{
    int num_video_frames = input_pixels.size();

    int num_pixels = input_pixels[0].size();

    // Create an Armadillo matrix
    arma::mat frame_data(num_pixels, num_video_frames);

    // Fill the Armadillo matrix from the std::vector
    for (int i = 0; i < num_video_frames; i++)
    {
        frame_data.col(i) = arma::conv_to<arma::vec>::from(input_pixels[i]);
    }
    UpdateProgressBar(round(num_video_frames/4));
    arma::vec std_frame = arma::stddev(frame_data,0,1);
    arma::vec var_frame = arma::var(frame_data,0,1);
    double jj = 0;
    arma::uvec index_dead = arma::find(var_frame <= arma::mean(var_frame) - jj*arma::stddev(var_frame));

    while (index_dead.size() > 0.0005 * num_pixels)
    {
        jj += .1;
        index_dead = arma::find(var_frame <= arma::mean(var_frame) - jj*arma::stddev(var_frame));
    }

    UpdateProgressBar(round(num_video_frames/2));
    arma::vec mean_frame = arma::mean(frame_data,1);
    double mean_mean_frame = arma::mean(mean_frame);
    double std_mean_frame = arma::stddev(mean_frame);

    double kk = 1;
    arma::uvec index_bad_scale = arma::find(arma::abs(mean_frame - mean_mean_frame) > kk*std_mean_frame);
    UpdateProgressBar(round(3*num_video_frames/4));

    while (index_bad_scale.size() > 0.0005 * num_pixels)
    {
        kk += .1;
        index_bad_scale = arma::find(arma::abs(mean_frame - mean_mean_frame) > kk*std_mean_frame);
    }

    index_dead = arma::unique(arma::join_vert(index_bad_scale,index_dead));
    UpdateProgressBar(num_video_frames);

    return index_dead;
}

std::vector<std::vector<uint16_t>> ImageProcessing::FixedNoiseSuppression(const QString& image_path,
    const QString& path_video_file, int frame0, int start_frame, int stop_frame, double version,
    const VideoDetails & original)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int num_pixels = original.frames_16bit[0].size();
    int number_avg_frames = stop_frame - start_frame + 1;
    int index_start_frame, index_stop_frame;
    index_start_frame = start_frame - frame0;
    index_stop_frame = stop_frame  - frame0;

    video_frames_16bit = original.frames_16bit;

    int compare = QString::compare(path_video_file, image_path, Qt::CaseInsensitive);
    if (compare!=0)
    {
        //Read External
        QByteArray array = image_path.toLocal8Bit();
        char* buffer = array.data();

        ABIRReader reader;
        if (!reader.Open(buffer, version))
        {
            return frames_out;
        }

        auto frames = reader.ReadFrames(start_frame, stop_frame, false);
        video_frames_16bit = std::move(frames->video_frames_16bit);
    }

    // Create an Armadillo matrix for submatrix average
    arma::mat window_data(num_pixels, number_avg_frames);

    // Fill the Armadillo matrix from the std::vector
    int k = 0;
    for (int i = index_start_frame; i < index_stop_frame; i++){
        window_data.col(k) = arma::conv_to<arma::vec>::from(video_frames_16bit[i]);
        k += 1;
    }

    arma::vec median_frame;

    median_frame = arma::median(window_data, 1);

    double M;
    arma::vec frame_vector(num_pixels, 1);

    //Loop through frames to subtract median
    for (int i = 0; i < num_video_frames; i++){
        UpdateProgressBar(i);
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }
        frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
        M = frame_vector.max();
        frame_vector -= median_frame;
        frame_vector -= frame_vector.min();
        frame_vector = M * frame_vector / frame_vector.max();
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector));
    }

    return frames_out;
}

std::vector<std::vector<uint16_t>> ImageProcessing::AdaptiveNoiseSuppressionByFrame(int start_frame, int num_of_averaging_frames, const VideoDetails& original)
{
    int num_video_frames = original.frames_16bit.size();
    int num_pixels = original.frames_16bit[0].size();
    int nRows = original.y_pixels;
    int nCols = original.x_pixels;
    int start_frame_index, stop_frame_index, abs_start_frame;
    double M;
    int N2 = num_of_averaging_frames/2;
    abs_start_frame = std::abs(start_frame);
    std::vector<std::vector<uint16_t>> frames_out;
    arma::mat window_data(num_pixels,num_of_averaging_frames);
    int abs_start_frame_new = abs_start_frame+N2;
    arma::mat adjusted_window_data(num_pixels,abs_start_frame_new);
    adjusted_window_data.fill(0.0);
    arma::vec moving_median(num_pixels, 1);
    arma::vec frame_vector(num_pixels,1);
    arma::vec frame_vector_out(num_pixels,1);
    int num_indices = std::max(N2,1);

    for (int j = 0; j < num_of_averaging_frames; j++) {
        window_data.col(j)  = arma::conv_to<arma::vec>::from(original.frames_16bit[j]);
    }

    for (int i = 0; i < num_video_frames; i++) {

        UpdateProgressBar(i);
        QCoreApplication::processEvents();

        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }

        frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
        M = arma::max(frame_vector);
        start_frame_index = std::max(i + start_frame - N2,0);
        stop_frame_index = std::min(start_frame_index + N2 - 1, num_video_frames - 1);

        if (i>abs_start_frame_new)
        {
            window_data.insert_cols(window_data.n_cols,arma::conv_to<arma::vec>::from(original.frames_16bit[stop_frame_index]));
            window_data.shed_col(0);
        }

        arma::uvec rindices = arma::unique(arma::randi<arma::uvec>(num_indices,arma::distr_param(0,num_of_averaging_frames-1)));

        if(num_indices>1){
            moving_median = arma::median(window_data.cols(rindices), 1);
        }
        else{
            moving_median = window_data.col(0);
        }

        frame_vector -= moving_median;

        frame_vector -= frame_vector.min();

        frame_vector_out = M * frame_vector / frame_vector.max();
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector_out));
    }

    return frames_out;
}

std::vector<std::vector<uint16_t>> ImageProcessing::AdaptiveNoiseSuppressionMatrix(int start_frame, int num_of_averaging_frames, const VideoDetails& original)
{
    int num_video_frames = original.frames_16bit.size();
    int num_pixels = original.frames_16bit[0].size();
    int N2 = num_of_averaging_frames/2;
    int num_indices = std::max(N2,1);
    int si, fi;
    arma::mat adjusted_window_data(num_pixels,num_of_averaging_frames);
    arma::mat frame_data(num_pixels,num_video_frames);

    for (int i = 0; i < num_video_frames; i++) {
        frame_data.col(i) = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }
    }

    arma::rowvec M = arma::max(frame_data,0);

    arma::mat moving_median(num_pixels, num_video_frames);
    for (int j = 0; j < num_video_frames; j++)
    {
        UpdateProgressBar(std::round(j));
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }
        si = std::max(0,j-N2);
        fi = std::min(num_video_frames-1,j+N2);
        arma::uvec rindices = arma::unique(arma::randi<arma::uvec>(num_indices,arma::distr_param(si,fi)));
        moving_median.col(j) = arma::median(frame_data.cols(rindices), 1);
    }

    frame_data -= arma::shift(moving_median,-start_frame,1);
    arma::vec frame_vector(num_pixels,1);
    std::vector<std::vector<uint16_t>> frames_out;
    for (int k = 0; k < num_video_frames; k++){
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }
        frame_vector = frame_data.col(k);
        frame_vector -= frame_vector.min();
        frame_vector = M(k) * frame_vector / frame_vector.max();
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector));
    }

    cv::destroyAllWindows();
    return frames_out;
}

std::vector<std::vector<uint16_t>> ImageProcessing::RPCPNoiseSuppression(const VideoDetails& original)
{
    std::vector<std::vector<uint16_t>> frames_out;
    int num_video_frames = original.frames_16bit.size();
    int num_pixels = original.frames_16bit[0].size();
    double lambda = 1/sqrt(std::max(num_pixels,num_video_frames));
    arma::mat M(num_pixels,num_video_frames);
    arma::vec frame_vector(num_pixels,1);
    M.zeros();

    for (int j = 0; j < num_video_frames; j++)
    {
        UpdateProgressBar(round(j/4));
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }
        M.col(j)  = arma::conv_to<arma::vec>::from(original.frames_16bit[j]);
    }

    arma::rowvec frame_max_vals = arma::max(M,0);
    double mu = num_pixels*num_video_frames/(4*arma::norm(M,1));
    double muinv = 1/mu;
    double tol = 1e-7;
    double M_Frob_Norm = norm(M,"fro");
    double convg_val = M_Frob_Norm * tol;
    bool converged = false;
    double lambda_mu = lambda*mu;
    int k = 0, kMax = 100;
    arma::mat L(M);
    L.zeros();
    arma::mat S(M);
    S.zeros();
    arma::mat Y(M);
    Y.zeros();

    double minimization_quantity;
    UpdateProgressBar(round(num_video_frames/2));
    int k0 = round(num_video_frames/2);

    while (!converged && k<kMax)
    {
        UpdateProgressBar(k0 + k);
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }
        L = perform_thresholding(M - S - muinv*Y, mu);
        S = apply_shrinkage_operator(M - L + muinv*Y, lambda_mu);
        Y = Y + mu*(M - L - S);
        minimization_quantity = arma::norm(M - L - S,"fro");
        if(minimization_quantity <= convg_val){
            converged = true;
        }
        k += 1;
    }

    int L0 = round(3*num_video_frames/4);
    UpdateProgressBar(L0);
    for(int kk = 0; kk < num_video_frames; kk++)
    {
        UpdateProgressBar(L0 + kk);
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }
        frame_vector = S.col(kk);
        frame_vector = frame_vector - frame_vector.min();
        frame_vector = frame_max_vals(kk) * frame_vector/frame_vector.max();
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector));
    }

    UpdateProgressBar(num_video_frames);
    return frames_out;
}

arma::mat ImageProcessing::perform_thresholding(arma::mat X, double tau)
{
    arma::mat U;
    arma::vec s;
    arma::mat V;
    arma::svd_econ(U,s,V,X);
    arma::mat S = arma::diagmat(s);
    arma::mat ST = apply_shrinkage_operator(S ,tau);
    arma:: mat D = U * ST * V.t();

    return D;
}

arma::mat ImageProcessing::apply_shrinkage_operator(arma::mat s, double tau)
{
    arma::mat z(s);
    arma::mat st = arma::sign(s) % arma::max(arma::abs(s)-tau, z.zeros());

    return st;
}

std::vector<std::vector<uint16_t>> ImageProcessing::AccumulatorNoiseSuppression(double weight, int offset, int NThresh, const VideoDetails& original, bool hide_shadow_choice)
{
    std::vector<std::vector<uint16_t>> frames_out;
    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels;
    int nCols = original.x_pixels;
    int offseti;
    double min0, max0, min, max;
    std::vector<uint16_t> frame_out;
    cv::Mat accumulator, foreground, foregroundn, frame_32FC1, frame_offset_32FC1, foreground_16UC1, foreground_64FC1;
    accumulator = cv::Mat::zeros(nRows,nCols, CV_32FC1);

    for (int framei = 0; framei < num_video_frames; framei++)
    {
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }
        std::vector<uint16_t> vec = {original.frames_16bit[framei].begin(),original.frames_16bit[framei].end()};
        if (offset<0){
            offseti = std::max(framei+offset,0);
        }
        else{
            offseti = std::min(framei+offset,num_video_frames-1);
        }
        std::vector<uint16_t> vecold = {original.frames_16bit[offseti].begin(),original.frames_16bit[offseti].end()};
        cv::Mat frame(nRows, nCols, CV_16UC1, vec.data());
        cv::Mat frameold(nRows, nCols, CV_16UC1, vecold.data());
        cv::minMaxLoc(frame,&min0,&max0);
        frame.convertTo(frame_32FC1, CV_32FC1);
        frameold.convertTo(frame_offset_32FC1, CV_32FC1);
        cv::accumulateWeighted(frame_offset_32FC1, accumulator, weight);
        cv::subtract(frame_32FC1, accumulator, foreground);

        if(hide_shadow_choice){
            foreground.convertTo(foreground_64FC1,CV_64FC1);
            arma::mat arma_frame( reinterpret_cast<double*>(foreground_64FC1.data), foreground_64FC1.cols, foreground_64FC1.rows );
            arma::vec frame_vector = arma::vectorise(arma_frame);
            ImageProcessing::remove_shadow(nRows, nCols, frame_vector, NThresh);
            cv::Mat foreground_64FC1 = cv::Mat(nRows, nCols, CV_64FC1, frame_vector.memptr());
            foregroundn = max0*foreground_64FC1;
        }
        else{
            cv::minMaxLoc(foreground,&min,&max);
            foregroundn = max0*(foreground - min)/(max - min);
        }

        foregroundn.convertTo(foreground_16UC1,CV_16UC1);
        frame_out.assign(foreground_16UC1.begin<uint16_t>(), foreground_16UC1.end<uint16_t>());
        frames_out.push_back(frame_out);
    }
    return frames_out;
}

void ImageProcessing::remove_shadow(int nRows, int nCols, arma::vec & frame_vector, int NThresh)
{
    double MEAN, SIGMA;
    int gs = 0;
    cv::Scalar m, s, m_old, s_old;
    cv::Size g(3,3);
    cv::Mat frame_out;
    cv::Mat SE_DILATE = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(5,5));
    cv::Mat SE_ERODE = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(3,3));
    cv::Mat SE_close = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(7,7));

    cv::Mat frame_matrix_filtered, frame_matrix_filtered_threshold, frame_matrix_morph_closed, frame_matrix_morph_dilated;
    cv::Mat frame_matrix = cv::Mat(nRows, nCols, CV_64FC1, frame_vector.memptr());

    cv::GaussianBlur(frame_matrix, frame_matrix_filtered, g, gs);
    cv::meanStdDev(frame_matrix_filtered, m, s);
    cv::threshold(frame_matrix_filtered, frame_matrix_filtered_threshold, m[0]-(NThresh)*s[0], 1, cv::THRESH_BINARY_INV);
    cv::morphologyEx(frame_matrix_filtered_threshold,frame_matrix_filtered_threshold,cv::MORPH_CLOSE, SE_close, cv::Point(-1,-1));
    cv::morphologyEx(frame_matrix_filtered_threshold,frame_matrix_filtered_threshold,cv::MORPH_DILATE, SE_DILATE, cv::Point(-1,-1), 2);
    cv::morphologyEx(frame_matrix_filtered_threshold,frame_matrix_filtered_threshold,cv::MORPH_ERODE, SE_ERODE, cv::Point(-1,-1),1);

    // Can we get rid of this? :
    // cv::imshow("Negative",frame_matrix_filtered_threshold);

    std::vector<cv::Point> index_change;
    cv::findNonZero(frame_matrix_filtered_threshold,index_change);

    arma::uvec index_change_arma(index_change.size(),1);
    for (int j = 0; j < index_change.size(); j++){
        index_change_arma(j) = nCols*index_change.at(j).y + index_change.at(j).x;
    }
    frame_matrix.convertTo(frame_matrix,CV_64FC1);;
    if(index_change_arma.size()>0){
        arma::uvec index_other = arma::find(arma::abs(frame_vector) <= arma::mean(frame_vector.as_col())+3*arma::stddev(frame_vector.as_col()));
        if(index_other.n_elem>0){
            MEAN = arma::mean(frame_vector.elem(index_other));
            SIGMA = arma::stddev(frame_vector.elem(index_other));
            if (SIGMA!=0){
                arma::uvec rindices = arma::randi<arma::uvec>(index_change_arma.size(),arma::distr_param(0,index_other.n_elem-1));
                arma::vec v = arma::randn<arma::vec>(index_other.size(),arma::distr_param(MEAN,SIGMA));
                frame_vector.elem(index_change_arma) = v.elem(rindices);
            }
        }
        else{
            MEAN = arma::mean(frame_vector);
            SIGMA = arma::stddev(frame_vector);
            if (SIGMA!=0){
                arma::uvec rindices = arma::randi<arma::uvec>(index_change_arma.size(),arma::distr_param(0,frame_vector.n_elem-1));
                arma::vec v = arma::randn<arma::vec>(frame_vector.size(),arma::distr_param(MEAN,SIGMA));
                frame_vector.elem(index_change_arma) = v.elem(rindices);
            }
        }
    }
    frame_vector -= frame_vector.min();
    frame_vector /= frame_vector.max();
}

std::vector<std::vector<uint16_t>>ImageProcessing::DeinterlaceOpenCVPhaseCorrelation(const VideoDetails & original)
{
    std::vector<std::vector<uint16_t>> frames_out;
    int num_video_frames = original.frames_16bit.size();

    for (int framei = 0; framei < num_video_frames; framei++)
    {
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }
        std::vector<uint16_t> frame = original.frames_16bit[framei];
        frames_out.push_back(DeinterlacePhaseCorrelationCurrent(framei, frame));
    }

    return frames_out;
}

std::vector<uint16_t> ImageProcessing::DeinterlacePhaseCorrelationCurrent(int framei, const std::vector<uint16_t>& current_frame_16bit) const
{
    int yOffset, xOffset;

    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
    arma::uvec odd_rows = arma::regspace<arma::uvec>(0, 2, nRows - 1);
    arma::uvec even_rows = arma::regspace<arma::uvec>(1, 2, nRows);
    arma::mat odd_frame(nRows2,nCols);
    arma::mat even_frame(nRows2,nCols);

    frame = arma::reshape(arma::conv_to<arma::vec>::from(current_frame_16bit),nCols,nRows);
    output = frame;
    odd_frame = frame.cols(odd_rows);
    even_frame = frame.cols(even_rows);

    cv::Mat source( nRows2, nCols, CV_64FC1, even_frame.memptr() );
    cv::Mat source_blurred;
    cv::GaussianBlur(source, source_blurred, cv::Size(deinterlace_kernel_size, deinterlace_kernel_size), 0);
    cv::Mat target( nRows2, nCols, CV_64FC1, odd_frame.memptr() );
    cv::Mat target_blurred;
    cv::GaussianBlur(target, target_blurred, cv::Size(deinterlace_kernel_size, deinterlace_kernel_size), 0);
    cv::Point2d shift = cv::phaseCorrelate(target_blurred, source_blurred);

    yOffset = shift.y;
    xOffset = shift.x;
    double d = sqrt(pow(xOffset,2) + pow(yOffset,2));
    if (d < max_deinterlace_dist && d > min_deinterlace_dist)
    {
        cv::Mat H = (cv::Mat_<float>(2, 3) << 1.0, 0.0, -shift.x/2, 0.0, 1.0, -shift.y/2);
        cv::Mat res;
        warpAffine(source, res, H, target_blurred.size(),cv::INTER_AREA + cv::WARP_FILL_OUTLIERS);
        cv::Mat H2 = (cv::Mat_<float>(2, 3) << 1.0, 0.0, shift.x/2, 0.0, 1.0, shift.y/2);
        cv::Mat res2;
        warpAffine(target, res2, H2, source_blurred.size(),cv::INTER_AREA + cv::WARP_FILL_OUTLIERS);
        arma::mat arma_mat_source( reinterpret_cast<double*>(res.data), res.cols, res.rows );
        arma::mat arma_mat_target( reinterpret_cast<double*>(res2.data), res2.cols, res2.rows );
        output.cols(odd_rows) = arma_mat_target;
        output.cols(even_rows) = arma_mat_source;
    }

    output = output - arma::min(output.as_col());
    return arma::conv_to<std::vector<uint16_t>>::from(output.as_col());
}


void ImageProcessing::TranslateFrameByOffsetsManual(TrackDetails &td, arma::mat &frame, bool &cont_search, int &framei, int &xOffset, arma::mat &output, std::vector<std::vector<int>>& track_centered_offsets, int &yOffset, int xOffset_correction, int yOffset_correction)
{
    yOffset = td.centroid_y - yOffset_correction;
    xOffset = td.centroid_x- xOffset_correction;
    output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
    track_centered_offsets.push_back({framei+1,xOffset,yOffset});
    cont_search = false;
}

void ImageProcessing::TranslateFramesByOffsetOsm(int &yOffset, std::vector<std::vector<int>>& track_centered_offsets, bool &cont_search, arma::mat &frame, int &xOffset, arma::mat &output, int &framei, TrackDetails &td)
{
    yOffset = td.centroid_y;
    xOffset = td.centroid_x;
    output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
    track_centered_offsets.push_back({framei+1,xOffset,yOffset});
    cont_search = false;
}

std::vector<std::vector<uint16_t>> ImageProcessing::CenterOnTracks(const QString& trackTypePriority,
    const VideoDetails& original, int OSM_track_id, int manual_track_id, const std::vector<TrackFrame>& osmFrames,
    const std::vector<TrackFrame>& manualFrames, boolean findAnyTrack, std::vector<std::vector<int>> & track_centered_offsets)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels, yOffset_correction = nRows/2;
    int nCols = original.x_pixels, xOffset_correction = nCols/2;
    int yOffset, xOffset;
    int OSMPriority = QString::compare(trackTypePriority, "OSM", Qt::CaseInsensitive);
    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
    std::vector<TrackFrame> trackFrames = manualFrames;
    bool cont_search;

    // Recenter the cube, frame by frame, for both non-OSM ('manual') tracks, and for the OSM track (track_id==0).
    for (int framei = 0; framei < num_video_frames; framei++)
    {
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }

        frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();
        output = frame;

        if (OSMPriority == 0) //OSM tracks have priority
        {
            cont_search = true;
            std::map<int, TrackDetails> trackMap = osmFrames[framei].tracks;

            auto it = trackMap.find(OSM_track_id);
            if(OSM_track_id>0 && it != trackMap.end()) //Specific track id
            {
                TrackDetails td = it->second;
                if (td.number_pixels != 0)
                {
                    TranslateFramesByOffsetOsm(yOffset, track_centered_offsets, cont_search, frame, xOffset, output, framei, td);
                }
            }
            else //Search for first track
            {
                auto it = trackMap.begin();
                while (cont_search && it != trackMap.end())
                {
                    TrackDetails td = it->second;
                    if (td.number_pixels != 0)
                    {
                        TranslateFramesByOffsetOsm(yOffset, track_centered_offsets, cont_search, frame, xOffset, output, framei, td);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            trackMap = manualFrames[framei].tracks;

            if(cont_search && findAnyTrack) //Now search for manual tracks
            {
                auto it = trackMap.find(manual_track_id);
                if(manual_track_id > 0 && it != trackMap.end()) //Specific track id
                {
                    TrackDetails td = it->second;
                    if (td.number_pixels != 0)
                    {
                        TranslateFrameByOffsetsManual(td, frame, cont_search, framei, xOffset, output, track_centered_offsets, yOffset, xOffset_correction, yOffset_correction);
                    }
                }
                else
                {
                    if(trackMap.size()>0)
                    {
                        cont_search = true;
                        auto it = trackMap.begin();
                        while (cont_search && it != trackMap.end())
                        {
                            TrackDetails td = it->second;
                            if (td.number_pixels != 0)
                            {
                                TranslateFrameByOffsetsManual(td, frame, cont_search, framei, xOffset, output, track_centered_offsets, yOffset, xOffset_correction, yOffset_correction);
                            }
                            else
                            {
                                ++it;
                            }
                        }
                    }
                }
            }
        }
        else //Manual Tracks have priority
        {
            cont_search = true;
            std::map<int, TrackDetails> trackMap = manualFrames[framei].tracks;

            auto it = trackMap.find(manual_track_id);
            if(manual_track_id > 0 && it != trackMap.end()) //Specific track id
            {
                TrackDetails td = it->second;
                if (td.number_pixels != 0)
                {
                    TranslateFrameByOffsetsManual(td, frame, cont_search, framei, xOffset, output, track_centered_offsets, yOffset, xOffset_correction, yOffset_correction);
                }
            }
            else //Search for first track
            {
                auto it = trackMap.begin();
                while (cont_search && it != trackMap.end())
                {
                    TrackDetails td = it->second;
                    if (td.number_pixels != 0){
                        TranslateFrameByOffsetsManual(td, frame, cont_search, framei, xOffset, output, track_centered_offsets, yOffset, xOffset_correction, yOffset_correction);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            if(cont_search && findAnyTrack) // Now search for OSM tracks
            {
                std::map<int, TrackDetails> trackMap = osmFrames[framei].tracks;

                auto it = trackMap.find(OSM_track_id);
                if(OSM_track_id>0 && it != trackMap.end()) //Specific track id
                {
                    TrackDetails td = it->second;
                    if (td.number_pixels != 0)
                    {
                        TranslateFramesByOffsetOsm(yOffset, track_centered_offsets, cont_search, frame, xOffset, output, framei, td);
                    }
                }
                else{
                    auto it = trackMap.begin();
                    while (cont_search && it != trackMap.end())
                    {
                        TrackDetails td = it->second;
                        if (td.number_pixels != 0){
                            TranslateFramesByOffsetOsm(yOffset, track_centered_offsets, cont_search, frame, xOffset, output, framei, td);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                }
            }
        }
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
    }

    return frames_out;
}

std::vector<std::vector<uint16_t>> ImageProcessing::CenterImageFromOffsets(const VideoDetails & original, const std::vector<std::vector<int>>& track_centered_offsets)
{
    std::vector<std::vector<uint16_t>> frames_out;
    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels;
    int nCols = original.x_pixels;
    int yOffset, xOffset;
    arma::mat output(nRows, nCols);
    arma::mat frame(nRows, nCols);
    arma::mat offset_matrix(track_centered_offsets.size(),3,arma::fill::zeros);
    for (int rowi = 0; rowi < track_centered_offsets.size(); rowi++){
        offset_matrix.row(rowi) = arma::conv_to<arma::rowvec>::from(track_centered_offsets[rowi]);
    }
    for (int framei = 0; framei < num_video_frames; framei++)
    {
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }

        frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();
        output = frame;
        arma::uvec kk = arma::find(offset_matrix.col(0) == framei + 1,0,"first");
        if (!kk.is_empty()){
            xOffset = offset_matrix(kk(0),1);
            yOffset = offset_matrix(kk(0),2);
            output = arma::shift(arma::shift(frame,-yOffset,0),-xOffset,1);
        }
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
    }
    return frames_out;
}

std::vector<std::vector<uint16_t>> ImageProcessing::CenterOnBrightest(const VideoDetails & original, std::vector<std::vector<int>> & brightest_centered_offsets)
{
    std::vector<std::vector<uint16_t>> frames_out;
    int num_video_frames = original.frames_16bit.size();
    int nRows = original.y_pixels, nRows2 = nRows/2;
    int nCols = original.x_pixels, nCols2 = nCols/2;
    int yOffset0, xOffset0, i_max;
    arma::uvec peak_index;
    arma::mat output(nRows, nCols);
    arma::mat frame1(nRows, nCols);

    for (int framei = 0; framei < num_video_frames; framei++)
    {
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }

        frame1 = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows).t();
        frame1 = frame1 - arma::mean(frame1.as_col());
        i_max = frame1.index_max();
        peak_index = arma::ind2sub(arma::size(frame1), i_max);
        yOffset0 = nRows2 - peak_index(0);
        xOffset0 = nCols2 - peak_index(1);
        double d = sqrt(pow(yOffset0,2) + pow(xOffset0,2));

        if (d > 1.5)
        {
            frame1 = arma::shift(arma::shift(frame1,yOffset0,0),xOffset0,1);
        }

        brightest_centered_offsets.push_back({framei + 1,-xOffset0,-yOffset0});
        output = frame1 - arma::min(frame1.as_col());
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.t().as_col()));
    }

    return frames_out;
}

 std::vector<std::vector<uint16_t>> ImageProcessing::FrameStacking(int num_of_averaging_frames, const VideoDetails & original)
 {
    int num_video_frames = original.frames_16bit.size();
    int num_pixels = original.frames_16bit[0].size();
    int stop_frame_index;
    double R1, R2;

    std::vector<std::vector<uint16_t>> frames_out;
    arma::mat window_data(num_pixels,num_of_averaging_frames);
    window_data.fill(0.0);
    arma::vec moving_mean(num_pixels, 1);
    arma::vec frame_vector(num_pixels,1);
    arma::vec frame_vector_out(num_pixels,1);

    for (int j = 0; j < num_of_averaging_frames; j++)
    {
        window_data.col(j) = arma::conv_to<arma::vec>::from(original.frames_16bit[j]);
    }

    for (int i = 0; i < num_video_frames; i++)
    {
        if (cancel_operation)
        {
            return std::vector<std::vector<uint16_t>>();
        }
        UpdateProgressBar(i);
        QCoreApplication::processEvents();
        frame_vector = arma::conv_to<arma::vec>::from(original.frames_16bit[i]);
        stop_frame_index = std::min(i + num_of_averaging_frames - 1,num_video_frames - 1);
        moving_mean = arma::mean(window_data,1);
        window_data.insert_cols(window_data.n_cols,arma::conv_to<arma::vec>::from(original.frames_16bit[stop_frame_index]));
        window_data.shed_col(0);
        // R = arma::max(frame_vector);
        R1 = arma::range(frame_vector);
        R2 = arma::range(moving_mean);
        // frame_vector_out = R * moving_mean / moving_mean.max();
        frame_vector_out = R1 * moving_mean / R2;
        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(frame_vector_out));
    }

    return frames_out;
 }


void ImageProcessing::UpdateProgressBar(unsigned int val)
{
    emit signalProgress(val);
}

void ImageProcessing::CancelOperation()
{
    cancel_operation = true;
}

std::vector<std::vector<uint16_t>> ImageProcessing::MedianFilterStandard(VideoDetails & original, int window_size)
{
    // Initialize output
    std::vector<std::vector<uint16_t>> frames_out;

    int num_video_frames = original.frames_16bit.size();
    arma::mat window(window_size,window_size);
    int nRows = original.y_pixels;
    int nCols = original.x_pixels;

    arma::mat output(nRows, nCols);
    arma::mat frame(nCols, nRows);
    arma::mat paddedInput(nCols + window_size - 1, nRows + window_size - 1);

    for (int framei = 0; framei < num_video_frames; framei++)
    {
        UpdateProgressBar(framei);
        QCoreApplication::processEvents();
        frame = arma::reshape(arma::conv_to<arma::vec>::from(original.frames_16bit[framei]),nCols,nRows);
        paddedInput.zeros();
        paddedInput.submat(window_size / 2, window_size / 2, nCols - 1 + window_size / 2, nRows - 1 + window_size / 2) = frame;

        // Create an output matrix
        output.zeros();

        // Iterate over the input matrix
        for (arma::uword i = 0; i < nCols; i++)
        {
            for (arma::uword j = 0; j < nRows; j++)
            {
                // Get the window of pixels around the current pixel
                window = paddedInput.submat(i, j, i + window_size - 1, j + window_size - 1);
                output(j, i) = arma::median(window.as_col());
            }
        }

        frames_out.push_back(arma::conv_to<std::vector<uint16_t>>::from(output.as_row()));
    }

    return frames_out;
}
