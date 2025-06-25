#include "video_display.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include "shared_tracking_functions.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QPainter>
#include <qscreen.h>

const QString VideoDisplay::kBoldLargeStyleSheet = "color: black; font-weight: bold; font-size: 12px";

VideoDisplay::VideoDisplay(QWidget* parent, StateManager* state_manager, const QVector<QRgb>& starting_color_table) :
    QWidget(parent), state_manager_(state_manager), color_table(std::move(starting_color_table))
{
    lbl_image_canvas = new EnhancedLabel(this);
    video_display_layout = new QVBoxLayout();
    video_display_layout->addStretch(1);
    SetupBlankFrame();
    SetupCreateTrackControls();
    SetupPinpointDisplay();
    SetupLabels();
    InitializeToggles();
    EstablishStencil();

    connect(lbl_image_canvas, &EnhancedLabel::hoverPoint, this, &VideoDisplay::DisplayManualBox);

    QScreen* screen = QApplication::primaryScreen();
    screenResolution = screen->size();
}

void VideoDisplay::EstablishStencil()
{
    annotation_stencil = new AnnotationStencil(this->lbl_image_canvas);
    annotation_stencil->hide();
    annotation_stencil->move(50, 50);

    connect(lbl_image_canvas, &EnhancedLabel::hoverPoint, this, &VideoDisplay::DisplayManualBox);
    connect(lbl_image_canvas, &EnhancedLabel::cursorInImage, this, &VideoDisplay::SetSelectCentroidBtn);
}

void VideoDisplay::SetSelectCentroidBtn(bool status)
{
    if (in_track_creation_mode && !btn_pinpoint->isChecked()) {
        if (status) {
            btn_select_track_centroid->setChecked(!is_zoom_active);
            cursor_in_image = true;
        } else {
            cursor_in_image = false;
        }
        UpdateDisplayFrame();
    }
}

void VideoDisplay::OnTrackFeatureRadioButtonClicked(int id)
{
    if (id == 1) {
        trackFeature = "INTENSITY_WEIGHTED_CENTROID";
    }
    if (id == 2) {
        trackFeature = "CENTROID";
    }
    if (id == 3) {
        trackFeature = "PEAK";
    }
}

void VideoDisplay::OnFilterRadioButtonClicked(int id)
{
    if (id == 1) {
        prefilter = "NONE";
    }
    if (id == 2) {
        prefilter = "GAUSSIAN";
    }
    if (id == 3) {
        prefilter = "MEDIAN";
    }
    if (id == 4) {
        prefilter = "NLMEANS";
    }
}

void VideoDisplay::SetCurrentIdx(int current_idx_new)
{
    if (current_idx_new == -1) {
        return;
    }
    current_idx = current_idx_new;
}

