#include "plot_engineering_data.h"
#include "qrubberband.h"

#include <QPushButton>
#include <QLegendMarker>

// NOTE: SirveApp's instance of this class, Engineering_Plots, does not yet control its own graphical updates like VideoDisplay

EngineeringPlots::EngineeringPlots(std::vector<Frame> const &osm_frames) : QtPlotting()
{
    num_frames = static_cast<unsigned int>(osm_frames.size());

    for (size_t i = 0; i < num_frames; i++)
    {
        /* Scaling each val by 1e6 to convert to microradians. */
        sensor_i_fov_x.push_back(osm_frames[i].data.i_fov_x * 1e6);
        sensor_i_fov_y.push_back(osm_frames[i].data.i_fov_y * 1e6);

        boresight_az.push_back(osm_frames[i].data.az_el_boresight[0]);
        boresight_el.push_back(osm_frames[i].data.az_el_boresight[1]);
    }

    x_axis_units = frames;
    plot_all_data = true;
    plot_primary_only = false;
    plot_current_marker = false;
    yaxis_is_log = false;
    yaxis_is_scientific = false;
    SetPlotTitle("EDIT CLASSIFICATION");
    current_chart_id = 0;

    index_sub_plot_xmin = 0;
    index_sub_plot_xmax = num_frames - 1;

    osm_track_color = colors.get_current_color();
    // osm_track_color = colors.get_color(7);

    connect(this, &EngineeringPlots::changeMotionStatus, this->chart_view, &NewChartView::UpdateChartFramelineStatus);
}

EngineeringPlots::~EngineeringPlots()
{
}

void EngineeringPlots::SetYAxisChartId(int yaxis_chart_id)
{
    // If the chart type has changed,
    if (current_chart_id != yaxis_chart_id)
    {
        // Get the chart state object for updating
        ChartState chartState = this->chart_view->get_chart_state();

        // Record the state of the chart we are leaving behind:
        if (yaxis_is_log) {
            chartState.scale_factor_max = axis_ylog->max() / chart_y_maxes[current_chart_id];
            chartState.scale_factor_min = axis_ylog->min() / chart_y_maxes[current_chart_id];
        } else
        {
            QValueAxis *axisY = qobject_cast<QValueAxis*>(this->chart_view->chart()->axisY());
            chartState.scale_factor_max = axisY->max() / chart_y_maxes[current_chart_id];
            chartState.scale_factor_min = axisY->min() / chart_y_maxes[current_chart_id];
        }

        this->chart_view->set_chart_state(chartState);
    }

    current_chart_id = yaxis_chart_id;
}

void EngineeringPlots::PlotChart()
{
    // Clear chart
    chart->removeAllSeries();
    colors.ResetColors();
    StartNewChart();
    CreateCurrentMarker();
    EstablishPlotLimits();

    size_t plot_number_tracks = number_of_tracks;
    if (plot_primary_only && plot_number_tracks > 0)
        plot_number_tracks = 1;

    switch (current_chart_id)
    {
    case 0:
        y_title = QString("ROI Counts");
        PlotIrradiance(plot_number_tracks);
        break;
    case 1:
        y_title = QString("Azimuth (deg)");
        PlotAzimuth(plot_number_tracks);
        break;
    case 2:
        y_title = QString("Elevation (deg)");
        PlotElevation(plot_number_tracks);
        break;
    case 3:
        y_title = QString("Sensor IFOV (microns)");
        PlotFovX();
        break;
    case 4:
        y_title = QString("Sensor IFOV (microns)");
        PlotFovY();
        break;
    case 5:
        y_title = QString("Boresight Azimuth");
        PlotBoresightAzimuth();
        break;
    case 6:
        y_title = QString("Boresight Elevation");
        PlotBoresightElevation();
    default:
        break;
    }

    DrawTitle();

    if (this->chart_view->is_zoomed)
    {
        // We're on a new chart. Apply the target ranges to the axes:
        ChartState chartState = this->chart_view->get_chart_state();

        QValueAxis *axisX = qobject_cast<QValueAxis*>(this->chart_view->chart()->axisX());
        QValueAxis *axisY = qobject_cast<QValueAxis*>(this->chart_view->chart()->axisY());

        if (axisX) {
            axisX->setRange(chartState.xMin, chartState.xMax);
        }

        if (axisY) {
            axisY->setRange(chart_y_maxes[current_chart_id] * chartState.scale_factor_min, chart_y_maxes[current_chart_id] * chartState.scale_factor_max);
        }
    }
}

