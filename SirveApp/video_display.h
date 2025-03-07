#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H

#include <string>
#include <vector>
#include <optional>

#include <opencv2/opencv.hpp>

#include <armadillo>

#include "annotation_stencil.h"
#include "abir_reader.h"
#include "video_container.h"
#include "Data_Structures.h"
#include "enhanced_label.h"
#include "calibration_data.h"
#include "annotation_info.h"
#include "video_display_zoom.h"
#include "tracks.h"

class VideoDisplay : public QWidget
{
    Q_OBJECT

public:
    VideoDisplay(const QVector<QRgb>& starting_color_table, QWidget *parent = nullptr);
    static const QString kBoldLargeStyleSheet;

    QPointer<QVBoxLayout> video_display_layout;
    void ReclaimLabel();

	unsigned int counter = 0;
    int x_correction = 0;
    int y_correction = 0;

    std::vector<AnnotationInfo> annotation_list;

    int image_x = 0;
    int image_y = 0;

    QPointer<EnhancedLabel> lbl_image_canvas;
    QPointer<AnnotationStencil> annotation_stencil;
    VideoContainer container;

    QString banner_text;
    QColor bad_pixel_color;

    void HighlightBadPixels(bool status);
    void HighlightBadPixelsColors(const QString& input_color);

    void UpdateFrameData(std::vector<PlottingFrameData> input_data);

    void InitializeTrackData(std::vector<TrackFrame> osm_frame_input, std::vector<TrackFrame> manual_frame_input);
    void InitializeFrameData(unsigned int frame_number, std::vector<PlottingFrameData> input_data, std::vector<ABIR_Frame>& input_frame_header);
    void UpdateManualTrackData(std::vector<TrackFrame> track_frame_input);

    void AddManualTrackIdToShowLater(int id);
    void EstablishStencil();
    void HideManualTrackId(int id);
    void ShowManualTrackId(int id);
    void RecolorManualTrack(int id, QColor color);
    void DeleteManualTrack(int id);

    void SetCalibrationModel(CalibrationData input);
    void AddNewFrame(const QImage &img, int format);
    bool StartRecording(const QString& file_name, double fps);
    void StopRecording();

    void ToggleActionZoom(bool status);
    void ToggleActionCalculateRadiance(bool status);
    void ToggleOsmTracks(bool input);

    void EnterTrackCreationMode(QPoint appPos, std::vector<std::optional<TrackDetails>> starting_track_details, int threshold, int bbox_buffer_pixels, double clamp_low_coeff, double clamp_high_coeff, std::string trackFeature, std::string prefilter);
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
    void DisplayManualBox(QPoint pt);
    void SetSelectCentroidBtn(bool status);
    void GetThreshold(const QVariant& data);
    void OnTrackFeatureRadioButtonClicked(int id);
    void OnFilterRadioButtonClicked(int id);
    void ClearPinpoints();
    void SetCurrentIdx(int current_idx_new);

    void HandleAnnotationChanges();
    void HandleColorMapUpdate(QVector<QRgb> color_table);
    void HandleFrameTimeToggle(bool checked);
    void HandleImageAreaSelection(QRect area);
    void HandlePinpointControlActivation(bool enabled);
    void HandlePixelSelection(QPoint origin);
    void HandleSensorBoresightDataCheck(bool checked);
    void HandleTrackerColorUpdate(QColor input_color);

    // stencil stuff
    void HideStencil();
    void ShowStencil();
    void InitializeStencilData(AnnotationInfo data);

    void UndoZoom();

    void UpdateFrameVector(std::vector<double> original, std::vector<uint8_t> converted, arma::mat offsets_matrix);
    void UpdateBannerText(const QString& input_banner_text);
    void UpdateBannerColor(const QString& input_color);

private:
    static constexpr int kFrameAdvanceLimit = 500;
    QVector<QRgb> color_table;

