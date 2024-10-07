#pragma once

#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H

#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include <vector>
#include <optional>
#include <windows.h>

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
#include <QFormLayout>

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
    VideoDisplay(QVector<QRgb> starting_color_table, QWidget *parent = nullptr);
    ~VideoDisplay();       

	QString bold_large_styleSheet = "color: black; font-weight: bold; font-size: 12px";

    QVBoxLayout *video_display_layout;
    void ReclaimLabel();

	unsigned int counter;
    QPoint hover_pt;
	std::vector<std::vector<int>> offsets;
	int xCorrection = 0;
    int yCorrection = 0;
    int current_idx;
    int counter_record, video_frame_number;
    bool record_frame;
    int threshold = 3;
    cv::VideoWriter video;

    std::vector<AnnotationInfo> annotation_list;

    int image_x, image_y, number_pixels;

    QImage frame;
    EnhancedLabel  *lbl_image_canvas;
    AnnotationStencil *annotation_stencil;
    VideoContainer container;

    QString banner_text, boresight_text;
    QColor OSM_track_color, bad_pixel_color;
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
    bool StartRecording(QString file_name, double fps);
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
    void InitializeToggles();
    void DrawAnnotations();

signals:
    void clearMouseButtons();
    void addNewBadPixels(std::vector<unsigned int> new_pixels);
    void removeBadPixels(std::vector<unsigned int> pixels);
    void advanceFrame(int frame_amt);
    void finishTrackCreation();
    void disableTrack(int id);
    void enableTrack(int id);

public slots:
    void GetThreshold(int threshold_in);
    void ClearPinpoints();
    void GetCurrentIdx(int current_idx_new);

    void HandleAnnotationChanges();
    void HandleColorMapUpdate(QVector<QRgb> color_table);
    void HandleFrameTimeToggle(bool checked);
    void HandleImageAreaSelection(QRect area);
    void HandlePinpointControlActivation(bool enabled);
    void HandlePixelSelection(QPoint origin);
    void HandleSensorBoresightDataCheck(bool checked);
    // void HandleTrackerColorUpdate(QString input_color);
    void HandleTrackerColorUpdate(QColor input_color);

    // stencil stuff
    void HideStencil();
    void ShowStencil();
    void InitializeStencilData(AnnotationInfo data);

    void UndoZoom();

    void UpdateFrameVector(std::vector<double> original, std::vector<uint8_t> converted, std::vector<std::vector<int>> offsets);
    void UpdateBannerText(QString input_banner_text);
    void UpdateBannerColor(QString input_color);

private:
    VideoDisplayZoomManager *zoom_manager;
    std::vector<double> original_frame_vector;
    std::vector<uint8_t> display_ready_converted_values;

    QVector<QRgb> colorTable;

    QLabel *lbl_pinpoint;
    QGroupBox *grp_pinpoint;
    std::vector<unsigned int> pinpoint_indices;
    QPushButton *btn_pinpoint, *btn_pinpoint_bad_pixel, *btn_pinpoint_good_pixel, *btn_clear_pinpoints;

    QLabel *lbl_create_track;
    QPushButton *btn_select_track_centroid, *btn_clear_track_centroid;
    QCheckBox  *chk_auto_advance_frame;
    QLineEdit *txt_frame_advance_amt,  *txt_ROI_dim;
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
    bool display_boresight, display_time;

    CalibrationData model;
    std::vector<PlottingFrameData> display_data;
    std::vector<TrackFrame> osm_track_frames;
    std::vector<TrackFrame> manual_track_frames;
    std::set<int> manual_track_ids_to_show;
    std::map<int, QColor> manual_track_colors;
    std::vector<ABIR_Frame> frame_headers;

    void DisplayManualBox(QPoint pt);
    void SetupLabels();
    void SetupCreateTrackControls();
    void SetupPinpointDisplay();

    void SetupCrosshairsCursor(QString icon_Name);
    void ExitSelectTrackCentroidMode();

    void Calibrate(QRect area);

    void PinpointPixel(unsigned int x, unsigned int y);
    void SelectTrackCentroid(unsigned int x, unsigned int y);

    void AddPinpointsToBadPixelMap();
    void RemovePinpointsFromBadPixelMap();
    void HandleBtnPinpoint(bool checked);
    void HandleBtnSelectTrackCentroid(bool checked);
    void HandleClearTrackCentroidClick();
    void HandleFrameAdvanceAmtEntry(const QString &text);
    void ResetCreateTrackMinAndMaxFrames();
    void UpdateCreateTrackLabel();
    void UpdateDisplayFrame();

    QString GetZuluTimeString(double seconds_midnight);
    QRectF GetRectangleAroundPixel(int x_center, int y_center, int box_size, double box_width, double box_height);
};

#endif // VIDEO_DISPLAY_H