void EngineeringPlots::PlotBoresightAzimuth()
{
    QLineSeries* series = new QLineSeries();
    series->setColor(osm_track_color);
    series->setName("OSM Data");

    if (plot_all_data)
    {
        std::vector<double> y_values = boresight_az;

        AddSeries(series, get_x_axis_values(0, num_frames - 1), y_values, true);
        DefineChartProperties(full_plot_xmin, full_plot_xmax, 0, 360);
    }
    else
    {
        std::vector<double> y_values(boresight_az.begin() + index_sub_plot_xmin, boresight_az.begin() + index_sub_plot_xmax + 1);

        AddSeries(series, get_x_axis_values(index_sub_plot_xmin, index_sub_plot_xmax), y_values, true);
        DefineChartProperties(sub_plot_xmin, sub_plot_xmax, 0, 360);
    }
}

void EngineeringPlots::PlotBoresightElevation()
{
    QLineSeries* series = new QLineSeries();
    series->setColor(osm_track_color);
    series->setName("OSM Data");

    if (plot_all_data)
    {
        std::vector<double> y_values = boresight_el;

        AddSeries(series, get_x_axis_values(0, num_frames - 1), y_values, true);
        DefineChartProperties(full_plot_xmin, full_plot_xmax, 0, 90);
    }
    else
    {
        std::vector<double> y_values(boresight_el.begin() + index_sub_plot_xmin, boresight_el.begin() + index_sub_plot_xmax + 1);

        AddSeries(series, get_x_axis_values(index_sub_plot_xmin, index_sub_plot_xmax), y_values, true);
        DefineChartProperties(sub_plot_xmin, sub_plot_xmax, 0, 90);
    }
}

void EngineeringPlots::PlotFovX()
{
    QLineSeries* series = new QLineSeries();
    series->setColor(osm_track_color);
    series->setName("OSM Data");

    if (plot_all_data)
    {
        std::vector<double> y_values = sensor_i_fov_x;

        AddSeries(series, get_x_axis_values(0, num_frames - 1), y_values, true);
        DefineChartProperties(full_plot_xmin, full_plot_xmax, 0, 750);
    }
    else
    {
        std::vector<double> y_values(sensor_i_fov_x.begin() + index_sub_plot_xmin, sensor_i_fov_x.begin() + index_sub_plot_xmax + 1);

        AddSeries(series, get_x_axis_values(index_sub_plot_xmin, index_sub_plot_xmax), y_values, true);
        DefineChartProperties(sub_plot_xmin, sub_plot_xmax, 0, 750);
    }
}

void EngineeringPlots::PlotFovY()
{
    QLineSeries* series = new QLineSeries();
    series->setColor(osm_track_color);
    series->setName("OSM Data");

    if (plot_all_data)
    {
        std::vector<double> y_values = sensor_i_fov_y;

        AddSeries(series, get_x_axis_values(0, num_frames - 1), y_values, true);
        DefineChartProperties(full_plot_xmin, full_plot_xmax, 0, 750);
    }
    else
    {
        std::vector<double> y_values(sensor_i_fov_y.begin() + index_sub_plot_xmin, sensor_i_fov_y.begin() + index_sub_plot_xmax + 1);

        AddSeries(series, get_x_axis_values(index_sub_plot_xmin, index_sub_plot_xmax), y_values, true);
        DefineChartProperties(sub_plot_xmin, sub_plot_xmax, 0, 750);
    }
}

