#include "video_display.h"
#include "frame_player.h"

VideoDisplay::VideoDisplay(QVector<QRgb> starting_color_table, QWidget *parent) : QWidget(parent)
{
    zoom_manager = new VideoDisplayZoomManager(0, 0);
    lbl_image_canvas = new EnhancedLabel(this);
    video_display_layout = new QVBoxLayout();
    video_display_layout->addStretch(1);
    SetupCreateTrackControls();
    SetupPinpointDisplay();
    SetupLabels();

    is_zoom_active = false;
    is_calculate_active = false;
    should_show_bad_pixels = false;
    in_track_creation_mode = false;
    display_boresight = false;
    cursor_in_image = false;

    counter = 0;
    starting_frame_number = 0;
    counter_record = 0;
    record_frame = false;
    number_pixels = 0;

    InitializeToggles();

    plot_tracks = true;
    display_time = false;

    colorTable = starting_color_table;

    original_frame_vector = {};

    xCorrection = 0;
    yCorrection = 0;
    current_idx = -1;

    EstablishStencil();

    connect(lbl_image_canvas, &EnhancedLabel::hoverPoint, this, &VideoDisplay::DisplayManualBox);
}

void VideoDisplay::EstablishStencil()
{
    annotation_stencil = new AnnotationStencil(this->lbl_image_canvas);
    annotation_stencil->hide();
    annotation_stencil->move(50, 50);

    connect(lbl_image_canvas, &EnhancedLabel::hoverPoint, this, &VideoDisplay::DisplayManualBox);
    connect(lbl_image_canvas, &EnhancedLabel::cursorInImage, this, &VideoDisplay::SetSelectCentroidBtn);
}


VideoDisplay::~VideoDisplay()
{
    delete lbl_image_canvas;

    delete lbl_frame_number;
    delete lbl_video_time_midnight;
    delete lbl_zulu_time;

    delete zoom_manager;
}

void VideoDisplay::SetSelectCentroidBtn(bool status)
{
    if (in_track_creation_mode && !btn_pinpoint->isChecked())
    {
        if (is_zoom_active && status){
            btn_select_track_centroid->setChecked(false);
            cursor_in_image = true;
        }
        else if (!is_zoom_active && status){
            btn_select_track_centroid->setChecked(true);
            cursor_in_image = true;
        }
        else if (!status){
            // btn_select_track_centroid->setChecked(false);
            cursor_in_image = false;
        }
        UpdateDisplayFrame();
    }
}

void VideoDisplay::GetCurrentIdx(int current_idx_new)
{
    if (current_idx_new == -1)
	{
		return;
	}
	current_idx = current_idx_new;
}

void VideoDisplay::GetThreshold(int threshold_in)
{
   threshold = 6 - threshold_in;
}

void VideoDisplay::InitializeToggles()
{
	banner_color = QString("yellow");
	banner_text = QString("EDIT CLASSIFICATION");
	OSM_track_color = QString("blue");
	QColor new_color(QString("yellow"));
	bad_pixel_color = new_color;
}

void VideoDisplay::SetupCrosshairsCursor(QString icon_resource)
{
    QPixmap crosshairs_icon(icon_resource);

    if (crosshairs_icon.isNull()) {
        qWarning("Failed to load cursor icon.");
    } else {
        QCursor crosshairs_cursor(crosshairs_icon);
        lbl_image_canvas->setCursor(crosshairs_cursor);
    }
}