void VideoDisplay::SetThreshold(const QVariant& data)
{
    threshold = data.toInt();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoDisplay::SetupBlankFrame() {
    QImage frame(width, height, QImage::Format_Grayscale16);
    frame.fill(0);
    lbl_image_canvas->setPixmap(QPixmap::fromImage(frame));
    lbl_image_canvas->setMinimumSize(width, height);
    lbl_image_canvas->update();
    lbl_image_canvas->repaint();

    lbl_image_canvas->update();
    lbl_image_canvas->repaint();
}

void VideoDisplay::InitializeToggles()
{
    banner_color = QString("yellow");
    banner_text = QString("EDIT CLASSIFICATION");
    OSM_track_color = QString("blue");
    QColor new_color(QString("yellow"));
    bad_pixel_color = new_color;
}

void VideoDisplay::SetupCreateTrackControls()
{
    grp_create_track = new QGroupBox("Track Editing");
    grp_create_track->setMaximumHeight(150);
    grp_create_track->setStyleSheet(kBoldLargeStyleSheet);
    track_details_min_frame = 0;
    track_details_max_frame = 0;

    QVBoxLayout* vlayout_create_track = new QVBoxLayout(grp_create_track);

    lbl_create_track = new QLabel("");
    lbl_create_track->setWordWrap(false);

    QVBoxLayout* vlayout_track_centroid = new QVBoxLayout();
    btn_select_track_centroid = new QPushButton("Select Track Centroid");
    btn_select_track_centroid->setCheckable(true);
    connect(btn_select_track_centroid, &QPushButton::clicked, this, &VideoDisplay::HandleBtnSelectTrackCentroid);
    vlayout_track_centroid->addWidget(btn_select_track_centroid);
    QFormLayout* form_ROI_dim = new QFormLayout;
    txt_ROI_dim = new QLineEdit("30");
    txt_ROI_dim->setFixedWidth(60);
    form_ROI_dim->addRow(tr("&ROI Dim"), txt_ROI_dim);
    vlayout_track_centroid->addLayout(form_ROI_dim);

    chk_show_crosshair = new QCheckBox("Show Crosshair");
    chk_show_crosshair->setChecked(true);
    chk_snap_to_feature = new QCheckBox("Snap to Feature");
    chk_snap_to_feature->setChecked(true);
    QVBoxLayout* vlayout_crosshair = new QVBoxLayout;
    vlayout_crosshair->addWidget(chk_show_crosshair);
    vlayout_crosshair->addWidget(chk_snap_to_feature);

    chk_auto_advance_frame = new QCheckBox("Auto Advance");
    QFormLayout* form_frame_advance_increment = new QFormLayout;
    txt_frame_advance_amt = new QLineEdit("1");
    form_frame_advance_increment->addRow(tr("&# Frames"), txt_frame_advance_amt);
    txt_frame_advance_amt->setFixedWidth(30);
    btn_clear_track_centroid = new QPushButton("Remove Track Pt\nFrom Frame");
    connect(btn_clear_track_centroid, &QPushButton::clicked, this, &VideoDisplay::HandleClearTrackCentroidClick);
    connect(txt_frame_advance_amt, &QLineEdit::textChanged, this, &VideoDisplay::HandleFrameAdvanceAmtEntry);

    QPushButton* btn_finish_create_track = new QPushButton("Finish Track\nEditing");
    connect(btn_finish_create_track, &QPushButton::clicked, this, &VideoDisplay::finishTrackCreation);

    QVBoxLayout* vlayout_frame_adv = new QVBoxLayout;
    vlayout_frame_adv->addWidget(chk_auto_advance_frame);
    vlayout_frame_adv->addLayout(form_frame_advance_increment);
    QHBoxLayout* hlayout_create_track = new QHBoxLayout();
    hlayout_create_track->addLayout(vlayout_track_centroid);
    hlayout_create_track->addLayout(vlayout_crosshair);
    hlayout_create_track->addLayout(vlayout_frame_adv);
    hlayout_create_track->addWidget(txt_frame_advance_amt);
    hlayout_create_track->addWidget(btn_clear_track_centroid);
    hlayout_create_track->addWidget(btn_finish_create_track);
    hlayout_create_track->insertStretch(-1, 0);
    hlayout_create_track->insertStretch(0, 0);
    hlayout_create_track->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    grp_create_track->setHidden(true);

    QSpacerItem* vspacer_item10 = new QSpacerItem(1, 10);
    vlayout_create_track->addItem(vspacer_item10);
    vlayout_create_track->addWidget(lbl_create_track);
    vlayout_create_track->addLayout(hlayout_create_track);
    video_display_layout->insertStretch(0, -1);
    video_display_layout->addWidget(grp_create_track);
}

void VideoDisplay::SetupPinpointDisplay()
{
    grp_pinpoint = new QGroupBox("Selected Pixels");
    grp_pinpoint->setStyleSheet(kBoldLargeStyleSheet);

    QHBoxLayout* pinpoint_layout = new QHBoxLayout(grp_pinpoint);

    lbl_pinpoint = new QLabel();

    btn_pinpoint = new QPushButton();
    btn_pinpoint->setMaximumSize(40, 40);
    btn_pinpoint->setIcon(QIcon(":icons/crosshair.png"));
    btn_pinpoint->setToolTip("Pinpoint");
    btn_pinpoint->setCheckable(true);
    btn_pinpoint->setEnabled(false);

    connect(btn_pinpoint, &QPushButton::clicked, this,
            &VideoDisplay::HandleBtnPinpoint);

    QVBoxLayout* button_layout = new QVBoxLayout();

    btn_pinpoint_bad_pixel = new QPushButton("Mark as Bad Pixels");
    btn_pinpoint_bad_pixel->setEnabled(false);

    connect(btn_pinpoint_bad_pixel, &QPushButton::clicked, this,
            &VideoDisplay::AddPinpointsToBadPixelMap);
    btn_pinpoint_good_pixel = new QPushButton("Mark as Good Pixels");
    btn_pinpoint_good_pixel->setEnabled(false);

    connect(btn_pinpoint_good_pixel, &QPushButton::clicked, this,
            &VideoDisplay::RemovePinpointsFromBadPixelMap);

    button_layout->addWidget(btn_pinpoint_bad_pixel);
    button_layout->addWidget(btn_pinpoint_good_pixel);

    btn_clear_pinpoints = new QPushButton();
    btn_clear_pinpoints->setMaximumSize(40, 40);
    btn_clear_pinpoints->setIcon(QIcon(":icons/cancel.png"));
    btn_clear_pinpoints->setToolTip("Clear");
    btn_clear_pinpoints->setEnabled(false);

    connect(btn_clear_pinpoints, &QPushButton::clicked, this,
            &VideoDisplay::ClearPinpoints);

    pinpoint_layout->addWidget(btn_pinpoint);
    pinpoint_layout->addWidget(lbl_pinpoint);
    pinpoint_layout->addStretch(1);
    pinpoint_layout->addLayout(button_layout);
    pinpoint_layout->addWidget(btn_clear_pinpoints);

    video_display_layout->addWidget(grp_pinpoint);
}

void VideoDisplay::SetupLabels()
{
    lbl_image_canvas->setBackgroundRole(QPalette::Base);
    lbl_image_canvas->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lbl_image_canvas->setScaledContents(true);
    lbl_image_canvas->setObjectName("video_object");

    connect(lbl_image_canvas, &EnhancedLabel::areaHighlighted, this, &VideoDisplay::HandleImageAreaSelection);
    connect(lbl_image_canvas, &EnhancedLabel::rightClicked, this, &VideoDisplay::UndoZoom);
    connect(lbl_image_canvas, &EnhancedLabel::clicked, this, &VideoDisplay::HandlePixelSelection);

    video_display_layout->insertWidget(0, lbl_image_canvas, 0, Qt::AlignHCenter);

    QHBoxLayout* hlayout_video_labels = new QHBoxLayout();
    lbl_video_time_midnight = new QLabel("");
    lbl_zulu_time = new QLabel("");
    lbl_video_time_midnight->setAlignment(Qt::AlignLeft);
    lbl_zulu_time->setAlignment(Qt::AlignRight);

    hlayout_video_labels->addWidget(lbl_video_time_midnight);
    hlayout_video_labels->addWidget(lbl_zulu_time);
    video_display_layout->insertLayout(2, hlayout_video_labels);

    lbl_pinpoint->setText("");
}

void VideoDisplay::HandleBtnSelectTrackCentroid(bool checked)
{
    if (checked) {
        emit clearMouseButtons();
        btn_pinpoint->setChecked(false);
        is_zoom_active = false;
        is_calculate_active = false;
        SetVideoCursor();
        lbl_image_canvas->setAttribute(Qt::WA_Hover);
    } else {
        lbl_image_canvas->unsetCursor();
    }
    UpdateDisplayFrame();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoDisplay::ExitSelectTrackCentroidMode()
{
    btn_select_track_centroid->setChecked(false);
    lbl_image_canvas->unsetCursor();
    cv::destroyAllWindows();
}

void VideoDisplay::HandleBtnPinpoint(bool checked)
{
    if (checked) {
        emit clearMouseButtons();
        ExitSelectTrackCentroidMode();
        is_zoom_active = false;
        is_calculate_active = false;
        SetVideoCursor();
    } else {
        lbl_image_canvas->unsetCursor();
    }
    UpdateDisplayFrame();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoDisplay::HandlePinpointControlActivation(bool status)
{
    btn_pinpoint->setEnabled(status);
    btn_pinpoint_bad_pixel->setEnabled(status);
    btn_pinpoint_good_pixel->setEnabled(status);
    btn_clear_pinpoints->setEnabled(status);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoDisplay::ReclaimLabel()
{
    video_display_layout->insertWidget(0, lbl_image_canvas, 0, Qt::AlignHCenter);
}

void VideoDisplay::ReceiveVideoData(int x, int y)
{
    width = x;
    height = y;
    number_pixels = width * height;
    
    zoom_manager.Clear(width, height);
    pinpoint_indices.clear();

    lbl_image_canvas->setMinimumWidth(width);
    lbl_image_canvas->setMinimumHeight(height);
}

void VideoDisplay::UpdateBannerText(const QString& input_banner_text)
{
    banner_text = input_banner_text;
    UpdateDisplayFrame();
}

void VideoDisplay::UpdateBannerColor(const QString& input_color)
{
    QColor new_color(input_color);
    banner_color = new_color;
    UpdateDisplayFrame();
}

void VideoDisplay::UpdateCursorColor(const QString& input_color)
{
    cursor_color = input_color;

   if (btn_pinpoint->isChecked())
   {
       SetVideoCursor();
   }
}

void VideoDisplay::SetVideoCursor()
{
    if ("auto detect" == cursor_color.toStdString())
    {
        lbl_image_canvas->setCursor(Qt::CrossCursor);
        return;
    }

    QString icon_path =ColorScheme::get_cursor_icon_path(cursor_color);
    QPixmap crosshairs_icon(icon_path);
    QCursor crosshairs_cursor(crosshairs_icon);
    lbl_image_canvas->setCursor(crosshairs_cursor);
}

void VideoDisplay::SetFrameTimeToggle(bool checked)
{
    display_time = checked;

    UpdateDisplayFrame();
}

void VideoDisplay::SetColorMap(QVector<QRgb> color_table)
{
    this->color_table = std::move(color_table);
    UpdateDisplayFrame();
}

void VideoDisplay::SetTrackerColor(QColor new_color)
{
    OSM_track_color = std::move(new_color);

    UpdateDisplayFrame();
}

void VideoDisplay::ToggleOsmTracks(bool input)
{
    plot_tracks = input;

    UpdateDisplayFrame();
}

void VideoDisplay::SetDisplayBoresight(bool checked)
{
    display_boresight = checked;

    UpdateDisplayFrame();
}

void VideoDisplay::ToggleActionZoom(bool status)
{
    if (status) {
        is_zoom_active = true;
        is_calculate_active = false;
        ExitSelectTrackCentroidMode();
        btn_pinpoint->setChecked(false);
    } else {
        is_zoom_active = false;
    }

    UpdateDisplayFrame();
}

void VideoDisplay::ToggleActionCalculateRadiance(bool status)
{
    if (status) {
        is_zoom_active = false;
        is_calculate_active = true;
        ExitSelectTrackCentroidMode();
        btn_pinpoint->setChecked(false);
    } else {
        is_calculate_active = false;
    }

    UpdateDisplayFrame();
}

void VideoDisplay::EnterTrackCreationMode(QPoint appPos, std::vector<std::optional<TrackDetails>> starting_track_details, int threshold_in,
                                          int bbox_buffer_pixels_in, double clamp_low_coeff_in, double clamp_high_coeff_in,
                                          std::string trackFeature_in, std::string prefilter_in)
{
    track_details = std::move(starting_track_details);
    in_track_creation_mode = true;

    threshold = threshold_in;
    clamp_low_coeff = clamp_low_coeff_in;
    clamp_high_coeff = clamp_high_coeff_in;
    trackFeature = std::move(trackFeature_in);
    prefilter = std::move(prefilter_in);
    bbox_buffer_pixels = bbox_buffer_pixels_in;

    btn_select_track_centroid->setChecked(true);
    HandleBtnSelectTrackCentroid(true);

    chk_auto_advance_frame->setChecked(true);

    ResetCreateTrackMinAndMaxFrames();
    grp_create_track->setHidden(false);

    SirveApp_x = appPos.x();
    SirveApp_y = appPos.y();
}

const std::vector<std::optional<TrackDetails>>& VideoDisplay::GetCreatedTrackDetails()
{
    return track_details;
}

void VideoDisplay::ExitTrackCreationMode()
{
    in_track_creation_mode = false;
    track_details_min_frame = 0;
    track_details_max_frame = 0;
    lbl_create_track->setText("");
    grp_create_track->setHidden(true);
    UpdateDisplayFrame();
    ExitSelectTrackCentroidMode();
}

void VideoDisplay::HandleAnnotationChanges()
{
    UpdateDisplayFrame();
}

void VideoDisplay::HandleImageAreaSelection(QRect area)
{
    // check to make sure rectangle doesn't exceed dimensions. if so, shorten
    if (area.x() + area.width() > width) {
        area.setWidth(width - area.x());
    }

    if (area.y() + area.height() > height) {
        area.setHeight(height - area.y());
    }

    if (is_zoom_active) {
        zoom_manager.ZoomImage(area);
        UpdateDisplayFrame();
    } else if (is_calculate_active) {
        Calibrate(area);
        UpdateDisplayFrame();
    }
}

void VideoDisplay::Calibrate(QRect area)
{
    // The calculation_region should be calculated by the zoom manager,
    // but I'm leaving this code as-is since it is unused and untestable
    size_t num_zooms = zoom_manager.GetZoomList().size();

    if (num_zooms == 1) {
        calculation_region = area;
    } else {
        QRect adjusted_area = area;

        for (const auto& zoom : zoom_manager.GetZoomList()) {
            int x1, y1, x2, y2;
            adjusted_area.getCoords(&x1, &y1, &x2, &y2);

            double x1_position = x1 * 1.0 / width;
            double y1_position = y1 * 1.0 / height;
            int new_x1 = std::round(x1_position * zoom.width()) + zoom.x();
            int new_y1 = std::round(y1_position * zoom.height()) + zoom.y();

            double x2_position = x2 * 1.0 / width;
            double y2_position = y2 * 1.0 / height;
            int new_x2 = std::round(x2_position * zoom.width()) + zoom.x();
            int new_y2 = std::round(y2_position * zoom.height()) + zoom.y();

            adjusted_area.setCoords(new_x1, new_y1, new_x2, new_y2);
        }

        calculation_region = adjusted_area;
    }
}

void VideoDisplay::UndoZoom()
{
    if (zoom_manager.IsCurrentlyZoomed()) {
        zoom_manager.UndoZoom();
        UpdateDisplayFrame();
    }
}

void VideoDisplay::HandlePixelSelection(QPoint origin)
{
    // Note that each element in zoom_list contains the _relative_ position within the previous zoom state
    // Simply storing the absolute position within the image makes a lot of code simpler - like the calculations in this function
    // However, due to the way we currently zoom using chained QImage.scaled calls (which require integer x/y/width/height) ...
    // ... the code must still maintain the relative zoom chain.

    // It may be worth "forcing" each view state to align (x/y/width/height) with pixel boundaries, which would enable us to ...
    // ... maintain only the absolute_zoom_list, but this would have implications for aspect ratio.
    // Storing the absolute zoom levels is not ideal (duplication), but a good half-way point that lets me keep moving for now.
    if (btn_pinpoint->isChecked() || btn_select_track_centroid->isChecked()) {
        const auto& rectangle = zoom_manager.GetAbsoluteZoomList().back();
        double absolute_x = rectangle.x() + rectangle.width() * (1.0 * origin.x() / width);
        double absolute_y = rectangle.y() + rectangle.height() * (1.0 * origin.y() / height);

        unsigned int x = std::floor(absolute_x);
        unsigned int y = std::floor(absolute_y);

        if (btn_pinpoint->isChecked()) {
            PinpointPixel(x, y);
        } else if (btn_select_track_centroid->isChecked()) {
            SelectTrackCentroid(x, y);
        }
    }
}

void VideoDisplay::SelectTrackCentroid(unsigned int x, unsigned int y)
{
    Display_res_x = screenResolution.width();
    Display_res_y = screenResolution.height();
    
    extent_window_x = 10;
    extent_window_y = SirveApp_y;
    if (Display_res_x > 1920) {
        extent_window_x = std::max(SirveApp_x - 450, extent_window_x);
    }

    TrackDetails details;

    int ROI_dim = txt_ROI_dim->text().toInt();

    uint indx = this->counter;

    cv::Mat frame, display_frame, clean_display_frame, raw_frame, raw_display_frame;

    const ProcessingState* base_processing_state = &this->state_manager_->front();
    ProcessingState& current_processing_state = this->state_manager_->back();
    for (const auto& test_state : *this->state_manager_) {
        if (test_state.method == ProcessingMethod::replace_bad_pixels) {
            base_processing_state = &test_state;
            break;
        }
    }
    int current_frame_num = starting_frame_number + indx;
    x_correction = offsets_matrix(indx, 0);
    y_correction = offsets_matrix(indx, 1);

    cv::Point frame_point;
    double peak_counts, mean_counts;
    cv::Scalar sum_counts;
    uint number_pixels;
    int frame_x, frame_y;

    SharedTrackingFunctions::GetFrameRepresentations(indx, clamp_low_coeff, clamp_high_coeff, current_processing_state.details,
                                                     base_processing_state->details, frame, prefilter, display_frame, raw_display_frame,
                                                     clean_display_frame, raw_frame);

    uint minx = std::max(0, static_cast<int>(x) - ROI_dim / 2);
    uint miny = std::max(0, static_cast<int>(y) - ROI_dim / 2);
    uint ROI_width = std::min(ROI_dim, static_cast<int>(nCols - minx));
    uint ROI_height = std::min(ROI_dim, static_cast<int>(nRows - miny));
    cv::Rect ROI(minx, miny, ROI_width, ROI_height);
    cv::Mat frame_crop_threshold;
    cv::Rect bbox = ROI;
    cv::Rect bbox_uncentered = bbox;
    SharedTrackingFunctions::FindTargetExtent(nRows, nCols, counter, clamp_low_coeff, clamp_high_coeff, clean_display_frame, threshold, bbox_buffer_pixels,
                                              frame_crop_threshold, ROI, bbox, offsets_matrix, bbox_uncentered, extent_window_x,
                                              extent_window_y);
    cv::Mat frame_crop = frame(bbox);
    cv::Mat raw_frame_crop = raw_frame(bbox_uncentered);

    SharedTrackingFunctions::GetTrackPointData(trackFeature, frame_crop, raw_frame_crop, frame_crop_threshold, frame_point, peak_counts,
                                               mean_counts, sum_counts, number_pixels);
    SharedTrackingFunctions::GetPointXY(frame_point, bbox, frame_x, frame_y);

    double frame_integration_time = frame_headers[counter].int_time;
    std::array<double, 3> measurements = {0, 0, 0};
    if (model.calibration_available) {
        measurements = SharedTrackingFunctions::CalculateSumCounts(indx, bbox_uncentered, base_processing_state->details,
                                                                    frame_integration_time, model);
    }
    double sum_relative_counts = SharedTrackingFunctions::GetAdjustedCounts(indx, bbox_uncentered, base_processing_state->details);

    details.sum_relative_counts = sum_relative_counts;
    details.centroid_x = round(x + x_correction);
    details.centroid_y = round(y + y_correction);
    if (chk_snap_to_feature->isChecked()) {
        details.centroid_x = round(frame_x + x_correction);
        details.centroid_y = round(frame_y + y_correction);
    }
    details.centroid_x_boresight = details.centroid_x - nCols2;
    details.centroid_y_boresight = details.centroid_y - nRows2;
    details.number_pixels = number_pixels;
    details.peak_counts = peak_counts;
    details.mean_counts = mean_counts;
    details.sum_counts = static_cast<uint64_t>(sum_counts[0]);
    details.peak_irradiance = measurements[0];
    details.mean_irradiance = measurements[1];
    details.sum_irradiance = measurements[2];
    details.bbox_x = bbox_uncentered.x + x_correction;
    details.bbox_y = bbox_uncentered.y + y_correction;
    details.bbox_width = bbox_uncentered.width;
    details.bbox_height = bbox_uncentered.height;
    if (track_details_min_frame == 0 || current_frame_num < track_details_min_frame) {
        track_details_min_frame = current_frame_num;
    }
    track_details_max_frame = std::max(current_frame_num, track_details_max_frame);
    UpdateCreateTrackLabel();

    track_details[current_frame_num - 1] = details;

    if (chk_auto_advance_frame->isChecked()) {
        emit advanceFrame(this->txt_frame_advance_amt->text().toInt());
    }

    UpdateDisplayFrame();
}

void VideoDisplay::HandleClearTrackCentroidClick()
{
    int current_frame_num = starting_frame_number + counter;
    track_details[current_frame_num - 1] = std::nullopt;

    if (chk_auto_advance_frame->isChecked()) {
        emit advanceFrame(this->txt_frame_advance_amt->text().toInt());
    } else {
        UpdateDisplayFrame();
    }
    ResetCreateTrackMinAndMaxFrames();
    UpdateCreateTrackLabel();
}

void VideoDisplay::HandleFrameAdvanceAmtEntry(const QString& text) const
{
    bool is_valid_int = false;
    int value = text.toInt(&is_valid_int);

    if (is_valid_int) {
        if (value > kFrameAdvanceLimit) {
            QString message =
                QString("Frame advance amount must be less than or equal to the limit of %1.").arg(kFrameAdvanceLimit);
            QMessageBox::information(nullptr, "Error", message);
        }

        if (value <= kFrameAdvanceLimit && value > osm_track_frames.size() - 2) {
            // Here we specify 'minus two', since 'minus one' would cycle through to the same point in the frame sequence after each "advance!"
            QMessageBox::information(
                nullptr, "Error", "Frame advance amount must be less the the total number of frames in the loaded video clip minus two.");
        }
    } else {
        QMessageBox::information(nullptr, "error", "Please enter a valid integer.");
    }
}

void VideoDisplay::ResetCreateTrackMinAndMaxFrames()
{
    track_details_min_frame = 0;
    track_details_max_frame = 0;

    for (int i = 0; i < track_details.size(); i++) {
        if (track_details[i].has_value()) {
            if (track_details_min_frame == 0) {
                track_details_min_frame = i + 1;
            }
            track_details_max_frame = i + 1;
        }
    }
    UpdateCreateTrackLabel();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoDisplay::UpdateCreateTrackLabel()
{
    if (track_details_min_frame == 0) {
        lbl_create_track->setText("Currently editing a track.  The track has no frames.");
    } else {
        lbl_create_track->setText(
            "Currently editing a track.  The track spans from frame " + QString::number(track_details_min_frame) + " to frame " +
            QString::number(track_details_max_frame) + ".");
    }
}

void VideoDisplay::PinpointPixel(unsigned int x, unsigned int y)
{
    unsigned int pinpoint_idx = y * width + x;

    //Disallow clicking an already-pinpointed pixel
    if (std::find(pinpoint_indices.begin(), pinpoint_indices.end(), pinpoint_idx) != pinpoint_indices.end()) {
        return;
    }

    pinpoint_indices.push_back(pinpoint_idx);

    if (pinpoint_indices.size() > 3) {
        pinpoint_indices.erase(pinpoint_indices.begin());
    }
    // Should be able to just update_partial_frame here or something
    UpdateDisplayFrame();
}

void VideoDisplay::RemovePinpointsFromBadPixelMap()
{
    if (pinpoint_indices.size() > 0) {
        emit removeBadPixels(pinpoint_indices);
    }
}

void VideoDisplay::AddPinpointsToBadPixelMap()
{
    if (pinpoint_indices.size() > 0) {
        emit addNewBadPixels(pinpoint_indices);
    }
}

void VideoDisplay::ClearPinpoints()
{
    pinpoint_indices.clear();
    UpdateDisplayFrame();
}

void VideoDisplay::UpdateFrameVector(std::vector<double> original, std::vector<uint8_t> converted, arma::mat offsets_matrix0)
{
    original_frame_vector = std::move(original);
    display_ready_converted_values = std::move(converted);
    x_correction = 0;
    y_correction = 0;
    offsets_matrix = std::move(offsets_matrix0);
    UpdateDisplayFrame();
}

void VideoDisplay::UpdateDisplayFrame()
{
    //Prevent attempts to render until the video display has been fully initialized
    if (number_pixels == 0) {
        return;
    }

    QRgb rgb_cyan = QColorConstants::Cyan.rgb();

    uint8_t* color_corrected_frame = display_ready_converted_values.data();
    frame = QImage((uchar*)color_corrected_frame, width, height, QImage::Format_Grayscale8);

    // Convert image to format_indexed. Allows color table to take effect on image
    frame = frame.convertToFormat(QImage::Format_Indexed8);
    frame.setColorTable(color_table);

    // Convert image back to RGB to facilitate use of the colors
    frame = frame.convertToFormat(QImage::Format_RGB888);

    x_correction = offsets_matrix(counter, 0);
    y_correction = offsets_matrix(counter, 1);

    if (should_show_bad_pixels && current_idx != -1) {
        for (auto i = 0; i < state_manager_->at(current_idx).replaced_pixels.size(); i++) {
            unsigned int pixel_index = state_manager_->at(current_idx).replaced_pixels[i];
            int pixel_x = pixel_index % width;
            int pixel_y = pixel_index / width;
            int new_pixel_x = pixel_x - x_correction;
            int new_pixel_y = pixel_y - y_correction;
            if (new_pixel_x < 0) {
                new_pixel_x = new_pixel_x + width;
            }
            if (new_pixel_y < 0) {
                new_pixel_y = new_pixel_y + height;
            }
            if (new_pixel_x > width) {
                new_pixel_x = new_pixel_x - width;
            }
            if (new_pixel_y > height) {
                new_pixel_y = new_pixel_y - height;
            }
            frame.setPixelColor(new_pixel_x, new_pixel_y, bad_pixel_color);
        }
    }

    QString pinpoint_text("");
    for (auto idx = 0; idx < pinpoint_indices.size(); idx++) {
        int pinpoint_idx = pinpoint_indices[idx];

        if (pinpoint_idx < original_frame_vector.size()) {
            int counts_value = original_frame_vector[pinpoint_idx];
            int pinpoint_x = pinpoint_idx % width;
            int pinpoint_y = pinpoint_idx / width;
            pinpoint_text += "Pixel: " + QString::number(pinpoint_x + 1) + "," + QString::number(pinpoint_y + 1) + ". Value: " +
                QString::number(counts_value);
            if (std::find(state_manager_->at(current_idx).replaced_pixels.begin(),
                          state_manager_->at(current_idx).replaced_pixels.end(),
                          pinpoint_idx) != state_manager_->at(current_idx).replaced_pixels.end()) {
                pinpoint_text += " * (adjusted, bad pixel)";
            }

            if (btn_pinpoint->isChecked()) {
                QRgb rgb_red = QColorConstants::Red.rgb();
                frame.setPixel(pinpoint_x, pinpoint_y, rgb_red);
            }
        } else {
            pinpoint_text += "Clicked outside of valid x/y coordinate range.";
        }

        pinpoint_text += "\n";
    }
    lbl_pinpoint->setText(pinpoint_text);

    if (in_track_creation_mode && btn_select_track_centroid->isChecked()) {
        QPainter manual_ROI_painter(&frame);
        manual_ROI_painter.setPen(QPen(rgb_cyan));

        int x = hover_pt.x();
        int y = hover_pt.y();

        auto rectangle = zoom_manager.GetAbsoluteZoomList().back();
        int ROI_box_size = txt_ROI_dim->text().toInt();
        double x_scale = rectangle.width() / width;
        double y_scale = rectangle.height() / height;

        double absolute_x = rectangle.x() + 1.0 * x * x_scale;
        double absolute_y = rectangle.y() + 1.0 * y * y_scale;

        unsigned int new_x = std::floor(absolute_x);
        unsigned int new_y = std::floor(absolute_y);

        if (cursor_in_image && rectangle.width() / ROI_box_size >= 1.5) {
            lbl_image_canvas->setCursor(Qt::BlankCursor);
            QPoint top_left(new_x - std::round(ROI_box_size / 2.), new_y - std::round(ROI_box_size / 2.));
            QPoint bottom_right(new_x + std::round(ROI_box_size / 2.), new_y + std::round(ROI_box_size / 2.));

            QRect manual_ROI_rectangle(top_left, bottom_right);
            manual_ROI_painter.drawRect(manual_ROI_rectangle);
        }
        if (chk_show_crosshair->isChecked()) {
            SetVideoCursor();
        }
        assert(starting_frame_number + counter - 1 < track_details.size());
        if (track_details[starting_frame_number + counter - 1].has_value()) {
            TrackDetails td0 = track_details[starting_frame_number + counter - 1].value();
            int new_x_in_progress_track = std::round(1.0 * (td0.centroid_x - x_correction));
            int new_y_in_progress_track = std::round(1.0 * (td0.centroid_y - y_correction));
            frame.setPixel(new_x_in_progress_track, new_y_in_progress_track, rgb_cyan);
        }
    }

    //SIRVE's display and zoom logic (a simple recursive "zoom, scale, zoom, scale" thing using QImage as the base) is problematic.

    //Qt QImage .copy/.scaled calls are susceptible to resulting in pixel problems when things do not match both these criteria:
    //1) The sub_frame .copy'd from the original image exactly matches the viewport aspect ratio and
    //2) The width/height scale directly to the viewport in an integer ratio, i.e. each new pixel is exactly 2 or 3 or 4 new pixels wide/tall
    //Otherwise, Qt just does what it wants with the original pixels, stretching random columns/rows and not keeping square pixels

    //Note: the issue with "non-square pixels" is that they're not CONSISTENTLY non-square
    //Being non-square would be okay if, for example, they were all uniformly rectangular
    //The issue described above is that _some_ rows/columns are squished while _other_ rows/columns get stretched, in a random pattern

    //The ideal way to handle zoom is probably a bit more complicated
    //I think SIRVE should continue using a QImage so that we can do things like .setPixel and such and access the pixels directly
    //However, we don't necessarily need to bake generating a whole new QImage just for the zoom feature, and
    //We don't necessarily need to display the raw QImage that we've manipulated

    //For simply displaying the image after we've done all the manipulating, particularly if we want to allow ...
    // ... 3d transformations or alternative view angles like the requested "bowl" display, ...
    // ... we probably want to look at a QGraphicsView or something.
    // It's possible to put a QImage in a GraphicsScene via a QPixMap, e.g. via https://stackoverflow.com/questions/5960074/qimage-in-a-qgraphics-scene

    //A bonus of doing this stuff is that it may get easier to allow zooms/panning/etc. in the future, e.g.
    //https://stackoverflow.com/questions/35508711/how-to-enable-pan-and-zoom-in-a-qgraphicsview
    //or
    //https://stackoverflow.com/questions/60240192/zooming-in-out-on-images-qt-c

    for (const auto& sub_frame : zoom_manager.GetZoomList()) {
        // get sub-image
        frame = frame.copy(sub_frame);

        // scale to initial aspect ratio
        frame = frame.scaled(width, height);
    }

    if (zoom_manager.IsCurrentlyZoomed()) {
        lbl_image_canvas->setStyleSheet("#video_object { border: 3px solid blue; }");
    } else {
        lbl_image_canvas->setStyleSheet("#video_object { border: 1px solid light gray; }");
    }

    double x_scale2 = 1.0;
    double y_scale2 = 1.0;

    if (zoom_manager.ZoomListExists())
    {
        auto final_zoom_level2 = zoom_manager.GetZoomList().back();

        x_scale2 = width / final_zoom_level2.width();
        y_scale2 = height / final_zoom_level2.height();
    }
    else
    {
        x_scale2 = width / nCols;
        y_scale2 = height / nRows;
    }

    double size_of_pixel_x2 = 1.0 * x_scale2;
    double size_of_pixel_y2 = 1.0 * y_scale2;
    int marker_size = 5;
    double marker_width = size_of_pixel_x2 - 1 + marker_size * 2;
    double marker_height = size_of_pixel_y2 - 1 + marker_size * 2;

    if (in_track_creation_mode) {
        if (track_details[starting_frame_number + counter - 1].has_value()) {
            QPainter track_creation_marker_painter(&frame);
            track_creation_marker_painter.setPen(QPen(rgb_cyan));

            TrackDetails td = track_details[starting_frame_number + counter - 1].value();
            int track_x = td.centroid_x;
            int track_y = td.centroid_y;
            int new_track_x = track_x - x_correction;
            int new_track_y = track_y - y_correction; 
            if (new_track_x < 0) {
                new_track_x = new_track_x + width;
            }
            if (new_track_y < 0) {
                new_track_y = new_track_y + height;
            }
            if (new_track_x > width) {
                new_track_x = new_track_x - width;
            }
            if (new_track_y > height) {
                new_track_y = new_track_y - height;
            }
            QRectF track_creation_marker = GetRectangleAroundPixel(new_track_x, new_track_y, marker_size, marker_width, marker_height);
            track_creation_marker_painter.drawRect(track_creation_marker);
        }
    }

    assert(counter < osm_track_frames.size());
    auto num_osm_tracks = osm_track_frames[counter].tracks.size();
    if (plot_tracks && num_osm_tracks > 0) {
        QPainter osm_track_marker_painter(&frame);
        osm_track_marker_painter.setPen(QPen(OSM_track_color));

        for (const auto& trackData : osm_track_frames[counter].tracks) {
            //The OSM tracks are stored offset from the center instead of the top left
            int x_center = width / 2 + trackData.second.centroid_x;
            int y_center = height / 2 + trackData.second.centroid_y;
            int new_x_center = x_center - x_correction;
            int new_y_center = y_center - y_correction;
            if (new_x_center < 0) {
                new_x_center = new_x_center + width;
            }
            if (new_y_center < 0) {
                new_y_center = new_y_center + height;
            }
            if (new_x_center > width) {
                new_x_center = new_x_center - width;
            }
            if (new_y_center > height) {
                new_y_center = new_y_center - height;
            }
            QRectF osm_track_marker = GetRectangleAroundPixel(new_x_center - 1, new_y_center - 1, marker_size, marker_width, marker_height);
            if (osm_track_marker.isNull())
                continue;
            osm_track_marker_painter.drawRect(osm_track_marker);
        }
    }

    assert(counter < manual_track_frames.size());
    auto num_manual_tracks = manual_track_frames[counter].tracks.size();
    if (num_manual_tracks > 0 && manual_track_ids_to_show.size() > 0) {
        QPainter manual_track_marker_painter(&frame);
        manual_track_marker_painter.setPen(QPen(rgb_cyan));

        for (const auto& trackData : manual_track_frames[counter].tracks) {
            int track_id = trackData.first;
            if (manual_track_ids_to_show.find(track_id) != manual_track_ids_to_show.end()) {
                QColor color = manual_track_colors[track_id];
                int track_x = trackData.second.centroid_x;
                int track_y = trackData.second.centroid_y;
                int new_track_x = track_x - x_correction;
                int new_track_y = track_y - y_correction;
                if (new_track_x < 0) {
                    new_track_x = new_track_x + width;
                }
                if (new_track_y < 0) {
                    new_track_y = new_track_y + height;
                }
                if (new_track_x > width) {
                    new_track_x = new_track_x - width;
                }
                if (new_track_y > height) {
                    new_track_y = new_track_y - height;
                }
                QRectF manual_track_marker = GetRectangleAroundPixel(new_track_x, new_track_y, marker_size, marker_width, marker_height);
                if (manual_track_marker.isNull())
                    continue;
                manual_track_marker_painter.setPen(QPen(color));
                manual_track_marker_painter.drawRect(manual_track_marker);
            }
        }
    }

    if (display_boresight) {
        QPainter p2(&frame);
        p2.setPen(QPen(banner_color));
        p2.setFont(QFont("Times", 8, QFont::Bold));

        QString boresight_txt = "Sensor Boresight \n Az: " + QString::number(display_data[counter].azimuth_sensor) + "\n El " +
            QString::number(display_data[counter].elevation_sensor);

        p2.drawText(frame.rect(), Qt::AlignBottom | Qt::AlignLeft, boresight_txt);
    }

    double seconds_midnight = display_data[counter].seconds_past_midnight;
    lbl_video_time_midnight->setText("From Midnight " + QString::number(seconds_midnight, 'g', 8));

    QString zulu_time = GetZuluTimeString(seconds_midnight);
    lbl_zulu_time->setText(zulu_time);

    if (display_time) {
        QPainter p2(&frame);
        p2.setPen(QPen(banner_color));
        p2.setFont(QFont("Times", 8, QFont::Bold));

        p2.drawText(frame.rect(), Qt::AlignBottom | Qt::AlignRight, zulu_time);
    }

    // ---------------------------------------------------------------------------------------

    DrawAnnotations();

    // determine if radiance calculation is selected
    bool rectangle_drawn = calculation_region.width() > 1 && calculation_region.height() > 1;
    if (is_calculate_active && rectangle_drawn) {
        // get the coordinates of calculation region
        auto top_left = zoom_manager.GetPositionWithinZoom(calculation_region.topLeft());
        auto bottom_right = zoom_manager.GetPositionWithinZoom(calculation_region.bottomRight());

        // determine if calculation region is within zoomed image
        if (top_left.x() >= 0 && bottom_right.x() >= 0) {
            // get frame data from original data set and convert mat
            std::vector<double> calibrate_original_frame_vector(state_manager_->front().details.frames_16bit[counter].begin(),
                                                                state_manager_->front().details.frames_16bit[counter].end());
            arma::vec original_image_vector(calibrate_original_frame_vector);
            arma::mat original_mat_frame(original_image_vector);
            original_mat_frame.reshape(width, height);
            original_mat_frame = original_mat_frame.t();

            // get counts sub-matrix corresponding to the calculation region
            unsigned int ur1 = (unsigned int)top_left.x();
            unsigned int uc1 = (unsigned int)top_left.y();
            unsigned int ur2 = (unsigned int)bottom_right.x();
            unsigned int uc2 = (unsigned int)bottom_right.y();

            arma::mat counts = original_mat_frame.submat(ur1, uc1, ur2, uc2);

            // clear all temporary variables
            calibrate_original_frame_vector.clear();
            original_mat_frame.clear();

            double frame_integration_time = frame_headers[counter].int_time;
            auto measurements = model.MeasureSumCounts(
                top_left.x(), top_left.y(), bottom_right.x(), bottom_right.y(), counts, frame_integration_time);

            // -----------------------------------------------------------------------------------
            // print radiance calculation data onto frame
            QString max_value = QString::number(measurements[0]) + " uW/cm^2-sr";
            QString avg_value = QString::number(measurements[1]) + " uW/cm^2-sr";
            QString sum_value = QString::number(measurements[2]) + " uW/cm^2-sr";

            QString calculation_text = "     **** Beta Calc.: ****\n";
            calculation_text.append("Max Pixel: " + max_value + "\n");
            calculation_text.append("Avg Pixel: " + avg_value + "\n");
            calculation_text.append("Total: " + sum_value);

            QPainter painter_calculation_text(&frame);
            painter_calculation_text.setPen(QPen(banner_color));
            painter_calculation_text.setFont(QFont("Times", 8, QFont::Bold));
            painter_calculation_text.drawText(frame.rect(), Qt::AlignTop | Qt::AlignRight, calculation_text);
            painter_calculation_text.end();

            // -----------------------------------------------------------------------------------
            // draw rectangle of calculation region
            QPainter calculation_area_painter(&frame);

            QPen pen_calculation_area;
            pen_calculation_area.setStyle(Qt::DashDotLine);
            pen_calculation_area.setWidth(3);
            pen_calculation_area.setBrush(OSM_track_color);

            QRect zoomed_rectangle(top_left, bottom_right);

            calculation_area_painter.setPen(pen_calculation_area);
            calculation_area_painter.drawRect(zoomed_rectangle);
        }
    }

    // Draw banner text
    QPainter p1(&frame);
    p1.setPen(QPen(banner_color));
    p1.setFont(QFont("Times New Roman", 12, QFont::Bold));
    p1.drawText(frame.rect(), Qt::AlignTop | Qt::AlignHCenter, banner_text);
    p1.drawText(frame.rect(), Qt::AlignBottom | Qt::AlignHCenter, banner_text);

    if (record_frame && video_frame_number != counter) {
        video_frame_number = counter;
        AddNewFrame(frame, CV_8UC3);
    }

    lbl_image_canvas->setPixmap(QPixmap::fromImage(frame));

    lbl_image_canvas->update();
    lbl_image_canvas->repaint();
}

void VideoDisplay::DrawAnnotations()
{
    size_t num_annotations = annotation_list.size();

    // if there are annotations ...
    if (num_annotations > 0) {
        // for each annotation ...
        for (auto i = 0; i < num_annotations; i++) {
            // get frame information
            AnnotationInfo a = annotation_list[i];
            unsigned int initial_frame_annotation = a.frame_start - a.min_frame;
            unsigned int last_frame_annotation = initial_frame_annotation + ((a.frame_stop - a.frame_start) + 1);

            // check that current frame is within bounds
            if (counter >= initial_frame_annotation && counter < last_frame_annotation) {
                QString annotation_color = a.color;
                int font_size = a.font_size;
                QString annotation_text = a.text;

                auto loc = zoom_manager.GetPositionWithinZoom(a.x_pixel, a.y_pixel);
                if (loc.x() >= 0 && loc.y() >= 0) {
                    // write text
                    QPainter p_a(&frame);
                    p_a.setPen(QPen(annotation_color));
                    p_a.setFont(QFont("Times", font_size));
                    p_a.drawText(loc, annotation_text);
                }
            }
        }
    }
}

QRectF VideoDisplay::GetRectangleAroundPixel(int x_center, int y_center, int box_size, double box_width, double box_height) const
{
    if (!zoom_manager.IsCurrentlyZoomed(x_center, y_center))
        return QRectF();

    auto loc = zoom_manager.GetPositionWithinZoom(x_center, y_center);

    // If the "centroid" pixel is not in the box, this helper method returns -1, -1 ...
    // So we position the box based on loc2, the pixel below and to the right
    QPointF top_left;
    if (loc.x() < 0 && loc.y() < 0) {
        auto loc2 = zoom_manager.GetPositionWithinZoom(x_center + 1, y_center + 1);
        top_left.rx() = loc2.x() - box_width - box_size;
        top_left.ry() = loc2.y() - box_height - box_size;
    } else {
        top_left.rx() = loc.x() - box_size;
        top_left.ry() = loc.y() - box_size;
    }

    return QRectF {top_left, QSizeF {box_width, box_height}};
}

QString VideoDisplay::GetZuluTimeString(double seconds_midnight)
{
    int hour = seconds_midnight / 3600;
    int minutes = (seconds_midnight - hour * 3600) / 60;
    double seconds = seconds_midnight - hour * 3600 - minutes * 60;

    QString zulu_time("");
    if (hour < 10)
        zulu_time.append("0");
    zulu_time.append(QString::number(hour));
    zulu_time.append(":");

    if (minutes < 10)
        zulu_time.append("0");
    zulu_time.append(QString::number(minutes));
    zulu_time.append(":");

    if (seconds < 10)
        zulu_time.append("0");
    zulu_time.append(QString::number(seconds, 'f', 3));
    zulu_time.append("Z");
    return zulu_time;
}

StateManager* VideoDisplay::GetStateManager()
{
    return state_manager_;
}

const StateManager* VideoDisplay::GetStateManager() const
{
    return state_manager_;
}

uint32_t VideoDisplay::GetStartingFrameNumber() const noexcept
{
    return starting_frame_number;
}

void VideoDisplay::HighlightBadPixels(bool status)
{
    should_show_bad_pixels = status;
    UpdateDisplayFrame();
}

void VideoDisplay::HighlightBadPixelsColors(const QString& input_color)
{
    QColor new_color(input_color);
    bad_pixel_color = new_color;
    // QRgb bad_pixel_color = QColorConstants::Red.rgb();
    UpdateDisplayFrame();
}

void VideoDisplay::UpdateFrameData(std::vector<PlottingFrameData> input_data)
{
    display_data = std::move(input_data);
    UpdateDisplayFrame();
}

void VideoDisplay::InitializeTrackData(std::vector<TrackFrame> osm_frame_input, std::vector<TrackFrame> manual_frame_input)
{
    osm_track_frames = std::move(osm_frame_input);
    manual_track_frames = std::move(manual_frame_input);
}

void VideoDisplay::UpdateManualTrackData(std::vector<TrackFrame> track_frame_input)
{
    manual_track_frames = std::move(track_frame_input);
    UpdateDisplayFrame();
}

void VideoDisplay::AddManualTrackIdToShowLater(int id)
{
    manual_track_ids_to_show.insert(id);
    manual_track_colors[id] = ColorScheme::get_track_colors()[0];
}

void VideoDisplay::HideManualTrackId(int id)
{
    manual_track_ids_to_show.erase(id);
    UpdateDisplayFrame();
}

void VideoDisplay::DeleteManualTrack(int id)
{
    manual_track_ids_to_show.erase(id);
    manual_track_colors.erase(id);
    UpdateDisplayFrame();
}

void VideoDisplay::ShowManualTrackId(int id)
{
    manual_track_ids_to_show.insert(id);
    UpdateDisplayFrame();
}

void VideoDisplay::RecolorManualTrack(int id, QColor color)
{
    manual_track_colors[id] = std::move(color);
    UpdateDisplayFrame();
}

void VideoDisplay::InitializeFrameData(unsigned int frame_number, std::vector<PlottingFrameData> input_data,
                                       std::vector<ABIRFrameHeader> input_frame_header)
{
    starting_frame_number = frame_number;
    display_data = std::move(input_data);
    frame_headers = std::move(input_frame_header);
}

void VideoDisplay::SetCalibrationModel(CalibrationData input)
{
    model = std::move(input);
}

bool VideoDisplay::StartRecording(const QString& file_name, double fps)
{
    std::string file_string = file_name.toLocal8Bit().constData();

    video.open(file_string, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(width, height));

    bool video_opened = video.isOpened();

    if (video_opened)
        record_frame = true;

    return video_opened;
}

auto VideoDisplay::AddNewFrame(const QImage& img, int format) -> void
{
    QImage image = img.rgbSwapped();
    cv::Mat output_frame(image.height(), image.width(), format, image.bits(), image.bytesPerLine());
    video.write(output_frame);
}

void VideoDisplay::StopRecording()
{
    video.release();
    record_frame = false;
}


void VideoDisplay::SaveFrame()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Images (*.png)"));

    if (!fileName.isEmpty()) {
        (void)frame.save(fileName);
    }
}

void VideoDisplay::ResetFrame()
{
    SetupBlankFrame();
    lbl_video_time_midnight->setText("");
    lbl_zulu_time->setText("");
    lbl_pinpoint->setText("");

    frame_data.clear();

    width = SirveAppConstants::VideoDisplayWidth;
    height = SirveAppConstants::VideoDisplayHeight;
    number_pixels = 0;

    zoom_manager.Clear(width, height);
    original_frame_vector.clear();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoDisplay::ShowStencil()
{
    annotation_stencil->show();
}

void VideoDisplay::HideStencil()
{
    annotation_stencil->hide();
    UpdateDisplayFrame();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void VideoDisplay::InitializeStencilData(AnnotationInfo data)
{
    annotation_stencil->InitializeData(std::move(data));
}

void VideoDisplay::SetVideoDimensions()
{
    //get the length width, need to apply corrections for larger d data screen size
    height = lbl_image_canvas->pixmap().height();
    width = lbl_image_canvas->pixmap().width();

    // if x, ychange then rows and columns and resolutions needs to be changed
    nRows = height;
    nCols = width;
    nRows2 = nRows/2;
    nCols2 = nCols/2;

    zoom_manager.Clear(width, height);
    ReceiveVideoData(width, height);
    
    QScreen* screen = QApplication::primaryScreen();
    screenResolution = screen->size();

}

void VideoDisplay::DisplayManualBox(QPoint pt)
{
    if (in_track_creation_mode) {
        hover_pt = pt;
        UpdateDisplayFrame();
    }
}