void EngineeringPlots::PlotAzimuth(size_t plot_number_tracks)
{
    for (size_t i = 0; i < plot_number_tracks; i++)
    {
        QLineSeries* series = new QLineSeries();
        series->setColor(osm_track_color);
        series->setName("OSM Data");

        std::vector<double> x_values = get_individual_x_track(i);
        std::vector<double> y_values = get_individual_y_track_azimuth(i);

        AddSeries(series, x_values, y_values, true);
    }

    for (int track_id : manual_track_ids)
    {
        std::vector<double> x_values, y_values;

        for (size_t i = 0; i < manual_track_frames.size(); i++)
        {
            std::map<int, ManualPlottingTrackDetails>::iterator it = manual_track_frames[i].tracks.find(track_id);
            if (it != manual_track_frames[i].tracks.end())
            {
                x_values.push_back(get_single_x_axis_value(i));
                y_values.push_back(it->second.azimuth);
            }
        }

        AddSeriesWithColor(x_values, y_values, manual_track_colors[track_id]);
    }

    if (plot_all_data)
        DefineChartProperties(full_plot_xmin, full_plot_xmax, 0, 360);
    else

    DefineChartProperties(sub_plot_xmin, sub_plot_xmax, 0, 360);
}
void EngineeringPlots::PlotElevation(size_t plot_number_tracks)
{
    for (size_t i = 0; i < plot_number_tracks; i++)
    {
        QLineSeries *series = new QLineSeries();
        // series->setColor(osm_track_color);
        series->setName("OSM Data");

        std::vector<double> x_values = get_individual_x_track(i);
        std::vector<double> y_values = get_individual_y_track_elevation(i);

        AddSeries(series, x_values, y_values, true);
        series->setColor(osm_track_color);
    }

    for (int track_id : manual_track_ids)
    {
        std::vector<double> x_values, y_values;

        for (size_t i = 0; i < manual_track_frames.size(); i++)
        {
            std::map<int, ManualPlottingTrackDetails>::iterator it = manual_track_frames[i].tracks.find(track_id);
            if (it != manual_track_frames[i].tracks.end())
            {
                x_values.push_back(get_single_x_axis_value(i));
                y_values.push_back(it->second.elevation);
            }
        }

        AddSeriesWithColor(x_values, y_values, manual_track_colors[track_id]);
    }

    if (plot_all_data)
        DefineChartProperties(full_plot_xmin, full_plot_xmax, 0, 90);
    else
        DefineChartProperties(sub_plot_xmin, sub_plot_xmax, 0, 90);
}

void EngineeringPlots::PlotIrradiance(size_t plot_number_tracks)
{
    std::vector<double> y_points;

    for (size_t i = 0; i < plot_number_tracks; i++)
    {
        QLineSeries *series = new QLineSeries();
        series->setColor(osm_track_color);
        series->setName("OSM Data");

        std::vector<double> x_values = get_individual_x_track(i);
        std::vector<double> y_values = get_individual_y_track_irradiance(i);

        AddSeries(series, x_values, y_values, true);;

        y_points.insert(y_points.end(), y_values.begin(), y_values.end());
    }

    chart_y_maxes[0] = FindMaxForAxis(y_points);
    fixed_max_y = chart_y_maxes[0];

    if (plot_all_data)
        DefineChartProperties(full_plot_xmin, full_plot_xmax, 0, FindMaxForAxis(y_points));
    else
        DefineChartProperties(sub_plot_xmin, sub_plot_xmax, 0, FindMaxForAxis(y_points));
}

void EngineeringPlots::set_plotting_track_frames(std::vector<PlottingTrackFrame> frames, int num_unique)
{
    track_frames = frames;
    number_of_tracks = num_unique;
}

std::vector<double> EngineeringPlots::get_individual_x_track(size_t i)
{
    std::vector<double> x_values;

    for (int track_frame_index = 0; track_frame_index < track_frames.size(); track_frame_index += 1)
    {
        if (i < track_frames[track_frame_index].details.size())
        {
            x_values.push_back(get_single_x_axis_value(track_frame_index));
        }
    }

    return x_values;
}

std::vector<double> EngineeringPlots::get_individual_y_track_irradiance(size_t i)
{
    std::vector<double> y_values;
    for (int track_frame_index = 0; track_frame_index < track_frames.size(); track_frame_index += 1)
    {
        if (i < track_frames[track_frame_index].details.size())
        {
            y_values.push_back(track_frames[track_frame_index].details[i].irradiance);
        }
    }

    return y_values;
}

