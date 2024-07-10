#pragma once

#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H

#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include <vector>
#include <optional>

#include <QString>
#include <QTimer>
#include <QLabel>
#include <QColor>
#include <vector>
#include <math.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qfiledialog.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/types_c.h"

#include <armadillo>

#include "annotation_stencil.h"
#include "color_scheme.h"
#include "abir_reader.h"
#include "video_container.h"
#include "Data_Structures.h"
#include "enhanced_label.h"
#include "color_map.h"
#include "calibration_data.h"
#include "annotation_info.h"
#include "video_display_zoom.h"
#include "tracks.h"

class VideoDisplay : public QWidget
{
    Q_OBJECT
public:
   
	QString dark_green_button_styleSheet = "color: white; background-color: #1a3533; font-weight: bold;";
	QString olive_green_button_styleSheet = "color: white; background-color: #555121; font-weight: bold;";
	QString dark_blue_button_styleSheet = "color: white; background-color: #0b2139; font-weight: bold;";
	QString dark_orange_button_styleSheet = "color: white; background-color: #743203; font-weight: bold;";
	QString track_button_styleSheet = "color: white; background-color: #002147; font-weight: bold;";
	QString dark_red_stop_styleSheet = "color: white; background-color: #331a1a; font-weight: bold;";
	QString orange_styleSheet = "color: black; background-color: #fbb31a; font-weight: bold;";
	QString bright_green_styleSheet = "color: black; background-color: #73ee53; font-weight: bold;";
	QString bold_large_styleSheet = "color: black; font-weight: bold; font-size: 12px";

    VideoDisplay(QVector<QRgb> starting_color_table);
    ~VideoDisplay();
    QVBoxLayout *video_display_layout;
    void ReclaimLabel();

	unsigned int counter;
	std::vector<std::vector<int>> offsets;
	int xCorrection = 0;
    int yCorrection = 0;

    int counter_record, video_frame_number;
    bool record_frame;
    cv::VideoWriter video;

    std::vector<AnnotationInfo> annotation_list;

    int image_x, image_y, number_pixels;

    QImage frame;
    EnhancedLabel  *lbl_image_canvas;
    AnnotationStencil *annotation_stencil;
    VideoContainer container;

    QString banner_text, boresight_text;
    QColor tracker_color, bad_pixel_color;
    bool plot_tracks;

    void HighlightBadPixels(bool status);
    void HighlightBadPixelsColors(QString input_color);

    void UpdateFrameData(std::vector<PlottingFrameData> input_data);

    void InitializeTrackData(std::vector<TrackFrame> osm_frame_input, std::vector<TrackFrame> manual_frame_input);
    void InitializeFrameData(unsigned int frame_number, std::vector<PlottingFrameData> input_data, std::vector<ABIR_Frame>& input_frame_header);
    void UpdateManualTrackData(std::vector<TrackFrame> track_frame_input);

    void AddManualTrackIdToShowLater(int id);
    void HideManualTrackId(int id);
    void ShowManualTrackId(int id);
    void RecolorManualTrack(int id, QColor color);
    void DeleteManualTrack(int id);

    void SetCalibrationModel(CalibrationData input);
    void AddNewFrame(QImage &img, int format);
    bool StartRecording(double fps);
    void StopRecording();

    void ToggleActionZoom(bool status);
    void ToggleActionCalculateRadiance(bool status);
    void ToggleOsmTracks(bool input);

    void EnterTrackCreationMode(std::vector<std::optional<TrackDetails>> starting_track_details);
    const std::vector<std::optional<TrackDetails>> & GetCreatedTrackDetails();
    void ExitTrackCreationMode();

    void SaveFrame();
    void ViewFrame(unsigned int frame_number);
    void RemoveFrame();

    void ReceiveVideoData(int x, int y);
    void InitializeToggles();;

signals:
    void clearMouseButtons();
    void addNewBadPixels(std::vector<unsigned int> new_pixels);
    void removeBadPixels(std::vector<unsigned int> pixels);
    void advanceFrame();
    void finishTrackCreation();

public slots:
    void UpdateFrameVector(std::vector<double> original, std::vector<uint8_t> converted, std::vector<std::vector<int>> offsets);
	void UpdateBannerText(QString input_banner_text);
	void UpdateBannerColor(QString input_color);
    void HandleTrackerColorUpdate(QString input_color);
    void HandleColorMapUpdate(QVector<QRgb> color_table);

    void HandleSensorBoresightDataCheck();
    void HandleFrameTimeToggle(bool checked);

    void HandlePixelSelection(QPoint origin);
    void ClearPinpoints();

    void HandleAnnotationChanges();

    void HandleImageAreaSelection(QRect area);
    void ShowStencil();
    void UpdateStencilText(QString text);
    void UndoZoom();


private:
    VideoDisplayZoomManager *zoom_manager;
    std::vector<double> original_frame_vector;
    std::vector<uint8_t> display_ready_converted_values;

    QVector<QRgb> colorTable;

    QLabel *lbl_pinpoint;
    QPushButton *btn_pinpoint, *btn_pinpoint_bad_pixel, *btn_pinpoint_good_pixel, *btn_clear_pinpoints;
    QGroupBox *grp_pinpoint;
    std::vector<unsigned int> pinpoint_indices;

    QLabel *lbl_create_track;
    QPushButton *btn_select_track_centroid, *btn_clear_track_centroid;
    QCheckBox  *chk_auto_advance_frame;
    QGroupBox *grp_create_track;
    QVBoxLayout* vlayout_create_track;
    std::vector<std::optional<TrackDetails>> track_details;
    int track_details_min_frame, track_details_max_frame;

    bool is_zoom_active, is_calculate_active, should_show_bad_pixels;
    bool in_track_creation_mode;
    QLabel *lbl_frame_number, *lbl_video_time_midnight, *lbl_zulu_time;

    QRect calculation_region;

    std::vector<std::vector<uint16_t>> frame_data;
    unsigned int starting_frame_number;

    std::vector<unsigned int> bad_pixels;

    QColor banner_color;
    bool display_boresight_txt, display_time;

    CalibrationData model;
    std::vector<PlottingFrameData> display_data;
    std::vector<TrackFrame> osm_track_frames;
    std::vector<TrackFrame> manual_track_frames;
    std::set<int> manual_track_ids_to_show;
    std::map<int, QColor> manual_track_colors;
    std::vector<ABIR_Frame> frame_headers;

    void SetupLabels();
    void SetupCreateTrackControls();
    void SetupPinpointDisplay();

    void SetupCrosshairsCursor();
    void ExitSelectTrackCentroidMode();

    void Calibrate(QRect area);

    void PinpointPixel(unsigned int x, unsigned int y);
    void SelectTrackCentroid(unsigned int x, unsigned int y);

    void AddPinpointsToBadPixelMap();
    void RemovePinpointsFromBadPixelMap();
    void HandleBtnPinpoint(bool checked);
    void HandleBtnSelectTrackCentroid(bool checked);
    void HandleClearTrackCentroidClick();
    void ResetCreateTrackMinAndMaxFrames();
    void UpdateCreateTrackLabel();
    void UpdateDisplayFrame();

    QString GetZuluTimeString(double seconds_midnight);
    QRectF GetRectangleAroundPixel(int x_center, int y_center, int box_size, double box_width, double box_height);
};

#endif // VIDEO_DISPLAY_H
