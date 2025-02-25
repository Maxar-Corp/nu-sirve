#pragma once

#include "qcolor.h"
#ifndef TRACKS_H
#define TRACKS_H

#include <QFile>
#include <QString>

#include <map>
#include <set>
#include <vector>
#include <optional>

#include "data_structures.h"
#include "support/az_el_calculation.h"


struct TrackDetails {
    double frame_time;
    double julian_date;
    double second_past_midnight;
    double timing_offset;
    int centroid_x_boresight;
    int centroid_y_boresight;
    int centroid_x;
    int centroid_y;
    double az;
    double el;
    int peak_counts;
    int sum_counts;
    int mean_counts;
    int number_pixels;
    double sum_relative_counts;
    double peak_irradiance;
    double mean_irradiance;
    double sum_irradiance;
    double sum_relative_irradiance;
    int bbox_x;
    int bbox_y;
    int bbox_width;
    int bbox_height;
};


struct PlottingTrackDetails {
    int track_id;
    TrackDetails centroid;
    double sum_relative_counts;
    double azimuth;
    double elevation;
};


struct ManualPlottingTrackDetails {
    TrackDetails centroid;
    double azimuth;
    double elevation;
    double sum_relative_counts;
};


struct ManualPlottingTrackFrame {
    std::map<int, ManualPlottingTrackDetails> tracks;
};


struct PlottingTrackFrame {
    std::vector<PlottingTrackDetails> details;
};


struct TrackFrame {
    std::map<int, TrackDetails> tracks;
};


struct TrackFileReadResult {
    std::vector<TrackFrame> frames;
    std::set<int> track_ids;
    QString error_string;
};


struct TrackEngineeringData {
    double i_fov_x, i_fov_y;
    double boresight_lat, boresight_long;
    std::vector<double> dcm;
};


class TrackInformation {

public:
        void AddCreatedManualTrack(std::vector<PlottingFrameData> frame_data,int track_id, const std::vector<std::optional<TrackDetails>> & new_track_details, QString new_track_file_name);
        void AddManualTracks(std::vector<TrackFrame> new_frames);
        void RemoveManualTrack(int track_id);
        void RemoveManualTrackImage(int track_id);
        void RemoveManualTrackPlotting(int track_id);
        int get_frame_count();
        int get_track_count();
        void set_manual_frame(int index, int track_id, TrackDetails * centroid);

private:
        ManualPlottingTrackDetails GetManualPlottingTrackDetails(int frame_number, int centroid_x, int centroid_y, double sum_relative_counts);