std::vector<double> EngineeringPlots::get_individual_y_track_azimuth(size_t i)
{
    std::vector<double> y_values;
    for (int track_frame_index = 0; track_frame_index < track_frames.size(); track_frame_index += 1)
    {
        if (i < track_frames[track_frame_index].details.size())
        {
            y_values.push_back(track_frames[track_frame_index].details[i].azimuth);
        }
    }

    return y_values;
}

std::vector<double> EngineeringPlots::get_individual_y_track_elevation(size_t i)
{
    std::vector<double> y_values;
    for (int track_frame_index = 0; track_frame_index < track_frames.size(); track_frame_index += 1)
    {
        if (i < track_frames[track_frame_index].details.size())
        {
            y_values.push_back(track_frames[track_frame_index].details[i].elevation);
        }
    }

    return y_values;
}

void EngineeringPlots::EstablishPlotLimits()
{

    sub_plot_xmin = get_single_x_axis_value(index_sub_plot_xmin);
    sub_plot_xmax = get_single_x_axis_value(index_sub_plot_xmax);

    full_plot_xmin = get_single_x_axis_value(0);
    full_plot_xmax = get_max_x_axis_value();
}

void EngineeringPlots::set_xaxis_units(XAxisPlotVariables unit_choice)
{
    x_axis_units = unit_choice;
    switch (x_axis_units)
    {
    case frames:
        x_title = "Frame #";
        break;
    case seconds_past_midnight:
        x_title = "Seconds Past Midnight";
        break;
    case seconds_from_epoch:
        x_title = "Seconds Past Epoch";
        break;
    default:
        break;
    }
}

std::vector<double> EngineeringPlots::get_x_axis_values(unsigned int start_idx, unsigned int end_idx)
{
    switch (x_axis_units)
    {
    case frames:
    {
        std::vector<double> x_values(end_idx - start_idx + 1);
        std::iota(std::begin(x_values), std::end(x_values), start_idx + 1);
        return x_values;
    }
    case seconds_past_midnight:
        return std::vector<double>(past_midnight.begin() + start_idx, past_midnight.begin() + end_idx + 1);
    case seconds_from_epoch:
        return std::vector<double>(past_epoch.begin() + start_idx, past_epoch.begin() + end_idx + 1);
    default:
        return std::vector<double>();
    }
}

double EngineeringPlots::get_single_x_axis_value(int x_index)
{
    switch (x_axis_units)
    {
    case frames:
        return x_index + 1;
    case seconds_past_midnight:
        return past_midnight[x_index];
    case seconds_from_epoch:
        return past_epoch[x_index];
    default:
        return 0;
    }
}

double EngineeringPlots::get_max_x_axis_value()
{
    switch (x_axis_units)
    {
    case frames:
        return num_frames;
    case seconds_past_midnight:
        return past_midnight[past_midnight.size() - 1];
    case seconds_from_epoch:
        return past_epoch[past_epoch.size() - 1];
    default:
        return 0;
    }
}

void EngineeringPlots::CreateCurrentMarker()
{
    current_frame_marker = new QLineSeries();
    current_frame_marker->setName("Red Line");

    QPen pen;
    pen.setColor(colors.get_color(2));
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(3);

    current_frame_marker->setPen(pen);

    current_frame_marker->append(0, 0);
    current_frame_marker->append(0, 0);

    chart->addSeries(current_frame_marker);
}

void EngineeringPlots::toggle_yaxis_log(bool input)
{
    yaxis_is_log = input;
}

void EngineeringPlots::toggle_yaxis_scientific(bool input)
{
    yaxis_is_scientific = input;
}

void EngineeringPlots::toggle_xaxis_fixed_pt(bool input)
{
    xaxis_is_fixed_pt = input;
}

void EngineeringPlots::PlotCurrentStep(int counter)
{
    if (plot_current_marker)
    {
        double current_x = get_single_x_axis_value(index_sub_plot_xmin + counter);
        double min_y, max_y;

        if (yaxis_is_log)
        {
            min_y = 0.000000001;
            max_y = axis_ylog->max();
        }
        else
        {
            min_y = axis_y->min();
            max_y = axis_y->max();
        }

        current_frame_marker->clear();
        current_frame_marker->append(current_x, min_y);
        current_frame_marker->append(current_x, max_y);
    }
}

