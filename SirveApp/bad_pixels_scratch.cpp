#include "bad_pixels.h"

std::vector<unsigned int> BadPixels::identify_dead_pixels(const std::vector<std::vector<uint16_t>>& input_pixels)
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

void BadPixels::replace_pixels_with_neighbors(std::vector<std::vector<uint16_t>> & original_pixels, std::vector<unsigned int> bad_pixel_indeces, int width_pixels)
{
    //For each frame, replaces any bad pixels with the mean of the value of 2 pixels above, below, left, and right
    //Other bad pixels are exempted from the calculation

    //Creating a set (for faster lookup) of bad pixel indeces simplifies/speeds up some of this code
    std::set<int> bad_pixel_indeces_set(bad_pixel_indeces.begin(), bad_pixel_indeces.end());

    for (auto frame = 0; frame < original_pixels.size(); frame++)
    {
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

std::vector<unsigned int> BadPixels::identify_dead_pixels_new(std::vector<std::vector<uint16_t>>& input_pixels)
{    
    //std::ofstream myfile("test.bin", std::ios::binary);
    //std::ofstream myfile;
    //std::string path = "test.txt";
	//myfile.open(path);
    int num_frames = input_pixels.size();
    num_frames = std::max(num_frames,100);
	//Initialize video frame storage
	int num_pixels = input_pixels[0].size();
	//arma::mat frame_data(num_pixels, 1);

    //arma::cube ircube(480,640,num_frames);
    //arma::mat frame_slice(640,480);
    //arma::mat frame_data(arma::conv_to<arma::mat>::from(input_pixels));
 	// for (int i = 0; i < num_frames; i++)
	// {
	// 	std::vector<double> frame_values(input_pixels[i].begin(), input_pixels[i].end());
	// 	arma::vec frame_vector(frame_values);
    //     //frame_slice = arma::reshape(frame_vector,640,480);
    //     //ircube.slice(i) = frame_slice.t();
	// 	frame_data.insert_cols(0, frame_vector);
	// }
 
    // frame_data.shed_col(frame_data.n_cols-1);
    // Create a std::vector
 
    // Create an Armadillo matrix
    arma::mat frame_data(num_pixels, num_frames);

    // Fill the Armadillo matrix from the std::vector
    for (int i = 0; i < num_frames; i++) {
        frame_data.col(i) = arma::conv_to<arma::vec>::from(input_pixels[i]);
    }
    

    arma::vec std_frame = arma::stddev(frame_data,0,1);
    arma::vec med_frame = arma::median(frame_data,1);
    arma::mat med_frame_M = arma::repmat(med_frame,1,frame_data.n_cols);
    //arma::mat m1 = arma::conv_to<arma::mat>::from(med_frame);
    double c = 1.4826;
    arma::mat diff_from_med = arma::abs(frame_data - med_frame_M);
    //arma::vec avg_diff_from_med = arma::mean(diff_from_med,1);
    arma::vec MAD = c*arma::median(diff_from_med,1);
    arma::mat MADM = arma::repmat(MAD,1,frame_data.n_cols);
    arma::umat OUTL = diff_from_med > 3*MADM;
    arma::uvec SUMB = arma::sum(OUTL,1);
    arma::vec P = arma::conv_to<arma::vec>::from(SUMB);
    arma::uvec index_outlier = arma::find(abs(P - arma::mean(P)) > 5.0*arma::stddev(P));
    arma::uvec index_dead = arma::find(std_frame == 0);
    arma::uvec index_bad = arma::unique(arma::join_vert(index_outlier,index_dead));
 
    // diff_from_med.save("diff_from_med.bin",arma::arma_binary);
    // std_frame.save("std_frame.bin",arma::arma_binary);
    // med_frame_M.save("med_frame_M.bin",arma::arma_binary);
    // SUMB.save("SUMB.bin",arma::arma_binary);
    // frame_slice.save("frame_slice.bin",arma::arma_binary);
    // ircube.save("ircube.bin",arma::arma_binary);
    // MAD.save("MAD.bin",arma::arma_binary);
    // index_dead.save("index_dead.bin",arma::arma_binary);
    //diff_from_med.save(myfile);
    //myfile.close();
	//std::vector<unsigned int> dead_pixels(index_dead.begin(), index_dead.end());
    std::vector<unsigned int> dead_pixels = arma::conv_to<std::vector<unsigned int>>::from(index_bad);

    // std::ofstream myfile;
    // std::string path = "test.txt";
	// myfile.open(path);
    // myfile << num_frames;
    // myfile.close();
// processing_state original = video_display->container.copy_current_state();
			// std::vector<std::vector<uint16_t>> test_frames;
			// test_frames.reserve(number_of_frames);
			// for (int i = 0; i < number_of_frames; i++){
			// 	int j = i + start_frame;
			// 	if (j<original.details.frames_16bit.size()){
			// 		test_frames.push_back(original.details.frames_16bit[j]);
			// 	}
			// }
    return dead_pixels;
}