void VideoDisplay::SetupCreateTrackControls()
{
    grp_create_track = new QGroupBox("Track Editing");
    grp_create_track->setMaximumHeight(150);
    grp_create_track->setStyleSheet(bold_large_styleSheet);
    track_details_min_frame = 0;
    track_details_max_frame = 0;

    QVBoxLayout* vlayout_create_track = new QVBoxLayout(grp_create_track);

    lbl_create_track = new QLabel("");
    lbl_create_track->setWordWrap(false);

    QVBoxLayout *vlayout_track_centroid = new QVBoxLayout();
    btn_select_track_centroid = new QPushButton("Select Track Centroid");
    btn_select_track_centroid->setCheckable(true);
    connect(btn_select_track_centroid, &QPushButton::clicked, this, &VideoDisplay::HandleBtnSelectTrackCentroid);
    vlayout_track_centroid->addWidget(btn_select_track_centroid);
    QFormLayout *form_ROI_dim = new QFormLayout;
    txt_ROI_dim = new QLineEdit("30");
    txt_ROI_dim->setFixedWidth(60);
    form_ROI_dim->addRow(tr("&ROI Dim"),txt_ROI_dim);
    vlayout_track_centroid->addLayout(form_ROI_dim);

    chk_show_crosshair = new QCheckBox("Show Crosshair");
    chk_show_crosshair->setChecked(true);
    chk_snap_to_peak = new QCheckBox("Snap to Peak");
    chk_snap_to_peak->setChecked(true);
    QVBoxLayout *vlayout_crosshair = new QVBoxLayout;
    vlayout_crosshair->addWidget(chk_show_crosshair);
    vlayout_crosshair->addWidget(chk_snap_to_peak);

    chk_auto_advance_frame = new QCheckBox("Auto Advance");
    QFormLayout *form_frame_advance_increment = new QFormLayout;
    txt_frame_advance_amt = new QLineEdit("1");
    form_frame_advance_increment->addRow(tr("&# Frames"),txt_frame_advance_amt);
    txt_frame_advance_amt->setFixedWidth(30);
    btn_clear_track_centroid = new QPushButton("Remove Track Pt\nFrom Frame");
    connect(btn_clear_track_centroid, &QPushButton::clicked, this, &VideoDisplay::HandleClearTrackCentroidClick);
    connect(txt_frame_advance_amt, &QLineEdit::textChanged, this, &VideoDisplay::HandleFrameAdvanceAmtEntry);

    QPushButton *btn_finish_create_track = new QPushButton("Finish Track\nEditing");
    connect(btn_finish_create_track, &QPushButton::clicked, this, &VideoDisplay::finishTrackCreation);

    QVBoxLayout *vlayout_frame_adv = new QVBoxLayout;
    vlayout_frame_adv->addWidget(chk_auto_advance_frame);
    vlayout_frame_adv->addLayout(form_frame_advance_increment);
    QHBoxLayout* hlayout_create_track = new QHBoxLayout();
    hlayout_create_track->addLayout(vlayout_track_centroid);
    hlayout_create_track->addLayout(vlayout_crosshair);
    hlayout_create_track->addLayout(vlayout_frame_adv);
    hlayout_create_track->addWidget(txt_frame_advance_amt);
    hlayout_create_track->addWidget(btn_clear_track_centroid);
    hlayout_create_track->addWidget(btn_finish_create_track);
    hlayout_create_track->insertStretch(-1,0);
    hlayout_create_track->insertStretch(0,0);
    hlayout_create_track->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);

    grp_create_track->setHidden(true);

    QSpacerItem *vspacer_item10 = new QSpacerItem(1,10);
    vlayout_create_track->addItem(vspacer_item10);
    vlayout_create_track->addWidget(lbl_create_track);
    vlayout_create_track->addLayout(hlayout_create_track);
    video_display_layout->insertStretch(0, -1);
    video_display_layout->addWidget(grp_create_track);
}

void VideoDisplay::SetupPinpointDisplay()
{
    grp_pinpoint = new QGroupBox("Selected Pixels");
    grp_pinpoint->setStyleSheet(bold_large_styleSheet);

    QHBoxLayout *pinpoint_layout = new QHBoxLayout(grp_pinpoint);

    lbl_pinpoint = new QLabel();

    btn_pinpoint = new QPushButton();
    btn_pinpoint->setMaximumSize(40, 40);
    btn_pinpoint->setIcon(QIcon(":icons/crosshair.png"));
    btn_pinpoint->setToolTip("Pinpoint");
    btn_pinpoint->setCheckable(true);
    btn_pinpoint->setEnabled(false);

    connect(btn_pinpoint, &QPushButton::clicked, this, &VideoDisplay::HandleBtnPinpoint);

    QVBoxLayout* button_layout = new QVBoxLayout();

    btn_pinpoint_bad_pixel = new QPushButton("Mark as Bad Pixels");
    btn_pinpoint_bad_pixel->setEnabled(false);

    connect(btn_pinpoint_bad_pixel, &QPushButton::clicked, this, &VideoDisplay::AddPinpointsToBadPixelMap);
    btn_pinpoint_good_pixel = new QPushButton("Mark as Good Pixels");
    btn_pinpoint_good_pixel->setEnabled(false);

    connect(btn_pinpoint_good_pixel, &QPushButton::clicked, this, &VideoDisplay::RemovePinpointsFromBadPixelMap);

    button_layout->addWidget(btn_pinpoint_bad_pixel);
    button_layout->addWidget(btn_pinpoint_good_pixel);

    btn_clear_pinpoints = new QPushButton();
    btn_clear_pinpoints->setMaximumSize(40, 40);
    btn_clear_pinpoints->setIcon(QIcon(":icons/cancel.png"));
    btn_clear_pinpoints->setToolTip("Clear");
    btn_clear_pinpoints->setEnabled(false);

    connect(btn_clear_pinpoints, &QPushButton::clicked, this, &VideoDisplay::ClearPinpoints);

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
    lbl_frame_number = new QLabel("");
    lbl_video_time_midnight = new QLabel("");
    lbl_zulu_time = new QLabel("");
    lbl_frame_number->setAlignment(Qt::AlignLeft);
    lbl_video_time_midnight->setAlignment(Qt::AlignHCenter);
    lbl_zulu_time->setAlignment(Qt::AlignRight);

    hlayout_video_labels->addWidget(lbl_frame_number);
    hlayout_video_labels->addWidget(lbl_video_time_midnight);
    hlayout_video_labels->addWidget(lbl_zulu_time);
    video_display_layout->insertLayout(2, hlayout_video_labels);

    lbl_pinpoint->setText("");
}

void VideoDisplay::HandleBtnSelectTrackCentroid(bool checked)
{
    if (checked)
    {
        emit clearMouseButtons();
        btn_pinpoint->setChecked(false);
        is_zoom_active = false;
        is_calculate_active = false;
        SetupCrosshairsCursor(":icons/crosshair-golden.png");
        lbl_image_canvas->setAttribute(Qt::WA_Hover);
    } else
    {
        lbl_image_canvas->unsetCursor();
    }
    UpdateDisplayFrame();
}