void EngineeringPlots::SetPlotTitle(QString input_title)
{

    title = input_title;
    DrawTitle();

}

void EngineeringPlots::DrawTitle()
{
    QColor brush_color("black");
    QBrush brush(brush_color);

    QFont font("Times New Roman", 12, QFont::Bold);
    // font.setPointSize(10);
    // font.setBold(true);

    chart->setTitleBrush(brush);
    chart->setTitleFont(font);

    chart->setTitle(title);
}

void EngineeringPlots::ToggleSubplot()
{
    if (plot_all_data)
    {
        set_xaxis_limits(full_plot_xmin, full_plot_xmax);
    }
    else
    {
        set_xaxis_limits(sub_plot_xmin, sub_plot_xmax);
    }
}

void EngineeringPlots::UpdateManualPlottingTrackFrames(std::vector<ManualPlottingTrackFrame> frames, std::set<int> track_ids)
{
    manual_track_frames = frames;
    manual_track_ids = track_ids;

    QColor starting_color = ColorScheme::get_track_colors()[0];
    for (auto track_id : track_ids)
    {
        if (manual_track_colors.find(track_id) == manual_track_colors.end())
        {
            manual_track_colors[track_id] = starting_color;
        }
    }
    //There is a bug here where deleted manual tracks will never be removed from `manual_track_colors`
    //But this has minimal impact and isn't worth the code required to solve it
}

void EngineeringPlots::RecolorManualTrack(int track_id, QColor new_color)
{
    manual_track_colors[track_id] = new_color;
}

void EngineeringPlots::RecolorOsmTrack(QColor color)
{
    // osm_track_color = new_color_str == "white" || new_color_str == "blue" ? colors.get_current_color() : QColor(new_color_str);
    osm_track_color = color;
    emit updatePlots();
}
void EngineeringPlots::HandlePlayerButtonClick()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString player_button_id = button->property("id").toString();

        if (player_button_id == "pause" ||
            player_button_id == "previous" ||
            player_button_id == "next")
        {
            emit changeMotionStatus(false);
        }
        else
        {
            emit changeMotionStatus(true);
        }
    }
}

// Generic plotting functions
NewChartView::NewChartView(QChart* chart)
    :QChartView(chart), rubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{
    newchart = chart;

    setMouseTracking(true);
    setInteractive(true);

    is_zoomed = false;
}

void NewChartView::UpdateChartFramelineStatus(bool status)
{
    is_frameline_moving = status;
}

void NewChartView::clearSeriesByName(const QString &seriesName) {
    for (QAbstractSeries *abstractSeries : chart()->series()) {
        if (QLineSeries *lineSeries = qobject_cast<QLineSeries *>(abstractSeries)) {
            if (lineSeries->name() == seriesName) {
                lineSeries->clear();
                return;
            }
        }
    }
}

void NewChartView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        origin = event->pos();
        rubberBand->setGeometry(QRect(origin, QSize()));
        rubberBand->show();
    }
    QChartView::mousePressEvent(event);
}

void NewChartView::mouseMoveEvent(QMouseEvent *event) {
    if (rubberBand->isVisible()) {
        rubberBand->setGeometry(QRect(origin, event->pos()).normalized());
    }
    QChartView::mouseMoveEvent(event);
}

void NewChartView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
    {
        newchart->zoomOut();
        newchart->zoomReset();
        is_zoomed = false;

        emit updatePlots();

        return;
    } else
    {
        if (this->is_frameline_moving) {
            clearSeriesByName("Red Line");
            newchart->update();
        }

        rubberBand->hide();
        QRect selectedRect = rubberBand->geometry();
        emit rubberBandChanged(selectedRect);

        if (!selectedRect.isEmpty()) {
            chart()->zoomIn(selectedRect);

            QValueAxis *axisX = qobject_cast<QValueAxis*>(chart()->axisX());
            QValueAxis *axisY = qobject_cast<QValueAxis*>(chart()->axisY());

            if (axisX) {
                savedChartState.xMin = axisX->min();
                savedChartState.xMax = axisX->max();
            }

            if (axisY) {
                savedChartState.yMin = axisY->min();
                savedChartState.yMax = 1000000000000;
            }
        }
        is_zoomed = true;
        emit updateFrameLine();
    }

    QChartView::mouseReleaseEvent(e);
}

