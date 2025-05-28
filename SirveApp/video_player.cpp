#include "video_player.h"

#include <armadillo>

#include "SirveApp.h"
#include "playback_controller.h"
#include "video_display.h"
#include "data_structures.h"

VideoPlayer::VideoPlayer(QWidget* parent, StateManager* state_manager, QVector<QRgb> color_table) :
    QWidget(parent), state_manager_(state_manager)
{
    setObjectName("video_player");

    if (state_manager == nullptr) { throw std::runtime_error("State manager is null"); }

    video_display_ = new VideoDisplay(this, state_manager, std::move(color_table));
    playback_controller_ = new PlaybackController(this);

    SetupUi();
    SetupConnections();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::AddAnnotation(AnnotationInfo anno)
{
    video_display_->annotation_list.push_back(std::move(anno));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::AddManualTrackIdToShowLater(int id)
{
    video_display_->AddManualTrackIdToShowLater(id);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::ClearAnnotations()
{
    if (video_display_->annotation_list.size() > 0)
    {
        video_display_->annotation_list.clear();
        video_display_->EstablishStencil();
        video_display_->HideStencil();
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::DeleteManualTrack(int id)
{
    video_display_->DeleteManualTrack(id);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::EnterTrackCreationMode(QPoint appPos, std::vector<std::optional<TrackDetails>> starting_track_details,
    int threshold, int bbox_buffer_pixels, double clamp_low_coeff, double clamp_high_coeff, std::string trackFeature,
    std::string prefilter)
{
    video_display_->EnterTrackCreationMode(appPos, std::move(starting_track_details), threshold, bbox_buffer_pixels,
        clamp_low_coeff, clamp_high_coeff, std::move(trackFeature), std::move(prefilter));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::ExitTrackCreationMode()
{
    video_display_->ExitTrackCreationMode();
}

const std::vector<AnnotationInfo>& VideoPlayer::GetAnnotations() const noexcept
{
    return video_display_->annotation_list;
}

std::vector<AnnotationInfo>& VideoPlayer::GetAnnotations() noexcept
{
    return video_display_->annotation_list;
}

const QString& VideoPlayer::GetBannerText() const
{
    return video_display_->banner_text;
}

uint32_t VideoPlayer::GetCounter() const
{
    return video_display_->counter;
}

// ReSharper disable once CppMemberFunctionMayBeConst
const std::vector<std::optional<TrackDetails>>& VideoPlayer::GetCreatedTrackDetails()
{
    return video_display_->GetCreatedTrackDetails();
}

uint32_t VideoPlayer::GetCurrentFrameNumber() const
{
    return playback_controller_->GetCurrentFrameNumber();
}

const std::vector<ABIRFrameHeader>& VideoPlayer::GetFrameHeaders() const
{
    return video_display_->frame_headers;
}

int VideoPlayer::GetImageWidth() const
{
    return video_display_->width;
}

int VideoPlayer::GetImageHeight() const
{
    return video_display_->height;
}

QVBoxLayout* VideoPlayer::GetLayout() const
{
    return layout_;
}

int VideoPlayer::GetXCorrection() const
{
    return video_display_->x_correction;
}

int VideoPlayer::GetYCorrection() const
{
    return video_display_->y_correction;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::HideManualTrackId(int id)
{
    video_display_->HideManualTrackId(id);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::HighlightBadPixelsColors(const QString& color)
{
    video_display_->HighlightBadPixelsColors(color);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::InitializeFrameData(unsigned int min_frame, std::vector<PlottingFrameData> input_data,
                                      std::vector<ABIRFrameHeader> input_frame_header)
{
    auto numFrames = input_frame_header.size();
    video_display_->InitializeFrameData(min_frame, std::move(input_data), std::move(input_frame_header));

    if (numFrames > 0) {
        frame_number_validator_->setRange(min_frame, min_frame + numFrames - 1);
        goto_frame_->setText(QLocale::c().toString(min_frame));

        playback_controller_->SetNumberOfFrames(numFrames);
        slider_->setRange(0, numFrames - 1);
    } else {
        goto_frame_->setText("");
        slider_->setRange(0, 0);
        frame_number_validator_->setRange(0, 0);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::InitializeStencilData(AnnotationInfo data)
{
    video_display_->InitializeStencilData(std::move(data));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::InitializeToggles()
{
    video_display_->InitializeToggles();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::InitializeTrackData(std::vector<TrackFrame> osm_frame_input, std::vector<TrackFrame> manual_frame_input)
{
    video_display_->InitializeTrackData(std::move(osm_frame_input), std::move(manual_frame_input));
}

bool VideoPlayer::IsPoppedOut() const
{
    return popout_dialog_ != nullptr;
}

bool VideoPlayer::IsRunning() const
{
    return playback_controller_->IsRunning();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::ReceiveVideoData(int x, int y)
{
    video_display_->ReceiveVideoData(x, y);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::RecolorManualTrack(int id, QColor color)
{
    video_display_->RecolorManualTrack(id, std::move(color));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::RemoveFrame()
{
    video_display_->ResetFrame();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::SetCalibrationModel(CalibrationData input)
{
    video_display_->SetCalibrationModel(std::move(input));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::SetColorMap(QVector<QRgb> color_table)
{
    video_display_->SetColorMap(std::move(color_table));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::SetTrackerColor(QColor color)
{
    video_display_->SetTrackerColor(std::move(color));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::ShowManualTrackId(int id)
{
    video_display_->ShowManualTrackId(id);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::ToggleOsmTracks(bool input)
{
    video_display_->ToggleOsmTracks(input);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::TogglePopout()
{

    auto pop_in = [this] {
        if (original_parent_) {
            popout_->setChecked(false);
            setParent(original_parent_);
            original_parent_->layout()->addWidget(this);
            popout_->setToolTip("Pop Out Video Player");
            original_parent_->show();
            original_parent_ = nullptr;
        }
        if (central_panel_)
        {
            central_panel_->show();
        }

        mw_->VideoPopoutToggled(false);
    };

    if (popout_dialog_.isNull()) {
        original_parent_ = parentWidget();
        auto minimum_width = original_parent_->minimumWidth();
        original_parent_->layout()->removeWidget(this);
        setParent(nullptr);
        original_parent_->hide();

        central_panel_ = original_parent_->parentWidget();
        central_panel_->hide();
        if (mw_ == nullptr)
        {
            InitMainWindow();
        }
        mw_->VideoPopoutToggled(true);


        popout_dialog_ = new QDialog(nullptr, Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
        popout_dialog_->setMinimumWidth(minimum_width);
        popout_dialog_->setAttribute(Qt::WA_DeleteOnClose);
        popout_dialog_->setSizeGripEnabled(true);
        popout_dialog_->setWindowTitle("Video Player");
        popout_dialog_->setLayout(new QGridLayout);
        popout_dialog_->layout()->addWidget(this);

        popout_->setToolTip("Pop In Video Player");
        popout_->setChecked(true);
        popout_dialog_->show();

        connect(popout_dialog_, &QDialog::finished, pop_in);
    } else if (original_parent_ != nullptr) {
        pop_in();

        if (popout_dialog_) {
            popout_dialog_->close();
        }
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::UpdateBannerColor(QString color)
{
    video_display_->UpdateBannerColor(std::move(color));
}

void VideoPlayer::UpdateCursorColor(QString color)
{
    video_display_->UpdateCursorColor(std::move(color));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::UpdateBannerText(const QString& text)
{
    video_display_->UpdateBannerText(text);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::UpdateManualTrackData(std::vector<TrackFrame> track_frame_input)
{
    video_display_->UpdateManualTrackData(std::move(track_frame_input));
}

void VideoPlayer::Close()
{
    if (popout_dialog_) {
        popout_dialog_->close();
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::SetPlaybackEnabled(bool enabled)
{
    play_->setEnabled(enabled);
    pause_->setEnabled(enabled);
    reverse_->setEnabled(enabled);
    next_->setEnabled(enabled);
    prev_->setEnabled(enabled);
    goto_frame_->setEnabled(enabled);
    slider_->setEnabled(enabled);
    record_->setEnabled(enabled);
    save_->setEnabled(enabled);
    zoom_->setEnabled(enabled);
    increase_fps_->setEnabled(enabled);
    decrease_fps_->setEnabled(enabled);

    if (!enabled) {
        playback_controller_->StopTimer();
        lbl_fps_->setText("");
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::HandlePinpointControlActivation(bool status)
{
    video_display_->HandlePinpointControlActivation(status);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::HideStencil()
{
    video_display_->HideStencil();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::HighlightBadPixels(bool status)
{
    video_display_->HighlightBadPixels(status);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::OnFilterRadioButtonClicked(int id)
{
    video_display_->OnFilterRadioButtonClicked(id);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::OnTrackFeatureRadioButtonClicked(int id)
{
    video_display_->OnTrackFeatureRadioButtonClicked(id);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::ResetSlider()
{
    // TODO: Investigate why this is needed. There should be a better way to handle this.
    slider_->setValue(0);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::SetCurrentIdx(int idx)
{
    video_display_->SetCurrentIdx(idx);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::SetDisplayBoresight(bool checked)
{
    video_display_->SetDisplayBoresight(checked);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::SetGotoFrameEnabled(bool enabled)
{
    goto_frame_->setEnabled(enabled);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::SetRadianceCalculationEnabled(bool enabled)
{
    calculate_radiance_->setEnabled(enabled);
    if (!enabled) {
        calculate_radiance_->setChecked(false);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::SetThreshold(const QVariant& data)
{
    video_display_->SetThreshold(data);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::SetFrameTimeToggle(bool checked)
{
    video_display_->SetFrameTimeToggle(checked);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::ShowStencil()
{
    video_display_->ShowStencil();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::StartTimer()
{
    playback_controller_->StartTimer();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::StopTimer()
{
    playback_controller_->StopTimer();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::UpdateFps()
{
    auto fps = QString::number(playback_controller_->GetFps(), 'g', 2);
    lbl_fps_->setText(fps + " fps");
    lbl_fps_->setToolTip(fps);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::UpdateFrameData(std::vector<PlottingFrameData> input_data)
{
    video_display_->UpdateFrameData(std::move(input_data));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoPlayer::UpdateFrameVector(std::vector<double> original, std::vector<uint8_t> converted, arma::mat offsets_matrix)
{
    video_display_->UpdateFrameVector(std::move(original), std::move(converted), std::move(offsets_matrix));
}

/*
 * Private Slots
 */

/*
 * Private Methods
 */

void VideoPlayer::SetupUi()
{
    layout_ = new QVBoxLayout(this);
    layout_->addLayout(video_display_->video_display_layout);

    slider_ = new QSlider();
    slider_->setOrientation(Qt::Horizontal);

    layout_->addWidget(slider_);

    constexpr int button_width = 42;
    constexpr int button_height = 50;

    QPointer video_buttons = new QHBoxLayout();

    save_ = new QPushButton();
    save_->setFixedWidth(button_width);
    save_->setIcon(QIcon(":/icons/content-save.png"));
    save_->setToolTip("Save Frame");
    save_->setEnabled(false);

    record_ = new QPushButton();
    record_->setFixedWidth(button_width);
    record_->setIcon(QIcon(":/icons/record.png"));
    record_->setToolTip("Record Video");
    record_->setEnabled(false);
    video_buttons->addWidget(record_);

    zoom_ = new QPushButton();
    zoom_->setFixedWidth(button_width);
    zoom_->setIcon(QIcon(":/icons/magnify.png"));
    zoom_->setCheckable(true);
    zoom_->setEnabled(false);
    video_buttons->addWidget(zoom_);

    calculate_radiance_ = new QPushButton();
    calculate_radiance_->resize(button_width, button_height);
    calculate_radiance_->setIcon(QIcon(":/icons/signal.png"));
    calculate_radiance_->setCheckable(true);
    calculate_radiance_->setEnabled(false);
    video_buttons->addWidget(calculate_radiance_);

    popout_ = new QPushButton();
    popout_->setFixedWidth(button_width);
    popout_->setIcon(QIcon(":/icons/expand.png"));
    popout_->setToolTip("Pop Out Video Player");
    popout_->setCheckable(true);
    video_buttons->addWidget(popout_);

    goto_frame_ = new QLineEdit("");
    goto_frame_->setFixedWidth(50);
    goto_frame_->setValidator(new QIntValidator(1, std::numeric_limits<int>::max(), goto_frame_));
    goto_frame_->setEnabled(false);
    frame_number_validator_ = new QIntValidator(1, 1, goto_frame_);
    goto_frame_->setValidator(frame_number_validator_);

    QPointer form_layout = new QFormLayout;
    form_layout->setAlignment(Qt::AlignHCenter|Qt::AlignCenter);
    form_layout->addRow(tr("&Frame:"),goto_frame_);
    video_buttons->addLayout(form_layout);

    prev_ = new QPushButton();
    prev_->setFixedWidth(button_width);
    prev_->setIcon(QIcon(":/icons/skip-previous.png"));
    prev_->setProperty("id", "previous");
    prev_->setToolTip("Previous Frame");
    prev_->setEnabled(false);
    video_buttons->addWidget(prev_);

    reverse_ = new QPushButton();
    reverse_->setFixedWidth(button_width);
    reverse_->setIcon(QIcon(":/icons/reverse.png"));
    reverse_->setProperty("id", "reverse");
    reverse_->setToolTip("Reverse Video");
    reverse_->setEnabled(false);
    video_buttons->addWidget(reverse_);

    pause_ = new QPushButton();
    pause_->setFixedWidth(button_width);
    pause_->setIcon(QIcon(":/icons/pause.png"));
    pause_->setProperty("id", "pause");
    pause_->setToolTip("Pause Video");
    pause_->setEnabled(false);
    video_buttons->addWidget(pause_);

    play_ = new QPushButton();
    play_->setFixedWidth(button_width);
    play_->setIcon(QIcon(":/icons/play.png"));
    play_->setProperty("id", "play");
    play_->setToolTip("Play Video");
    play_->setEnabled(false);
    video_buttons->addWidget(play_);

    next_ = new QPushButton();
    next_->setFixedWidth(button_width);
    next_->setIcon(QIcon(":/icons/skip-next.png"));
    next_->setProperty("id", "next");
    next_->setToolTip("Next Frame");
    next_->setEnabled(false);
    video_buttons->addWidget(next_);

    lbl_fps_ = new QLabel("fps");
    lbl_fps_->setFixedWidth(40);
    video_buttons->addWidget(lbl_fps_);

    increase_fps_ = new QPushButton();
    increase_fps_->setFixedWidth(button_width);
    increase_fps_->setIcon(QIcon(":/icons/chevron-double-up.png"));
    increase_fps_->setToolTip("Increase FPS");
    increase_fps_->setEnabled(false);
    video_buttons->addWidget(increase_fps_);

    decrease_fps_ = new QPushButton();
    decrease_fps_->setFixedWidth(button_width);
    decrease_fps_->setIcon(QIcon(":/icons/chevron-double-down.png"));
    decrease_fps_->setToolTip("Decrease FPS");
    decrease_fps_->setEnabled(false);
    video_buttons->addWidget(decrease_fps_);

    video_buttons->insertStretch(0, 0);
    video_buttons->insertStretch(-1, 0);
    layout_->addLayout(video_buttons);
}

void VideoPlayer::SetupConnections()
{
    connect(next_, &QPushButton::clicked, playback_controller_, &PlaybackController::GotoNextFrame);
    connect(pause_, &QPushButton::clicked, playback_controller_, &PlaybackController::StopTimer);
    connect(play_, &QPushButton::clicked, playback_controller_, &PlaybackController::StartTimer);
    connect(prev_, &QPushButton::clicked, playback_controller_, &PlaybackController::GotoPrevFrame);
    connect(reverse_, &QPushButton::clicked, playback_controller_, &PlaybackController::ReverseTimer);
    connect(slider_, &QSlider::valueChanged, playback_controller_, &PlaybackController::SetCurrentFrameNumber);
    connect(video_display_, &VideoDisplay::advanceFrame, playback_controller_, &PlaybackController::CustomAdvanceFrame);

    connect(calculate_radiance_, &QPushButton::toggled, [this] (bool checked) {
        video_display_->ToggleActionCalculateRadiance(checked);
        if (checked) {
            zoom_->setChecked(false);
        }
    });

    connect(decrease_fps_, &QPushButton::clicked, [this] {
        playback_controller_->DecreaseTimerInterval();
        UpdateFps();
    });

    connect(goto_frame_, &QLineEdit::editingFinished, [this] {
        auto frame_number = goto_frame_->text().toUInt();
        playback_controller_->SetCurrentFrameNumber(frame_number - 1);
    });

    connect(increase_fps_, &QPushButton::clicked, [this] {
        playback_controller_->IncreaseTimerInterval();
        UpdateFps();
    });

    connect(playback_controller_, &PlaybackController::frameSelected, [this](uint32_t frame_number) {
        ViewFrame(frame_number);
    });

    connect(popout_, &QPushButton::clicked, [this] {
        TogglePopout();
    });

    connect(record_, &QPushButton::toggled, [this] (bool checked) {
        if (checked) {
            StopRecording();
        } else {
            StopRecording();
        }
    });

    connect(save_, &QPushButton::clicked, [this] {
        auto was_running = playback_controller_->IsRunning();
        if (was_running) { playback_controller_->StopTimer(); }
        video_display_->SaveFrame();
        if (was_running) { playback_controller_->StartTimer(); }
    });

    connect(video_display_, &VideoDisplay::clearMouseButtons, [this] {
        zoom_->setChecked(false);
        calculate_radiance_->setChecked(false);
    });

    connect(video_display_, &VideoDisplay::addNewBadPixels, [this](const std::vector<unsigned int>& new_pixels) {
        emit addNewBadPixels(new_pixels);
    });

    connect(video_display_, &VideoDisplay::finishTrackCreation, [this] {
        emit finishTrackCreation();
    });

    connect(video_display_, &VideoDisplay::removeBadPixels, [this] (const std::vector<unsigned int>& pixels) {
        emit removeBadPixels(pixels);
    });

    connect(video_display_->annotation_stencil, &AnnotationStencil::mouseMoved, [this](const QPoint& pt) {
        emit annotationStencilMouseMoved(pt);
    });

    connect(video_display_->annotation_stencil, &AnnotationStencil::mouseReleased, [this](const QPoint& pt) {
        emit annotationStencilMouseReleased(pt);
    });

    connect(zoom_, &QPushButton::clicked, [this] {
        auto zoomed = zoom_->isChecked();
        video_display_->ToggleActionZoom(zoomed);
        if (zoomed) { calculate_radiance_->setChecked(false); }
    });
}

std::once_flag mw_once_flag;

void VideoPlayer::InitMainWindow()
{
    std::call_once(mw_once_flag, [this] {
        mw_ = SirveApp::GetMainWindow();
    });

    if (mw_.isNull()) {
        throw std::runtime_error("Main window is null");
    }
}

void VideoPlayer::ViewFrame(uint32_t frame_number)
{
    InitMainWindow();

    int num_video_frames = state_manager_->GetCurrentState().details.frames_16bit.size();
    video_display_->ViewFrame(frame_number);
    // TODO: This is fugly. This should probably be handled by accessors and/or signals
    int current_auto_track_start = mw_->txt_auto_track_start_frame->text().toInt();
    int current_auto_track_stop = mw_->txt_auto_track_stop_frame->text().toInt();
    int current_frame_number = frame_number + mw_->txt_start_frame->text().toInt();
    int min_frame = mw_->txt_start_frame->text().toInt();
    int max_frame = min_frame + num_video_frames - 1;

    mw_->txt_auto_track_start_frame->setText(QString::number(current_frame_number));
    if ((current_frame_number >= current_auto_track_start) &&
        ((mw_->txt_auto_track_stop_frame->text().toInt() - mw_->txt_auto_track_start_frame->text().toInt())<2))
    {
        mw_->txt_auto_track_stop_frame->setText(
            QString::number(std::min(static_cast<double>(current_frame_number+1), static_cast<double>(max_frame))));
        current_auto_track_stop = mw_->txt_auto_track_stop_frame->text().toInt();
    }
    if (current_frame_number >= current_auto_track_stop)
    {
        int new_auto_track_stop = std::min(static_cast<double>(current_frame_number + 1), static_cast<double>(max_frame));
        mw_->txt_auto_track_stop_frame->setText(QString::number(new_auto_track_stop));
    }

    goto_frame_->setText(QLocale::c().toString(video_display_->GetStartingFrameNumber() + frame_number));
    slider_->setValue(frame_number);

    mw_->UpdateGlobalFrameVector();
    emit frameNumberChanged(frame_number);
}

void VideoPlayer::StartRecording()
{
    // TODO: This is fugly. Start and stop frame stuff should be handled by the PlaybackController
    QString start_frame = mw_->txt_start_frame->text();
    QString stop_frame = mw_->txt_stop_frame->text();
    QString base_folder = mw_->config_values.workspace_folder;
    QDate today = QDate::currentDate();
    QTime currentTime = QTime::currentTime();;
    QString formattedDate = today.toString("yyyyMMdd") + "_" + currentTime.toString("HHmm");
    QString suggested_name =
        base_folder + "/" + mw_->abp_file_metadata.file_name + "_" + start_frame + "_" + stop_frame + "_" + formattedDate;
    QString file_name = QFileDialog::getSaveFileName(this,
        "Save file to your workspace location using name suggested, or choose a new location/filename..",
        suggested_name, "Video (*.avi)");

    if (file_name.isEmpty()) {
        return;
    }

    if (video_display_->StartRecording(file_name, playback_controller_->GetFps())) {
        record_->setIcon(QIcon(":icons/stop.png"));
        record_->setText("");
        record_->setToolTip("Stop Record");
        record_->setEnabled(true);

        recording_ = true;
    } else {
        QtHelpers::LaunchMessageBox(QString("Video Record Failed "), "Video file could not be saved to this location");
    }
}

void VideoPlayer::StopRecording()
{
    bool calculate = calculate_radiance_->isChecked();
    video_display_->ToggleActionCalculateRadiance(calculate);
    if (calculate) {
        zoom_->setChecked(false);
    }

    recording_ = false;
}

void VideoPlayer::SetVideoDimensions()
{
    video_display_->SetVideoDimensions();
}

VideoPlayerPopout::VideoPlayerPopout(QWidget* parent, VideoPlayer* video_player): QDialog(parent), video_player_(video_player)
{
    setWindowTitle("Video Player");

    setSizeGripEnabled(true);
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
}