void VideoDisplay::ExitSelectTrackCentroidMode() {
    btn_select_track_centroid->setChecked(false);
    lbl_image_canvas->unsetCursor();
}

void VideoDisplay::HandleBtnPinpoint(bool checked)
{
    if (checked)
    {
        emit clearMouseButtons();
        ExitSelectTrackCentroidMode();
        is_zoom_active = false;
        is_calculate_active = false;
        SetupCrosshairsCursor(":icons/crosshair-golden_pinpoint.png");
    } else
    {
        lbl_image_canvas->unsetCursor();
    }
    UpdateDisplayFrame();
}

void VideoDisplay::HandlePinpointControlActivation(bool status)
{
    btn_pinpoint->setEnabled(status);
    btn_pinpoint_bad_pixel->setEnabled(status);
    btn_pinpoint_good_pixel->setEnabled(status);
    btn_clear_pinpoints->setEnabled(status);
}

void VideoDisplay::ReclaimLabel()
{
    video_display_layout->insertWidget(0, lbl_image_canvas, 0, Qt::AlignHCenter);
}

void VideoDisplay::ReceiveVideoData(int x, int y)
{
    image_x = x;
    image_y = y;
    number_pixels = image_x*image_y;

    delete zoom_manager;
    zoom_manager = new VideoDisplayZoomManager(image_x, image_y);
    pinpoint_indices.clear();

    lbl_image_canvas->setMinimumWidth(image_x);
    lbl_image_canvas->setMinimumHeight(image_y);
}

void VideoDisplay::UpdateBannerText(QString input_banner_text)
{
    banner_text = input_banner_text;
    UpdateDisplayFrame();
}

void VideoDisplay::UpdateBannerColor(QString input_color)
{
    QColor new_color(input_color);
    banner_color = new_color;
    UpdateDisplayFrame();
}

void VideoDisplay::HandleFrameTimeToggle(bool checked)
{
    display_time = checked;

    UpdateDisplayFrame();
}

void VideoDisplay::HandleColorMapUpdate(QVector<QRgb> color_table)
{
    colorTable = color_table;
    UpdateDisplayFrame();
}

void VideoDisplay::HandleTrackerColorUpdate(QColor new_color)
{
    OSM_track_color = new_color;

    UpdateDisplayFrame();
}

void VideoDisplay::ToggleOsmTracks(bool input)
{
    plot_tracks = input;

    UpdateDisplayFrame();
}

void VideoDisplay::HandleSensorBoresightDataCheck(bool checked)
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
    }
    else {
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
    }
    else {
        is_calculate_active = false;
    }

    UpdateDisplayFrame();
}

void VideoDisplay::EnterTrackCreationMode(std::vector<std::optional<TrackDetails>> starting_track_details)
{
    track_details = starting_track_details;
    in_track_creation_mode = true;

    btn_select_track_centroid->setChecked(true);
    HandleBtnSelectTrackCentroid(true);

    chk_auto_advance_frame->setChecked(true);

    ResetCreateTrackMinAndMaxFrames();
    grp_create_track->setHidden(false);

}

const std::vector<std::optional<TrackDetails>> & VideoDisplay::GetCreatedTrackDetails()
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
    if (area.x() + area.width() > image_x)
    {
        area.setWidth(image_x - area.x());
    }

    if (area.y() + area.height() > image_y)
    {
        area.setHeight(image_y - area.y());
    }

    if (is_zoom_active)
    {
        zoom_manager->ZoomImage(area);
        UpdateDisplayFrame();
    }
    else if (is_calculate_active)
    {
        Calibrate(area);
        UpdateDisplayFrame();
    }
    else
    {
        return;
    }
}

void VideoDisplay::Calibrate(QRect area)
{
    // The calculation_region should be calculated by the zoom manager,
    // but I'm leaving this code as-is since it is unused and untestable
    size_t num_zooms = zoom_manager->zoom_list.size();

    if (num_zooms == 1) {
        calculation_region = area;
    }

    else {
        QRect adjusted_area = area;

        for (auto i = num_zooms - 1; i > 0; i--)
        {

            int* x1, * y1, * x2, * y2;
            x1 = new int;
            y1 = new int;
            x2 = new int;
            y2 = new int;

            adjusted_area.getCoords(x1, y1, x2, y2);

            QRect zoom = zoom_manager->zoom_list[i];

            double x1_position = *x1 * 1.0 / image_x;
            double y1_position = *y1 * 1.0 / image_y;
            int new_x1 = std::round(x1_position * zoom.width()) + zoom.x();
            int new_y1 = std::round(y1_position * zoom.height()) + zoom.y();

            double x2_position = *x2 * 1.0 / image_x;
            double y2_position = *y2 * 1.0 / image_y;
            int new_x2 = std::round(x2_position * zoom.width()) + zoom.x();
            int new_y2 = std::round(y2_position * zoom.height()) + zoom.y();

            adjusted_area.setCoords(new_x1, new_y1, new_x2, new_y2);
        }

        calculation_region = adjusted_area;
    }
}