void NewChartView::apply_nice_numbers()
{
    QList<QAbstractAxis*> axes_list = newchart->axes();
    for each (QAbstractAxis * abstract_axis in axes_list)
    {
        QValueAxis* value_axis = qobject_cast<QValueAxis*>(abstract_axis);
        if (value_axis)
        {
            value_axis->applyNiceNumbers();
        }
    }
}

ChartState NewChartView::get_chart_state()
{
    return this->savedChartState;
}

void NewChartView::set_chart_state(ChartState chartState)
{
    this->savedChartState = chartState;
}

// ---------------------------------------------------------------------------------------------

QtPlotting::QtPlotting()
{
    chart = new QChart();
    chart_view = new NewChartView(chart);

    axis_x = new QValueAxis;
    axis_y = new QValueAxis;
    axis_ylog = new QLogValueAxis;
}

QtPlotting::~QtPlotting()
{
    delete chart;
    delete chart_view;
}

void QtPlotting::StartNewChart()
{
    delete axis_x;
    axis_x = new QValueAxis();
    axis_x->setTitleText("x");
    axis_x->setRange(0, 10);

    delete axis_y;
    axis_y = new QValueAxis();
    axis_y->setTitleText("y");
    axis_y->setRange(0, 10);

    delete axis_ylog;
    axis_ylog = new QLogValueAxis();
    axis_ylog->setTitleText("y");
    axis_ylog->setBase(10);
    axis_ylog->setRange(1, 10);


    if (yaxis_is_scientific)
    {
        axis_y->setLabelFormat("%.3e");
        axis_ylog->setLabelFormat("%.3e");
    }
    else
    {
        axis_y->setLabelFormat("%.3f");
        axis_ylog->setLabelFormat("%.3f");
    }

    if (xaxis_is_fixed_pt)
    {
        axis_x->setLabelFormat("%.3f");
    }
    else
    {
        axis_x->setLabelFormat("%d");
    }

    chart->addAxis(axis_x, Qt::AlignBottom);

    if (yaxis_is_log)
    {
        chart->addAxis(axis_ylog, Qt::AlignLeft);
    }
    else {
        chart->addAxis(axis_y, Qt::AlignLeft);
    }
}

void QtPlotting::AddSeriesWithColor(std::vector<double> x, std::vector<double> y, QColor color)
{
    double base_x_distance = 1.5;

    QLineSeries *series = new QLineSeries();

    QPen pen;
    pen.setColor(color);
    series->setPen(pen);

    series->append(x[0], y[0]);
    for (size_t i = 1; i < x.size(); i++)
    {
        //If there was a break, add the old series and start a new one
        if (x[i] - x[i-1] > base_x_distance)
        {
            chart->addSeries(series);

            series = new QLineSeries();
            series->setPen(pen);
        }

        series->append(x[i], y[i]);
        base_x_distance = 1.5 * x[i] - x[i-1];
    }

    chart->addSeries(series);
}

void QtPlotting::AddSeries(QXYSeries *series, std::vector<double> x, std::vector<double> y, bool broken_data)
{
    size_t num_data_pts = x.size();
    int num_breaks = 0;

    if(osm_track_color == QColor(0,0,0,255)){
       osm_track_color = QColor(0,0,0); 
    }
    double base_x_distance = 1;
    if (num_data_pts > 1)
    {
        // distance to separate into different line series is the median difference of the x-value set
        arma::vec x_vector(x);
        arma::vec diff = arma::diff(x_vector);
        base_x_distance = 1.5; //arma::median(diff);
    }

    for (size_t i = 0; i < num_data_pts; i++)
    {
        if (i == 0)
        { //If first pt in series then continue...
            series->append(x[i], y[i]);
            series->setColor(osm_track_color);
        }
        else if ((x[i] - x[i - 1] > base_x_distance) & broken_data)
        {
            //if current point is greater than 1 frame away then start new series...
            chart->addSeries(series);
            series->setColor(osm_track_color);

            if (num_breaks > 0)
                RemoveSeriesLegend();
            num_breaks++;

            series = new QLineSeries();
            series->append(x[i], y[i]);
            series->setColor(osm_track_color);
        }
        else
        {
            //if continuation of current series...
            series->append(x[i], y[i]);
            series->setColor(osm_track_color);
        }

    }

    QPen pen;
    // pen.setColor(osm_track_color);
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(3);
    series->setPen(pen);

    chart->addSeries(series);
    if ((num_breaks > 0) & broken_data)
        RemoveSeriesLegend();
}

