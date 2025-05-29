#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <armadillo>
#include <QtWidgets>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>

#include "abir_reader.h"
#include "tracks.h"
#include "video_details.h"

enum CenterOnTracks
{
    OSM,
    MANUAL,
    OSM_THEN_MANUAL,
    MANUAL_THEN_OSM
};

class  ImageProcessing : public QObject
{
	Q_OBJECT

public:

    ImageProcessing(ABPFileType file_type);

    double min_deinterlace_dist = 1.5;
    double max_deinterlace_dist = 40.0;
    int deinterlace_kernel_size = 3;
    int frame_val = 0;
    bool cancel_operation = false;

    void ReplacePixelsWithNeighbors(std::vector<std::vector<uint16_t>> & original_pixels, const std::vector<unsigned int>& bad_pixel_indeces, int width_pixels);
    void UpdateProgressBar(unsigned int value);

    arma::uvec FindDeadBadscalePixels(const std::vector<std::vector<uint16_t>>& input_pixels);
    arma::uvec IdentifyBadPixelsMedian(double N, const std::vector<std::vector<uint16_t>>& input_pixels);
    arma::uvec IdentifyBadPixelsMovingMedian(int half_window_length, double N, const std::vector<std::vector<uint16_t>> & input_pixels);

    std::vector<std::vector<uint16_t>> FixedNoiseSuppression(const QString& image_path, const QString& path_video_file, int frame0, int start_frame, int end_frame, ABPFileType file_type, const VideoDetails & original);
    std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppressionByFrame(int start_frame, int num_of_averaging_frames_input, const VideoDetails& original);
    std::vector<std::vector<uint16_t>> AdaptiveNoiseSuppressionMatrix(int start_frame, int num_of_averaging_frames, const VideoDetails& original);
    std::vector<std::vector<uint16_t>> AccumulatorNoiseSuppression(double weight, int offset, int NThresh, const VideoDetails& original, bool hide_shadow_choice);
    std::vector<std::vector<uint16_t>> RPCPNoiseSuppression(const VideoDetails& original);
	std::vector<std::vector<uint16_t>> DeinterlaceOpenCVPhaseCorrelation(const VideoDetails & original);
    std::vector<uint16_t> DeinterlacePhaseCorrelationCurrent(int current_frame, const std::vector<uint16_t>& current_frame_16bit) const;

    std::vector<std::vector<uint16_t>> CenterOnTracks(const QString& trackTypePriority, const VideoDetails& original, int osm_track_id, int manual_track_id, const std::vector<TrackFrame>& osm_frames,\
                                                      const std::vector<TrackFrame>& manual_frames, bool any_track, std::vector<std::vector<int>> & out_offsets);

    std::vector<std::vector<uint16_t>> CenterOnBrightest(const VideoDetails & original, std::vector<std::vector<int>> & brightest_centered_offsets);
    std::vector<std::vector<uint16_t>> FrameStacking(int number_of_frames, const VideoDetails & original);
    std::vector<std::vector<uint16_t>> MedianFilterStandard(VideoDetails & original, int window_size);
    std::vector<std::vector<uint16_t>> CenterImageFromOffsets(const VideoDetails & original, const std::vector<std::vector<int>>& track_centered_offsets);

signals:
     void signalProgress(unsigned int frameval);

public slots:
    void CancelOperation();

private:

    int nRows = 480;
    int nRows2 = 240;
    int nCols = 640;
    int nCols2 = 320;
    std::vector<std::vector<uint16_t>> video_frames_16bit;
    arma::mat disk_avg_kernel;

    static arma::mat apply_shrinkage_operator(const arma::mat& s, double tau);
    void remove_shadow(int nRows, int nCols, arma::vec & frame_vector, int NThresh);
    static arma::mat perform_thresholding(const arma::mat& X, double tau);
};

#endif
