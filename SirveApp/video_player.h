#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <QPointer>
#include <armadillo>

#include "ABIR_Reader.h"
#include "annotation_info.h"
#include "calibration_data.h"
#include "tracks.h"

struct PlottingFrameData;
class PlaybackController;
class SirveApp;
class StateManager;
class VideoDisplay;

class QVBoxLayout;
class QLineEdit;
class QLabel;
class QSlider;
class QPushButton;

class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    VideoPlayer(QWidget *parent, StateManager* state_manager, QVector<QRgb> color_table = {});

    void AddAnnotation(AnnotationInfo anno);
    void AddManualTrackIdToShowLater(int id);
    void ClearAnnotations();
    void DeleteManualTrack(int id);
    void EnterTrackCreationMode(QPoint appPos, std::vector<std::optional<TrackDetails>> starting_track_details,
        int threshold, int bbox_buffer_pixels, double clamp_low_coeff, double clamp_high_coeff,
        std::string trackFeature, std::string prefilter);
    void ExitTrackCreationMode();
    const std::vector<AnnotationInfo>& GetAnnotations() const noexcept;
    std::vector<AnnotationInfo>& GetAnnotations() noexcept;
    const QString& GetBannerText() const;
    uint32_t GetCounter() const;
    const std::vector<std::optional<TrackDetails>>& GetCreatedTrackDetails();
    uint32_t GetCurrentFrameNumber() const;
    const std::vector<ABIRFrameHeader>& GetFrameHeaders() const;
    int GetImageWidth() const;
    int GetImageHeight() const;
    QVBoxLayout* GetLayout() const;
    int GetXCorrection() const;
    int GetYCorrection() const;
    void HideManualTrackId(int id);
    void HighlightBadPixelsColors(const QString& color);
    void InitializeFrameData(unsigned int min_frame, std::vector<PlottingFrameData> input_data,
        std::vector<ABIRFrameHeader> input_frame_header);
    void InitializeStencilData(AnnotationInfo data);
    void InitializeToggles();
    void InitializeTrackData(std::vector<TrackFrame> osm_frame_input, std::vector<TrackFrame> manual_frame_input);
    bool IsPoppedOut() const;
    bool IsRunning() const;
    void ReceiveVideoData(int x, int y);
    void RecolorManualTrack(int id, QColor color);
    void RemoveFrame();
    void SetCalibrationModel(CalibrationData input);
    void SetColorMap(QVector<QRgb> color_table);
    void SetTrackerColor(QColor color);
    void ShowManualTrackId(int id);
    void ToggleOsmTracks(bool input);
    void TogglePopout();
    void UpdateBannerColor(QString color);
    void UpdateCursorColor(QString color);
    void UpdateBannerText(const QString& text);
    void UpdateManualTrackData(std::vector<TrackFrame> track_frame_input);

signals:
    void addNewBadPixels(const std::vector<unsigned int>& new_pixels);
    void annotationStencilMouseMoved(const QPoint& pt);
    void annotationStencilMouseReleased(const QPoint& pt);
    void finishTrackCreation();
    void frameNumberChanged(uint32_t frame_number);
    void removeBadPixels(const std::vector<unsigned int>& pixels);

public slots:
    void SetPlaybackEnabled(bool enabled);
    void HandlePinpointControlActivation(bool status);
    void HideStencil();
    void HighlightBadPixels(bool status);
    void OnFilterRadioButtonClicked(int id);
    void OnTrackFeatureRadioButtonClicked(int id);
    void ResetSlider();
    void SetCurrentIdx(int idx);
    void SetDisplayBoresight(bool checked);
    void SetGotoFrameEnabled(bool enabled);
    void SetRadianceCalculationEnabled(bool enabled);
    void SetThreshold(const QVariant& data);
    void SetFrameTimeToggle(bool checked);
    void ShowStencil();
    void StartTimer();
    void StopTimer();
    void UpdateFps();
    void UpdateFrameData(std::vector<PlottingFrameData> input_data);
    void UpdateFrameVector(std::vector<double> original, std::vector<uint8_t> converted, arma::mat offsets_matrix);
    void ViewFrame(uint32_t frame_number);

private:
    void SetupUi();
    void SetupConnections();
    void InitMainWindow();
    void StartRecording();
    void StopRecording();

    bool recording_ = false;

    QPointer<SirveApp> mw_;
    QPointer<PlaybackController> playback_controller_;
    QPointer<VideoDisplay> video_display_;
    QPointer<StateManager> state_manager_;

    QPointer<QVBoxLayout> layout_;
    QPointer<QLineEdit> goto_frame_;
    QPointer<QIntValidator> frame_number_validator_;
    QPointer<QLabel> lbl_fps_;
    QPointer<QSlider> slider_;
    QPointer<QPushButton> play_, pause_, reverse_, next_, prev_, record_, save_, zoom_, calculate_radiance_, popout_,
        increase_fps_, decrease_fps_;

    QPointer<QWidget> original_parent_, central_panel_;
    QPointer<QDialog> popout_dialog_;
};

class VideoPlayerPopout : public QDialog
{
    Q_OBJECT
public:
    VideoPlayerPopout(QWidget *parent, VideoPlayer* video_player);

private:
    QPointer<VideoPlayer> video_player_;
};

#endif //VIDEO_PLAYER_H