    bool record_frame = false;
    QPoint hover_pt;
    arma::mat offsets_matrix;
    int bbox_buffer_pixels = 0;
    std::vector<double> original_frame_vector;
    std::vector<uint8_t> display_ready_converted_values;
    int video_frame_number = 0;
    int current_idx = -1;
    int extent_window_x = 0;
    int extent_window_y = 0;
    int threshold = 0;
    double clamp_low_coeff = 0;
    double clamp_high_coeff = 0;
    int SirveApp_x = 0, SirveApp_y = 0, Display_res_x = 0, Display_res_y = 0;
    QSize screenResolution;
    std::string trackFeature;
    std::string prefilter;
    cv::VideoWriter video;
    int number_pixels = 0;
    QImage frame;
    QColor OSM_track_color;
    bool plot_tracks = true;

    // Pinpoint controls and data
    std::vector<unsigned int> pinpoint_indices;

    QPointer<QLabel> lbl_pinpoint;
    QPointer<QGroupBox> grp_pinpoint;
    QPointer<QPushButton> btn_pinpoint;
    QPointer<QPushButton> btn_pinpoint_bad_pixel;
    QPointer<QPushButton> btn_pinpoint_good_pixel;
    QPointer<QPushButton> btn_clear_pinpoints;

    // Track-related controls and data
    std::vector<std::optional<TrackDetails>> track_details;
    int track_details_min_frame = 0, track_details_max_frame = 0;

    QPointer<QLabel> lbl_create_track;
    QPointer<QPushButton> btn_select_track_centroid, btn_clear_track_centroid;
    QPointer<QCheckBox>  chk_auto_advance_frame, chk_snap_to_feature, chk_show_crosshair;
    QPointer<QLineEdit> txt_frame_advance_amt, txt_ROI_dim;
    QPointer<QGroupBox> grp_create_track;
    QPointer<QVBoxLayout> vlayout_create_track;

    // Zooming-related controls and data
    VideoDisplayZoomManager zoom_manager;
    bool is_zoom_active = false, is_calculate_active = false, should_show_bad_pixels = false;
    bool in_track_creation_mode = false, cursor_in_image = false;
    QPointer<QLabel> lbl_video_time_midnight, lbl_zulu_time;
    QPointer<QLabel> lbl_frame_number;
    QRect calculation_region;

    // Frame data
    std::vector<std::vector<uint16_t>> frame_data;
    unsigned int starting_frame_number = 0;

    std::vector<unsigned int> bad_pixels;

    QColor banner_color;
    bool display_boresight = false, display_time = false;

    CalibrationData model;
    std::vector<PlottingFrameData> display_data;
    std::vector<TrackFrame> osm_track_frames;
    std::vector<TrackFrame> manual_track_frames;
    std::set<int> manual_track_ids_to_show;
    std::map<int, QColor> manual_track_colors;
    std::vector<ABIR_Frame> frame_headers;

    void SetupUi();
    void SetupLabels();
    void SetupCreateTrackControls();
    void SetupPinpointDisplay();
    void SetupCrosshairCursor(const QString& icon_Name);

    void ExitSelectTrackCentroidMode();

    void Calibrate(QRect area);

    void PinpointPixel(unsigned int x, unsigned int y);
    void SelectTrackCentroid(unsigned int x, unsigned int y);

    void AddPinpointsToBadPixelMap();
    void RemovePinpointsFromBadPixelMap();
    void HandleBtnPinpoint(bool checked);
    void HandleBtnSelectTrackCentroid(bool checked);
    void HandleClearTrackCentroidClick();
    void HandleFrameAdvanceAmtEntry(const QString &text) const;
    void ResetCreateTrackMinAndMaxFrames();
    void UpdateCreateTrackLabel();
    void UpdateDisplayFrame();
    static QString GetZuluTimeString(double seconds_midnight);
    QRectF GetRectangleAroundPixel(int x_center, int y_center, int box_size, double box_width, double box_height);
};

#endif // VIDEO_DISPLAY_H
