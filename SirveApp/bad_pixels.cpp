#include "bad_pixels.h"

std::vector<unsigned int> BadPixels::IdentifyDeadPixels(const std::vector<std::vector<uint16_t>>& input_pixels)
{    
    std::vector<unsigned int> dead_pixels;

    size_t num_frames = input_pixels.size();
    // Loop through each pixel
    for (auto i = 0; i < input_pixels[0].size(); i++)
    {
        double mean = 0.0;
        double squared_diff_sum = 0.0;

        for (int j = 0; j < num_frames; j++) {
            mean += input_pixels[j][i];
        }
        mean /= num_frames;

        for (int j = 0; j < num_frames; j++) {
            double diff = input_pixels[j][i] - mean;
            squared_diff_sum += diff * diff;
        }

        double variance = squared_diff_sum / num_frames;
        double std_deviation = std::sqrt(variance);

        if (std_deviation == 0)
        {
            dead_pixels.push_back(i);
        }
    }

    return dead_pixels;
}

void BadPixels::ReplacePixelsWithNeighbors(std::vector<std::vector<uint16_t>> & original_pixels, std::vector<unsigned int> bad_pixel_indeces, int width_pixels, QProgressDialog & progress)
{
    //For each frame, replaces any bad pixels with the mean of the value of 2 pixels above, below, left, and right
    //Other bad pixels are exempted from the calculation

    //Creating a set (for faster lookup) of bad pixel indeces simplifies/speeds up some of this code
    std::set<int> bad_pixel_indeces_set(bad_pixel_indeces.begin(), bad_pixel_indeces.end());

    for (auto frame = 0; frame < original_pixels.size(); frame++)
    {
        progress.setValue(frame);
        for (auto i = 0; i < bad_pixel_indeces.size(); i++)
        {
            int bad_pixel_index = bad_pixel_indeces[i];
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
                if (bad_pixel_indeces_set.count(neighbor_pixels[j]))
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

std::vector<unsigned int> BadPixels::IdentifyDeadPixelsMedian(double N, std::vector<std::vector<uint16_t>>& input_pixels, bool only_dead, QProgressDialog & progress)
{    
    int num_frames = input_pixels.size();
    num_frames = std::min(num_frames,500);
	int num_pixels = input_pixels[0].size();

    // Create an Armadillo matrix
    arma::mat frame_data(num_pixels, num_frames);

    // Fill the Armadillo matrix from the std::vector
    for (int i = 0; i < num_frames; i++) {
        frame_data.col(i) = arma::conv_to<arma::vec>::from(input_pixels[i]);
    }
    
    arma::vec std_frame = arma::stddev(frame_data,0,1);
    arma::uvec index_dead = arma::find(std_frame == 0);
    if (!only_dead){
        arma::vec med_frame = arma::median(frame_data,1);
        arma::mat med_frame_M = arma::repmat(med_frame,1,frame_data.n_cols);
        progress.setValue(2);  
        double c = 1.4826;
        arma::mat diff_from_med = arma::abs(frame_data - med_frame_M);
        arma::vec MAD = c*arma::median(diff_from_med,1);
        arma::mat MADM = arma::repmat(MAD,1,frame_data.n_cols);
        arma::umat OUTL = diff_from_med > 3*MADM;
        arma::uvec SUMB = arma::sum(OUTL,1);
        progress.setValue(3);  
        arma::vec P = arma::conv_to<arma::vec>::from(SUMB);
        arma::uvec index_outlier = arma::find(arma::abs(P - arma::mean(P)) > N*arma::stddev(P));
    
        index_dead = arma::unique(arma::join_vert(index_outlier,index_dead));
    }
    progress.setValue(4);  
    std::vector<unsigned int> dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_dead);

    return dead_pixels;
}

std::vector<unsigned int> BadPixels::IdentifyDeadPixelsMovingMedian(int half_window_length, double N, std::vector<std::vector<uint16_t>>& input_pixels, QProgressDialog & progress)
{
	// Initialize output
	int num_video_frames = input_pixels.size();
	
	//Initialize video frame storage
	int num_pixels = input_pixels[0].size();

    arma::mat moving_median(num_pixels, num_video_frames);
    arma::mat MAD(num_pixels, num_video_frames);


	// initialize noise frames
	int index_first_frame, index_last_frame;

	// Create an Armadillo matrix
    arma::mat frame_data(num_pixels, num_video_frames);

    // Fill the Armadillo matrix from the std::vector
    for (int i = 0; i < num_video_frames; i++) {
        frame_data.col(i) = arma::conv_to<arma::vec>::from(input_pixels[i]);
    }

    arma::vec std_frame = arma::stddev(frame_data,0,1);
    arma::uvec index_dead = arma::find(std_frame == 0);

    double c = 1.4826;
    arma::umat OUTL(num_pixels, num_video_frames);

    //frame_data.save("frame_data.bin",arma::arma_binary);

    // std::ofstream myfile;
    // std::string path = "test.txt";
	// myfile.open(path);

	for (int i = 0; i < num_video_frames; i++)
	{
		if (progress.wasCanceled())
		{
			return std::vector<unsigned int>();
		}

		progress.setValue(i);

        index_first_frame = std::max(i - (half_window_length),0);
        index_last_frame = std::min(i + (half_window_length),num_video_frames - 1);  
        moving_median.col(i) = arma::median(frame_data.cols(index_first_frame,index_last_frame), 1);
        MAD.col(i) = c*arma::median(arma::abs(frame_data.cols(index_first_frame,index_last_frame) - arma::repmat(moving_median.col(i),1,frame_data.cols(index_first_frame,index_last_frame).n_cols)),1);

        // myfile << i << " " << index_first_frame << " " << index_last_frame << "\n" ;
    }
    // myfile.close();
    OUTL = arma::abs(frame_data - moving_median) > 3*MAD;
    //OUTL.save("OUTL.bin",arma::arma_binary);
    arma::uvec SUMB = arma::sum(OUTL,1);
    //SUMB.save("SUMB.bin",arma::arma_binary);
    arma::vec P = arma::conv_to<arma::vec>::from(SUMB);
    arma::uvec index_outlier = arma::find(arma::abs(P - arma::mean(P)) > N*arma::stddev(P));
    index_outlier = arma::unique(arma::join_vert(index_outlier,index_dead));
    std::vector<unsigned int> dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_outlier);
    return dead_pixels;

}