double QtPlotting::FindTickSpacing(double data_range, int min_number_ticks, int max_number_ticks)
{
    double min_spacing = data_range / max_number_ticks;
    double magnitude_spacing = std::pow(10, std::floor(std::log10(min_spacing)));
    double residual = min_spacing / magnitude_spacing;

    double tick_spacing;
    if (residual > 5) {
        tick_spacing = magnitude_spacing * 10;
    }
    else if (residual > 2) {
        tick_spacing = magnitude_spacing * 5;
    }
    else if (residual > 1) {
        tick_spacing = magnitude_spacing * 2;
    }
    else {
        tick_spacing = magnitude_spacing * 1;
    }

    double num_ticks = std::floor(data_range / tick_spacing);
    if (num_ticks < min_number_ticks) {
        tick_spacing = tick_spacing / 2;
    }

    return tick_spacing;
}

void QtPlotting::RemoveSeriesLegend()
{
    int num_legend_entries = chart->legend()->markers().size();
    if (num_legend_entries > 1)
    {
        QLegendMarker *last_entry = chart->legend()->markers().last();
        last_entry->setVisible(false);
    }
}

void QtPlotting::DefineChartProperties(double min_x, double max_x, double min_y, double max_y)
{
    axis_x->setRange(min_x, max_x);
    axis_x->setTitleText(x_title);

    if (yaxis_is_log)
    {
        if (min_y <= 0.001)
            min_y = 0.01;
        if (max_y <= 0.001)
            max_y = 0.01;

        axis_ylog->setRange(min_y, max_y);
        axis_ylog->setTitleText(y_title);
    }
    else
    {
        axis_y->setRange(min_y, max_y);
        axis_y->setTitleText(y_title);
    }

    int num_series = chart->series().size();

    bool check1, check2;
    QList<QAbstractSeries *> all_series = chart->series();

    for (int i = 0; i < num_series; i++)
    {
        check1 = all_series[i]->attachAxis(axis_x);

        if (yaxis_is_log)
        {
            check2 = all_series[i]->attachAxis(axis_ylog);
        }
        else
        {
            check2 = all_series[i]->attachAxis(axis_y);
        }
    }

    chart->setMargins(QMargins(0, 0, 0, 0));

    // Legend properties
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignTop);
    chart->legend()->setMarkerShape(QLegend::MarkerShapeDefault);

    // Get all legend markers
    QList<QLegendMarker *> markers = chart->legend()->markers();

    // Hide all but the OSM marker at index 1:
    markers[0]->setVisible(false);

    for (int i = 2; i < markers.size(); i++) {

        markers[i]->setVisible(false);
    }
}

void QtPlotting::set_axis_limits(QAbstractAxis *axis, double min_x, double max_x)
{
    axis->setMin(min_x);
    axis->setMax(max_x);
}

void QtPlotting::set_xaxis_limits(double min_x, double max_x)
{
    chart->axisX()->setRange(min_x, max_x);
}

void QtPlotting::set_yaxis_limits(double min_y, double max_y)
{
    chart->axisY()->setRange(min_y, max_y);
}

double QtPlotting::FindMaxForAxis(std::vector<double> data)
{
    double min, max;

    arma::vec input_data(data);

    if(data.size() == 0)
    {
        min = 0.000001;
        max = 0.00001;
    }
    else
    {
        min = arma::min(input_data);
        max = arma::max(input_data);
    }

    double range_value = max - min;

    double tick_spacing_x = FindTickSpacing(range_value, 3, 10);
    double max_limit_x = std::floor(max / tick_spacing_x) * tick_spacing_x + 0.5 * tick_spacing_x;

    return max_limit_x;
}

void QtPlotting::SavePlot()
{
    QPixmap p = chart_view->grab();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Images (*.png)"));

    if (!fileName.isEmpty())
        p.save(fileName, "PNG");

}
