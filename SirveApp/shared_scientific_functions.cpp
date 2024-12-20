#include "shared_scientific_functions.h"

double IrradianceCountsCalc::ComputeIrradiance(int indx, int height2, int width2, int x, int y, VideoDetails & base_processing_state_details)
{
    int number_median_frames = 30;
    double irradiance_val;
     
    int start_indx;
    start_indx = std::max(indx - number_median_frames,0);

    int nRows = base_processing_state_details.y_pixels;
    int nCols = base_processing_state_details.x_pixels;

    int row1 = std::max(y - height2,0);
    int row2 = std::min(y + height2,nRows);
    int col1 = std::max(x - width2,0);
    int col2 = std::min(x + width2,nCols);

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