void VideoDisplay::UndoZoom()
{
    if (zoom_manager->is_currently_zoomed())
    {
        zoom_manager->UndoZoom();
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
    if (btn_pinpoint->isChecked() || btn_select_track_centroid->isChecked())
    {
        AbsoluteZoomInfo rectangle = zoom_manager->absolute_zoom_list[zoom_manager->zoom_list.size() - 1];
        double absolute_x = rectangle.x + rectangle.width * (1.0 * origin.x() / image_x);
        double absolute_y = rectangle.y + rectangle.height * (1.0 * origin.y() / image_y);

        unsigned int x = std::floor(absolute_x);
        unsigned int y = std::floor(absolute_y);

        if (btn_pinpoint->isChecked())
        {
            PinpointPixel(x, y);
        }
        else if (btn_select_track_centroid->isChecked())
        {
            SelectTrackCentroid(x, y);
        }
    }
}

void VideoDisplay::SelectTrackCentroid(unsigned int x, unsigned int y)
{
    TrackDetails details;
    AutoTracking AutoTracker;
    std::vector<uint16_t> frame_std_vector = {this->container.processing_states[this->container.current_idx].details.frames_16bit[this->counter].begin(),
           this->container.processing_states[this->container.current_idx].details.frames_16bit[this->counter].end()};  
    
    processingState base_processing_state = this->container.processing_states[0];
  
    for (auto ii = 0; ii < this->container.processing_states.size(); ii++)
    {
        processingState test_state = this->container.processing_states[ii];
        if (test_state.method == ProcessingMethod::replace_bad_pixels)
        {
            base_processing_state = test_state;
            break;
        }
            
    }
    int nrows = this->container.processing_states[this->container.current_idx].details.y_pixels;
    int ncols = this->container.processing_states[this->container.current_idx].details.x_pixels;       

    arma::vec frame_vector = arma::conv_to<arma::vec>::from(frame_std_vector);
    cv::Mat frame_matrix = cv::Mat(nrows, ncols, CV_64FC1, frame_vector.memptr());
    int ROI_dim = txt_ROI_dim->text().toInt();
    uint minx = std::max(0,static_cast<int>(x)-ROI_dim/2);
    uint miny = std::max(0,static_cast<int>(y)-ROI_dim/2);
    uint ROI_width = std::min(ROI_dim, static_cast<int>(ncols - minx));
    uint ROI_height = std::min(ROI_dim, static_cast<int>(nrows - miny));

    cv::Rect ROI(minx,miny,ROI_width,ROI_height);
    cv::Mat frame_crop = frame_matrix(ROI);
    cv::Mat frame_crop_threshold;
    cv::Scalar frame_crop_mean, frame_crop_sigma;
    cv::meanStdDev(frame_crop, frame_crop_mean, frame_crop_sigma);
    cv::Scalar sum_ROI_counts = cv::sum(frame_crop);
    int N_ROI_pixels = cv::countNonZero(frame_crop > 0);
    cv::Point frame_point;
    cv::threshold(frame_crop, frame_crop_threshold, frame_crop_mean[0]+threshold*frame_crop_sigma[0], NULL, cv::THRESH_TOZERO);
    cv::Scalar sum_counts = cv::sum(frame_crop_threshold);
    int N_threshold_pixels = cv::countNonZero(frame_crop_threshold > 0);
    double peak_counts;
    int x2 = x;
    int y2 = y;
    details.centroid_x = round(x2 + xCorrection);
    details.centroid_y = round(y2 + yCorrection);
    cv::minMaxLoc(frame_crop_threshold, NULL, &peak_counts, NULL, &frame_point); 
    if (chk_snap_to_peak->isChecked()){  
        x2 = frame_point.x;
        y2 = frame_point.y;
        details.centroid_x = round(x2 + xCorrection + ROI.x);
        details.centroid_y = round(y2 + yCorrection + ROI.y);
    }
    details.peak_counts = peak_counts;
    details.sum_counts = static_cast<uint32_t>(sum_counts[0]);
    details.sum_ROI_counts = static_cast<uint32_t>(sum_ROI_counts[0]);
    details.N_threshold_pixels = N_threshold_pixels;
    details.N_ROI_pixels = N_ROI_pixels;
    cv::Rect ROI2(minx + xCorrection,miny + yCorrection,ROI_width,ROI_height);
    details.irradiance =  AutoTracker.ComputeIrradiance(this->counter, ROI2, base_processing_state.details);
    details.ROI_x = minx + xCorrection;
    details.ROI_y = miny + yCorrection;
    details.ROI_Width = ROI_width;
    details.ROI_Height = ROI_height;
    int current_frame_num = starting_frame_number + counter;
    if (track_details_min_frame == 0 || current_frame_num < track_details_min_frame)
    {
        track_details_min_frame = current_frame_num;
    }
    track_details_max_frame = std::max(current_frame_num, track_details_max_frame);
    UpdateCreateTrackLabel();

    track_details[current_frame_num - 1] = details;

    if (chk_auto_advance_frame->isChecked())
    {
        emit advanceFrame(this->txt_frame_advance_amt->text().toInt());
    }
    else
    {
        UpdateDisplayFrame();
    }
}

void VideoDisplay::HandleClearTrackCentroidClick()
{
    int current_frame_num = starting_frame_number + counter;
    track_details[current_frame_num - 1] = std::nullopt;

    if (chk_auto_advance_frame->isChecked())
    {
        emit advanceFrame(this->txt_frame_advance_amt->text().toInt());
    }
    else
    {
        UpdateDisplayFrame();
    }
    ResetCreateTrackMinAndMaxFrames();
    UpdateCreateTrackLabel();
}

void VideoDisplay::HandleFrameAdvanceAmtEntry(const QString &text)
{
    bool is_valid_int = false;
    int value = text.toInt(&is_valid_int);

    if (is_valid_int)
    {
        if (value > frame_advance_limit)
        {
            QString message = QString("Frame advance amount must be less than or equal to the limit of %1.").arg(frame_advance_limit);
            QMessageBox::information(0, "Error", message);
        }

        if (value <= frame_advance_limit && value > osm_track_frames.size()-2)
        {
            // Here we specify 'minus two', since 'minus one' would cycle through to the same point in the frame sequence after each "advance!"
            QMessageBox::information(0, "Error", "Frame advance amount must be less the the total number of frames in the loaded video clip minus two.");
        }
    }
    else
    {
        QMessageBox::information(0, "error", "Please enter a valid integer.");
    }
}

void VideoDisplay::ResetCreateTrackMinAndMaxFrames()
{
    track_details_min_frame = 0;
    track_details_max_frame = 0;

    for (int i = 0; i < track_details.size(); i++)
    {
        if (track_details[i].has_value())
        {
            if (track_details_min_frame == 0){
                track_details_min_frame = i + 1;
            }
            track_details_max_frame = i + 1;
        }
    }
    UpdateCreateTrackLabel();
}

void VideoDisplay::UpdateCreateTrackLabel()
{
    if (track_details_min_frame == 0)
    {
        lbl_create_track->setText("Currently editing a track.  The track has no frames.");
    }
    else
    {
        lbl_create_track->setText("Currently editing a track.  The track spans from frame " + QString::number(track_details_min_frame) + " to frame " + QString::number(track_details_max_frame) + ".");
    }
}

void VideoDisplay::PinpointPixel(unsigned int x, unsigned int y)
{
    unsigned int pinpoint_idx = y * image_x + x;

    //Disallow clicking an already-pinpointed pixel
    if ( std::find(pinpoint_indices.begin(), pinpoint_indices.end(), pinpoint_idx) != pinpoint_indices.end())
    {
        return;
    }

    pinpoint_indices.push_back(pinpoint_idx);

    if (pinpoint_indices.size() > 3)
    {
        pinpoint_indices.erase(pinpoint_indices.begin());
    }
    // Should be able to just update_partial_frame here or something
    UpdateDisplayFrame();
}

void VideoDisplay::RemovePinpointsFromBadPixelMap()
{
    if (pinpoint_indices.size() > 0)
    {
        emit removeBadPixels(pinpoint_indices);
    }
}

void VideoDisplay::AddPinpointsToBadPixelMap()
{
    if (pinpoint_indices.size() > 0)
    {
        emit addNewBadPixels(pinpoint_indices);
    }
}

void VideoDisplay::ClearPinpoints()
{
    pinpoint_indices.clear();
    UpdateDisplayFrame();
}

void VideoDisplay::UpdateFrameVector(std::vector<double> original, std::vector<uint8_t> converted,std::vector<std::vector<int>> offsets0)
{
    original_frame_vector = original;
    display_ready_converted_values = converted;
    xCorrection = 0;
    yCorrection = 0;
    offsets = offsets0;
    UpdateDisplayFrame();
}

void VideoDisplay::UpdateDisplayFrame()
{
    //Prevent attempts to render until the video display has been fully initialized
    if (number_pixels == 0)
    {
        return;
    }

    QRgb rgb_cyan = QColorConstants::Cyan.rgb();

    uint8_t* color_corrected_frame = display_ready_converted_values.data();
    frame = QImage((uchar*)color_corrected_frame, image_x, image_y, QImage::Format_Grayscale8);

    // Convert image to format_indexed. allows color table to take effect on image
    frame = frame.convertToFormat(QImage::Format_Indexed8);
    frame.setColorTable(colorTable);

    // Convert image back to RGB to facilitate use of the colors
    frame = frame.convertToFormat(QImage::Format_RGB888);

    arma::mat offset_matrix(1,3,arma::fill::zeros);
    if ( offsets.size() > 0 ){
        for (int rowi = 0; rowi< offsets.size(); rowi++){
            offset_matrix.insert_rows(offset_matrix.n_rows,arma::conv_to<arma::rowvec>::from(offsets[rowi]));
        }
        offset_matrix.shed_row(0);
        arma::uvec ind = arma::find(offset_matrix.col(0) - 1 == counter);
        if (ind.n_elem>0){
            int ri = ind(0);
            xCorrection = offsets[ri][1];
            yCorrection = offsets[ri][2];
        }
    }

    if (should_show_bad_pixels && current_idx!=-1)
    {
        for (auto i = 0; i < container.processing_states[current_idx].replaced_pixels.size(); i++)
        {
            unsigned int pixel_index = container.processing_states[current_idx].replaced_pixels[i];
            int pixel_x = pixel_index % image_x;
            int pixel_y = pixel_index / image_x;
            int new_pixel_x = pixel_x - xCorrection;
            int new_pixel_y = pixel_y - yCorrection;
            if (new_pixel_x < 0){
                new_pixel_x = new_pixel_x + image_x;
            }
             if (new_pixel_y < 0){
                new_pixel_y = new_pixel_y + image_y;
            }
            if (new_pixel_x > image_x){
                new_pixel_x = new_pixel_x - image_x;
            }
             if (new_pixel_y > image_y){
                new_pixel_y = new_pixel_y - image_y ;
            }
			frame.setPixelColor(new_pixel_x, new_pixel_y, bad_pixel_color);
		}
	}

    QString pinpoint_text("");
    for (auto idx = 0; idx < pinpoint_indices.size(); idx++)
    {
        int pinpoint_idx = pinpoint_indices[idx];

        if (pinpoint_idx < original_frame_vector.size())
        {
            int irradiance_value = original_frame_vector[pinpoint_idx];
            int pinpoint_x = pinpoint_idx % image_x;
            int pinpoint_y = pinpoint_idx / image_x;
            pinpoint_text += "Pixel: " + QString::number(pinpoint_x + 1) + "," + QString::number(pinpoint_y + 1) + ". Value: " + QString::number(irradiance_value);
            if ( std::find(container.processing_states[current_idx].replaced_pixels.begin(), container.processing_states[current_idx].replaced_pixels.end(), pinpoint_idx) != container.processing_states[current_idx].replaced_pixels.end() )
            {
                pinpoint_text += " * (adjusted, bad pixel)";
            }

            if (btn_pinpoint->isChecked())
            {
                QRgb rgb_red = QColorConstants::Red.rgb();
                frame.setPixel(pinpoint_x, pinpoint_y, rgb_red);
            }
        }
        else
        {
            pinpoint_text += "Clicked outside of valid x/y coordinate range.";
        }

        pinpoint_text += "\n";
    }
    lbl_pinpoint->setText(pinpoint_text);

    if (in_track_creation_mode && btn_select_track_centroid->isChecked())
    {
        QPainter  manual_ROI_painter(&frame);
        manual_ROI_painter.setPen(QPen(rgb_cyan));

        int x = hover_pt.x();
        int y = hover_pt.y();
    
        AbsoluteZoomInfo rectangle = zoom_manager->absolute_zoom_list[zoom_manager->zoom_list.size() - 1];
        int ROI_box_size = txt_ROI_dim->text().toInt();
        double x_scale = rectangle.width/image_x;
        double y_scale = rectangle.height/image_y;

        double absolute_x = rectangle.x + 1.0 * x * x_scale;
        double absolute_y = rectangle.y + 1.0 * y * y_scale;

        unsigned int new_x = std::floor(absolute_x);
        unsigned int new_y = std::floor(absolute_y);

        if (cursor_in_image && rectangle.width/ROI_box_size >=1.5)
        {
            lbl_image_canvas->setCursor(Qt::BlankCursor);
            QPoint top_left(new_x - std::round(ROI_box_size/2.), new_y - std::round(ROI_box_size/2.));
            QPoint bottom_right(new_x + std::round(ROI_box_size/2.), new_y + std::round(ROI_box_size/2.));

            QRect manual_ROI_rectangle(top_left, bottom_right);
            manual_ROI_painter.drawRect(manual_ROI_rectangle);
        }
        if (chk_show_crosshair->isChecked())
        {
            SetupCrosshairsCursor(":icons/crosshair-golden.png");
        }

        if (track_details[starting_frame_number + counter - 1].has_value())
        {
            TrackDetails td0 = track_details[starting_frame_number + counter - 1].value();
            int new_x_in_progress_track = std::round(1.0*(td0.centroid_x - xCorrection));
            int new_y_in_progress_track = std::round(1.0*(td0.centroid_y - yCorrection));
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

    for (int i = 0; i < zoom_manager->zoom_list.size(); i++)
    {
        QRect sub_frame = zoom_manager->zoom_list[i];

        // get sub-image
        frame = frame.copy(sub_frame);

        // scale to initial aspect ratio
        frame = frame.scaled(image_x, image_y);
    }

    if (zoom_manager->is_currently_zoomed())
    {
        lbl_image_canvas->setStyleSheet("#video_object { border: 3px solid blue; }");
    }
    else
    {
        lbl_image_canvas->setStyleSheet("#video_object { border: 1px solid light gray; }");
    }
    AbsoluteZoomInfo final_zoom_level2 = zoom_manager->absolute_zoom_list[zoom_manager->absolute_zoom_list.size() - 1];
    double x_scale2 = image_x / final_zoom_level2.width;
    double y_scale2 = image_y / final_zoom_level2.height;
    double size_of_pixel_x2 = 1.0 * x_scale2;
    double size_of_pixel_y2 = 1.0 * y_scale2;
    int marker_size = 5;
    double marker_width = size_of_pixel_x2 - 1 + marker_size * 2;
    double marker_height = size_of_pixel_y2 - 1 + marker_size * 2;
    if (in_track_creation_mode)
    {
        if (track_details[starting_frame_number + counter - 1].has_value())
        {
            QPainter track_creation_marker_painter(&frame);
            track_creation_marker_painter.setPen(QPen(rgb_cyan));

            TrackDetails td = track_details[starting_frame_number + counter - 1].value();
            int track_x = td.centroid_x;
            int track_y = td.centroid_y;
            int new_track_x = track_x - xCorrection;
            int new_track_y = track_y - yCorrection;
            if (new_track_x < 0){
                new_track_x = new_track_x + image_x;
            }
            if (new_track_y < 0){
                new_track_y = new_track_y + image_y;
            }
            if (new_track_x > image_x){
                new_track_x = new_track_x - image_x;
            }
            if (new_track_y > image_y){
                new_track_y = new_track_y - image_y ;
            }
            QRectF track_creation_marker = GetRectangleAroundPixel(new_track_x, new_track_y, marker_size, marker_width, marker_height);
            track_creation_marker_painter.drawRect(track_creation_marker);
        }
    }

    size_t num_osm_tracks = osm_track_frames[counter].tracks.size();
    if (plot_tracks && num_osm_tracks > 0)
    {
        QPainter osm_track_marker_painter(&frame);
        osm_track_marker_painter.setPen(QPen(OSM_track_color));

        for ( const auto &trackData : osm_track_frames[counter].tracks )
        {
            //The OSM tracks are stored offset from the center instead of the top left
            int x_center = image_x / 2 + trackData.second.centroid_x;
            int y_center = image_y / 2 + trackData.second.centroid_y;
            int new_x_center = x_center - xCorrection;
            int new_y_center = y_center - yCorrection;
            if (new_x_center < 0){
                new_x_center = new_x_center + image_x;
            }
             if (new_y_center < 0){
                new_y_center = new_y_center + image_y;
            }
            if (new_x_center > image_x){
                new_x_center = new_x_center - image_x;
            }
             if (new_y_center > image_y){
                new_y_center = new_y_center - image_y;
            }
            QRectF osm_track_marker = GetRectangleAroundPixel(new_x_center-1, new_y_center-1, marker_size, marker_width, marker_height);
            if (osm_track_marker.isNull())
                continue;
            osm_track_marker_painter.drawRect(osm_track_marker);
        }
    }

    size_t num_manual_tracks = manual_track_frames[counter].tracks.size();
    if (num_manual_tracks > 0 && manual_track_ids_to_show.size() > 0)
    {
        QPainter manual_track_marker_painter(&frame);
        manual_track_marker_painter.setPen(QPen(rgb_cyan));

        for ( const auto &trackData : manual_track_frames[counter].tracks )
        {
            int track_id = trackData.first;
            if (manual_track_ids_to_show.find(track_id) != manual_track_ids_to_show.end())
            {
                QColor color = manual_track_colors[track_id];
                int track_x = trackData.second.centroid_x;
                int track_y = trackData.second.centroid_y;
                int new_track_x = track_x - xCorrection;
                int new_track_y = track_y - yCorrection;
                if (new_track_x < 0){
                    new_track_x = new_track_x + image_x;
                }
                if (new_track_y < 0){
                    new_track_y = new_track_y + image_y;
                }
                if (new_track_x > image_x){
                    new_track_x = new_track_x - image_x;
                }
                if (new_track_y > image_y){
                    new_track_y = new_track_y - image_y ;
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

        QString boresight_txt = "Sensor Boresight \n Az: " + QString::number(display_data[counter].azimuth_sensor) + "\n El " + QString::number(display_data[counter].elevation_sensor);

        p2.drawText(frame.rect(), Qt::AlignBottom | Qt::AlignLeft, boresight_txt);
    }

    lbl_frame_number->setText("Frame # " + QString::number(starting_frame_number + counter));

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

        int* r1 = new int;
        int* r2 = new int;
        int* c1 = new int;
        int* c2 = new int;

        // get the coordinates of calculation region
        calculation_region.getCoords(c1, r1, c2, r2);
        std::vector<int> pt1 = zoom_manager->GetPositionWithinZoom(*c1, *r1);
        std::vector<int> pt2 = zoom_manager->GetPositionWithinZoom(*c2, *r2);

        // determine if calculation region is within zoomed image
        bool region_within_zoom = pt1[0] >= 0 && pt2[0] >= 0;
        if (rectangle_drawn && region_within_zoom) {

            // get frame data from original data set and convert mat
            std::vector<double> calibrate_original_frame_vector(container.processing_states[0].details.frames_16bit[counter].begin(), container.processing_states[0].details.frames_16bit[counter].end());
            arma::vec original_image_vector(calibrate_original_frame_vector);
            arma::mat original_mat_frame(original_image_vector);
            original_mat_frame.reshape(image_x, image_y);
            original_mat_frame = original_mat_frame.t();

            // get counts sub-matrix corresponding to the calculation region
            unsigned int ur1 = (unsigned int)*r1;
            unsigned int uc1 = (unsigned int)*c1;
            unsigned int ur2 = (unsigned int)*r2;
            unsigned int uc2 = (unsigned int)*c2;

            arma::mat counts = original_mat_frame.submat(ur1, uc1, ur2, uc2);

            // clear all temporary variables
            calibrate_original_frame_vector.clear();
            original_mat_frame.clear();

            double frame_integration_time = frame_headers[counter].header.int_time;
            std::vector<double>measurements = model.MeasureIrradiance(*r1, *c1, *r2, *c2, counts, frame_integration_time);

            // -----------------------------------------------------------------------------------
            // print radiance calculation data onto frame
            QString max_value = QString::number(measurements[0]) + " uW/cm^2-sr";
            QString avg_value = QString::number(measurements[1]) + " uW/cm^2-sr";
            QString sum_value = QString::number(measurements[2]) + " uW/cm^2-sr";

            QString calculation_text = "***** Beta Calculation *****\n";
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

            QPoint top_left(pt1[0], pt1[1]);
            QPoint bottom_right(pt2[0], pt2[1]);
            QRect zoomed_rectange(top_left, bottom_right);

            calculation_area_painter.setPen(pen_calculation_area);
            calculation_area_painter.drawRect(zoomed_rectange);
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
            unsigned int last_frame_annotation = initial_frame_annotation + a.num_frames;

            // check that current frame is within bounds
            if (counter >= initial_frame_annotation && counter < last_frame_annotation) {

                QString annotation_color = a.color;
                int font_size = a.font_size;
                QString annotation_text = a.text;

                std::vector<int> loc = zoom_manager->GetPositionWithinZoom(a.x_pixel, a.y_pixel);
                int new_x = loc[0];
                int new_y = loc[1];

                if (loc[0] >= 0)
                {
                    // write text
                    QPainter p_a(&frame);
                    p_a.setPen(QPen(annotation_color));
                    p_a.setFont(QFont("Times", font_size));
                    p_a.drawText(new_x, new_y, annotation_text);
                }
            }
        }
    }
}

QRectF VideoDisplay::GetRectangleAroundPixel(int x_center, int y_center, int box_size, double box_width, double box_height)
{
    if (!zoom_manager->is_any_piece_within_zoom(x_center, y_center))
        return QRectF();

    std::vector<int> loc = zoom_manager->GetPositionWithinZoom(x_center, y_center);
    int x = loc[0];
    int y = loc[1];
    std::vector<int> loc2 = zoom_manager->GetPositionWithinZoom(x_center + 1, y_center + 1);
    int x2 = loc2[0];
    int y2 = loc2[1];

    // If the "centroid" pixel is not in the box, this helper method returns -1, -1 ...
    // So we position the box based on loc2, the pixel below and to the right
    int top_left_x, top_left_y;
    if (x < 0)
    {
        top_left_x = x2 - box_width - box_size;
        top_left_y = y2 - box_height - box_size;
    }
    else
    {
        top_left_x = x - box_size;
        top_left_y = y - box_size;
    }

    QRectF rectangle(top_left_x, top_left_y, box_width, box_height);
    return rectangle;
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

void VideoDisplay::HighlightBadPixels(bool status)
{
	should_show_bad_pixels = status;
	UpdateDisplayFrame();
}

void VideoDisplay::HighlightBadPixelsColors(QString input_color)
{
	QColor new_color(input_color);
	bad_pixel_color = new_color;
	// QRgb bad_pixel_color = QColorConstants::Red.rgb();
	UpdateDisplayFrame();
}

void VideoDisplay::UpdateFrameData(std::vector<PlottingFrameData> input_data)
{
    display_data = input_data;
    UpdateDisplayFrame();
}

void VideoDisplay::InitializeTrackData(std::vector<TrackFrame> osm_frame_input, std::vector<TrackFrame> manual_frame_input)
{
    osm_track_frames = osm_frame_input;
    manual_track_frames = manual_frame_input;
}

void VideoDisplay::UpdateManualTrackData(std::vector<TrackFrame> track_frame_input)
{
    manual_track_frames = track_frame_input;
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
    manual_track_colors[id] = color;
    UpdateDisplayFrame();
}

void VideoDisplay::InitializeFrameData(unsigned int frame_number, std::vector<PlottingFrameData> input_data, std::vector<ABIR_Frame>& input_frame_header)
{
    starting_frame_number = frame_number;
    display_data = input_data;
    frame_headers = input_frame_header;
}

void VideoDisplay::SetCalibrationModel(CalibrationData input)
{
    model = input;
}

bool VideoDisplay::StartRecording(QString file_name, double fps)
{
    std::string file_string = file_name.toLocal8Bit().constData();

    video.open(file_string, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(image_x, image_y));

    bool video_opened = video.isOpened();

    if (video_opened)
        record_frame = true;

    return video_opened;
}

void VideoDisplay::AddNewFrame(QImage& img, int format)
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
        frame.save(fileName);
    }
}

void VideoDisplay::RemoveFrame()
{
    delete lbl_image_canvas;

    delete lbl_frame_number;
    delete lbl_video_time_midnight;
    delete lbl_zulu_time;

    lbl_image_canvas = new EnhancedLabel(this);
    SetupLabels();

    frame_data.clear();

    image_x = 0;
    image_y = 0;
    number_pixels = image_x * image_y;

    delete zoom_manager;
    zoom_manager = new VideoDisplayZoomManager(0, 0);
    original_frame_vector.clear();
}

void VideoDisplay::ViewFrame(unsigned int frame_number)
{
    counter = frame_number;
}

void VideoDisplay::ShowStencil()
{
    annotation_stencil->show();
}

void VideoDisplay::HideStencil()
{
    annotation_stencil->hide();
    UpdateDisplayFrame();
}

void VideoDisplay::InitializeStencilData(AnnotationInfo data)
{
    annotation_stencil->InitializeData(data);
}

 void VideoDisplay::DisplayManualBox(QPoint pt)
 {
    if (in_track_creation_mode){
        hover_pt = pt;
        UpdateDisplayFrame();
    }
 }
