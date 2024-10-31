#include <QPushButton>
#include <QLegendMarker>
#include <functional>

#include "plot_engineering_data.h"

EngineeringPlots::EngineeringPlots(std::vector<Frame> const &osm_frames) : JKQTPlotter()
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
    //SetPlotTitle("EDIT CLASSIFICATION");

    plot_all_data = true;
    plot_primary_only = false;
    plot_current_marker = false;

    x_axis_units = frames;

    current_unit_id = 0; // 0 -> counts
    current_chart_id = 2; // 2 -> Elevation

    // 'sub-plot' refers to the frame range, not to zoom
    index_sub_plot_xmin = 0;
    index_sub_plot_xmax = num_frames - 1;

    //connect(this, &EngineeringPlots::changeMotionStatus, this->chart_view, &NewChartView::UpdateChartFramelineStatus);
}

EngineeringPlots::~EngineeringPlots()
{
}

void EngineeringPlots::InitializeIntervals(const std::vector<Frame> &osm_frames)
{
    x_axis_units = frames;
    QPair<qreal,qreal> frame_interval = *new QPair<qreal,qreal>();
    frame_interval.first = get_single_x_axis_value(0);
    frame_interval.second = get_max_x_axis_value();

    x_axis_units = seconds_past_midnight;
    QPair<qreal,qreal> midnight_interval = *new QPair<qreal, qreal>();
    midnight_interval.first = get_single_x_axis_value(0);
    midnight_interval.second = get_max_x_axis_value();

    x_axis_units = seconds_from_epoch;
    QPair<qreal,qreal> epoch_interval = *new QPair<qreal, qreal>();
    epoch_interval.first = get_single_x_axis_value(0);
    epoch_interval.second = get_max_x_axis_value();

    chart_x_intervals[0] = frame_interval;
    chart_x_intervals[1] = midnight_interval;
    chart_x_intervals[2] = epoch_interval;
}

void EngineeringPlots::SetXAxisChartId(int xaxis_chart_id)
{

    current_unit_id = xaxis_chart_id;
}

void EngineeringPlots::SetYAxisChartId(int yaxis_chart_id)
{
    current_chart_id = yaxis_chart_id;
}

void EngineeringPlots::PlotChart(bool yAxisChangedLocal)
{
    int plot_number_tracks = 1;

    auto func = std::bind(&EngineeringPlots::get_individual_y_track_irradiance, this, std::placeholders::_1);

    PlotSirveQuantity(func, plot_number_tracks, QString("ROI Counts"));

    func = std::bind(&EngineeringPlots::get_individual_y_track_azimuth, this, std::placeholders::_1);

    PlotSirveQuantity(func, plot_number_tracks, QString("Azimuth"));

    func = std::bind(&EngineeringPlots::get_individual_y_track_elevation, this, std::placeholders::_1);

    PlotSirveQuantity(func, plot_number_tracks, QString("Elevation"));
}

void EngineeringPlots::PlotSirveQuantity(std::function<std::vector<double>(size_t)> get_y_track_func, size_t plot_number_tracks, QString title)
{
    std::vector<double> y_points;

    for (size_t track_index = 0; track_index < plot_number_tracks; track_index++)
    {
        JKQTPDatastore* ds= this->getDatastore();

        std::vector<double> x_values = get_individual_x_track(track_index);
        std::vector<double> y_values = get_y_track_func(track_index);

        QVector<double> X(x_values.begin(), x_values.end());
        QVector<double> Y(y_values.begin(), y_values.end());

        // 3. make data available to JKQTPlotter by adding it to the internal datastore.
        size_t columnX=ds->addCopiedColumn(X, "x");
        size_t columnY=ds->addCopiedColumn(Y, "y");

        // 4. create a graph in the plot, which plots the dataset X/Y:
        JKQTPXYLineGraph* graph1=new JKQTPXYLineGraph(this);
        graph1->setXColumn(columnX);
        graph1->setYColumn(columnY);
        graph1->setTitle(title);

        graph1->setSymbolSize(5);
        graph1->setSymbolLineWidth(1);
        graph1->setColor(colors.get_current_color());
        graph1->setSymbolColor(QColor::fromRgb(255,20,20));

        // 5. add the graph to the plot, so it is actually displayed
        this->addGraph(graph1);

        // 6. autoscale the plot so the graph is contained
        this->zoomToFit();
    }
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

    //chart->addSeries(current_frame_marker);
}

void EngineeringPlots::toggle_yaxis_log(bool input)
{
    // yaxis_is_log = input;
}

void EngineeringPlots::toggle_yaxis_scientific(bool input)
{
    // yaxis_is_scientific = input;
}

void EngineeringPlots::toggle_xaxis_fixed_pt(bool input)
{
    // xaxis_is_fixed_pt = input;
}

void EngineeringPlots::PlotCurrentStep(int counter)
{
    // if (plot_current_marker)
    // {
    //     double current_x = get_single_x_axis_value(index_sub_plot_xmin + counter);
    //     double min_y, max_y;

    //     if (yaxis_is_log)
    //     {
    //         min_y = 0.000000001;
    //         max_y = axis_ylog->max();
    //     }
    //     else
    //     {
    //         min_y = axis_y->min();
    //         max_y = axis_y->max();
    //     }

    //     current_frame_marker->clear();
    //     current_frame_marker->append(current_x, min_y);
    //     current_frame_marker->append(current_x, max_y);
    // }
}

void EngineeringPlots::SetPlotTitle(QString input_title)
{

    // title = input_title;
    // DrawTitle();

}

void EngineeringPlots::DrawTitle()
{
    // QColor brush_color("black");
    // QBrush brush(brush_color);

    // QFont font("Times New Roman", 12, QFont::Bold);

    // chart->setTitleBrush(brush);
    // chart->setTitleFont(font);

    // chart->setTitle(title);
}

void EngineeringPlots::ToggleSubplot()
{
    // if (plot_all_data)
    // {
    //     set_xaxis_limits(full_plot_xmin, full_plot_xmax);
    // }
    // else
    // {
    //     set_xaxis_limits(sub_plot_xmin, sub_plot_xmax);
    // }
}

void EngineeringPlots::UpdateManualPlottingTrackFrames(std::vector<ManualPlottingTrackFrame> frames, std::set<int> track_ids)
{
    // manual_track_frames = frames;
    // manual_track_ids = track_ids;

    // QColor starting_color = ColorScheme::get_track_colors()[0];
    // for (auto track_id : track_ids)
    // {
    //     if (manual_track_colors.find(track_id) == manual_track_colors.end())
    //     {
    //         manual_track_colors[track_id] = starting_color;
    //     }
    // }
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
    //osm_track_color = color;
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
