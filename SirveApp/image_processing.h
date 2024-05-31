#pragma once
#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H
#include "video_details.h"

#include <cmath>
#include <vector>
#include <set>
#include <armadillo>
#include <QProgressDialog>

namespace MedianFilter
{
    std::vector<std::vector<uint16_t>> median_filter_standard(video_details & original, int window_size, QProgressDialog & progress);
};

namespace Deinterlacing
{
    std::vector<std::vector<uint16_t>> cross_correlation(video_details & original, QProgressDialog & progress);
    static arma::cx_mat xcorr2(arma::mat inFrame1, arma::mat inFrame2);
};

#endif