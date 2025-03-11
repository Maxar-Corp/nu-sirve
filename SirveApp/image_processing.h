#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <armadillo>
#include <QtWidgets>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>

#include "abir_reader.h"
#include "tracks.h"
#include "video_details.h"
#include "constants.h"

class  ImageProcessing : public QObject
{
	Q_OBJECT

public:

    ImageProcessing();
    ~ImageProcessing();

    double min_deinterlace_dist;
    double max_deinterlace_dist;
    int deinterlace_kernel_size;
    int frameval;
    bool cancel_operation;
    int nRows = SirveAppConstants::VideoDisplayHeight;
    int nRows2 = nRows/2;
    int nCols = SirveAppConstants::VideoDisplayWidth;

    void ReplacePixelsWithNeighbors(std::vector<std::vector<uint16_t>> & original_pixels, const std::vector<unsigned int>& bad_pixel_indeces, int width_pixels);
    void UpdateProgressBar(unsigned int value);

    arma::uvec FindDeadBadscalePixels(const std::vector<std::vector<uint16_t>>& input_pixels);
    arma::uvec IdentifyBadPixelsMedian(double N, const std::vector<std::vector<uint16_t>>& input_pixels);
    arma::uvec IdentifyBadPixelsMovingMedian(int half_window_length, double N, const std::vector<std::vector<uint16_t>> & input_pixels);

    std::vector<std::vector<uint16_t>> FixedNoiseSuppression(const QString& image_path, const QString& path_video_file, int frame0, int start_frame, int end_frame, double version, const VideoDetails & original);
    std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppressionByFrame(int start_frame, int num_of_averaging_frames_input, const VideoDetails& original);
    std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppressionMatrix(int start_frame, int num_of_averaging_frames, const VideoDetails& original);
    std::vector<std::vector<uint16_t>> AccumulatorNoiseSuppression(double weight, int offset, int NThresh, const VideoDetails& original, bool hide_shadow_choice);
    std::vector<std::vector<uint16_t>> RPCPNoiseSuppression(const VideoDetails& original);
	std::vector<std::vector<uint16_t>> DeinterlaceOpenCVPhaseCorrelation(const VideoDetails & original);
    std::vector<uint16_t> DeinterlacePhaseCorrelationCurrent(int current_frame, const std::vector<uint16_t>& current_frame_16bit) const;

    std::vector<std::vector<uint16_t>> CenterOnTracks(const QString& trackTypePriority, const VideoDetails& original, int OSM_track_id, int manual_track_id, const std::vector<TrackFrame>& osmFrames,\
                                                      const std::vector<TrackFrame>& manualFrames, boolean findAnyTrack, std::vector<std::vector<int>> & track_centered_offsets);

    std::vector<std::vector<uint16_t>> CenterOnBrightest(const VideoDetails & original, std::vector<std::vector<int>> & brightest_centered_offsets);
    std::vector<std::vector<uint16_t>> FrameStacking(int number_of_frames, const VideoDetails & original);
    std::vector<std::vector<uint16_t>> MedianFilterStandard(VideoDetails & original, int window_size);
    std::vector<std::vector<uint16_t>> CenterImageFromOffsets(const VideoDetails & original, const std::vector<std::vector<int>>& track_centered_offsets);

signals:
     void signalProgress(unsigned int frameval);

public slots:
    void CancelOperation();

private:

    ABIRData abir_data;
    arma::mat disk_avg_kernel;

    static arma::mat apply_shrinkage_operator(arma::mat s, double tau);
    void remove_shadow(int nRows, int nCols, arma::vec & frame_vector, int NThresh);
    static arma::mat perform_thresholding(arma::mat X, double tau);
    void TranslateFrameByOffsets(TrackDetails &td, arma::mat &frame, bool &cont_search, int &framei, int &xOffset, arma::mat &output, std::vector<std::vector<int>>& track_centered_offsets, int &yOffset);
};

#endif
