#include <QPushButton>
#include <QLegendMarker>
#include <functional>
#include "plot_engineering_data.h"
#include "enums.h"
#include "SirveApp.h"

EngineeringPlot::EngineeringPlot(std::vector<Frame> *osm_frames, std::vector<Quantity> quantities) : JKQTPlotter()
{
    osm_frames_ref = osm_frames;

    plot_classification = "EDIT CLASSIFICATION";
    my_quantities = quantities;
    plotYType = Enums::getPlotTypeByIndex(Enums::getPlotTypeIndexFromString(quantities.at(0).getName()));
    plotXType = Enums::getPlotTypeByIndex(Enums::getPlotTypeIndexFromString(quantities.at(1).getName()));
    num_frames = static_cast<unsigned int>(osm_frames->size());

    EngineeringData *engineeringData = new EngineeringData(osm_frames);

    this->past_midnight = engineeringData->get_seconds_from_midnight();
    this->past_epoch = engineeringData->get_seconds_from_epoch();

    plot_all_data = true;
    plot_primary_only = false;
    plot_current_marker = false;

    x_axis_units = plotXType;

    // 'sub-plot' refers to the chart's domain
    index_sub_plot_xmin = 0;
    index_sub_plot_xmax = num_frames - 1;

    ds = this->getDatastore();

    show_frame_line = true;

    actToggleFrameLine=new QAction(QIcon(":icons/jkqtp_frameline.png"), tr("Toggle Frame Line"), this);
    actToggleFrameLine->setToolTip(tr("Toggle the frame line for this plot"));

    toolbar->addAction(this->getActionToggleFrameLine());

    connect(actToggleFrameLine, SIGNAL(triggered()), this, SLOT(ToggleFrameLine()));

    this->setToolbarAlwaysOn(true);
}

QAction *EngineeringPlot::getActionToggleFrameLine() const {
    return this->actToggleFrameLine;
}

EngineeringPlot::~EngineeringPlot()
{
}

void EngineeringPlot::PlotChart()
{
    int plot_number_tracks = 1;

    std::function<std::vector<double>(size_t)> func_x, func_y;

    func_x = std::bind(&EngineeringPlot::get_individual_x_track, this, std::placeholders::_1);

    if (plotXType == Enums::PlotType::Azimuth)
    {
        func_x = std::bind(&EngineeringPlot::get_individual_y_track_azimuth, this, std::placeholders::_1);
    }
    else if (plotXType == Enums::PlotType::Boresight_Azimuth)
    {
        func_x = std::bind(&EngineeringPlot::get_individual_y_track_boresight_azimuth, this, std::placeholders::_1);
    }
    else if (plotXType == Enums::PlotType::Elevation)
    {
        func_x = std::bind(&EngineeringPlot::get_individual_y_track_boresight_elevation, this, std::placeholders::_1);
    }
    else if (plotXType == Enums::PlotType::Boresight_Elevation)
    {
        func_x = std::bind(&EngineeringPlot::get_individual_y_track_elevation, this, std::placeholders::_1);
    }
    else if (plotXType == Enums::PlotType::Irradiance)
    {
        func_x = std::bind(&EngineeringPlot::get_individual_y_track_irradiance, this, std::placeholders::_1);
    }
    else
    {
        func_x = std::bind(&EngineeringPlot::get_individual_x_track, this, std::placeholders::_1);
    }

    if (plotYType == Enums::PlotType::Azimuth)
    {
        func_y = std::bind(&EngineeringPlot::get_individual_y_track_azimuth, this, std::placeholders::_1);
    }
    else if (plotYType == Enums::PlotType::Boresight_Azimuth)
    {
        func_y = std::bind(&EngineeringPlot::get_individual_y_track_boresight_azimuth, this, std::placeholders::_1);
    }
    else if (plotYType == Enums::PlotType::Boresight_Elevation)
    {
        func_y = std::bind(&EngineeringPlot::get_individual_y_track_boresight_elevation, this, std::placeholders::_1);
    }
    else if (plotYType == Enums::PlotType::Elevation)
    {
        func_y = std::bind(&EngineeringPlot::get_individual_y_track_elevation, this, std::placeholders::_1);
    }
    else if (plotYType == Enums::PlotType::Irradiance)
    {
        func_y = std::bind(&EngineeringPlot::get_individual_y_track_irradiance, this, std::placeholders::_1);
    }
    else
    {
        func_y = std::bind(&EngineeringPlot::get_individual_x_track, this, std::placeholders::_1);
    }

    PlotSirveQuantities(func_x, func_y, plot_number_tracks, my_quantities.at(0).getName());
    PlotSirveTracks();

    this->getPlotter()->setPlotLabel(plot_classification);
}

void EngineeringPlot::PlotSirveTracks()
{
    if (plotYType == Enums::PlotType::Azimuth)
    {
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
            AddSeriesWithColor(x_values, y_values, track_id);
        }
    }
}

void EngineeringPlot::PlotSirveQuantities(std::function<std::vector<double>(size_t)> get_x_func, std::function<std::vector<double>(size_t)> get_y_func, size_t plot_number_tracks, QString title)
{
    for (size_t track_index = 0; track_index < plot_number_tracks; track_index++)
    {
        std::vector<double> x_values = get_x_func(track_index);
        std::vector<double> y_values = get_y_func(track_index);

        QVector<double> X(x_values.begin(), x_values.end());
        QVector<double> Y(y_values.begin(), y_values.end());

        // make data available to JKQTPlotter by adding it to the internal datastore.
        size_t columnX=ds->addCopiedColumn(X, Enums::plotTypeToString(plotXType));
        size_t columnY=ds->addCopiedColumn(Y, Enums::plotTypeToString(plotYType));

        // create a graph in the plot, which plots the dataset X/Y:
        graph=new JKQTPXYLineGraph(this);
        graph->setXColumn(columnX);
        graph->setYColumn(columnY);
        graph->setTitle(title.replace('_',' '));

        graph->setSymbolSize(5);
        graph->setSymbolLineWidth(1);
        graph->setColor(colors.get_current_color());
        graph->setSymbolColor(QColor::fromRgb(255,20,20));

        // add the graph to the plot, so it is actually displayed
        this->addGraph(graph);

        this->getXAxis()->setAxisLabel(my_quantities[1].getName().replace('_', ' ') + " (" + Enums::plotUnitToString(my_quantities[1].getUnit()) + ") ");
        this->getYAxis()->setAxisLabel(my_quantities[0].getName().replace('_', ' ') + " (" + Enums::plotUnitToString(my_quantities[0].getUnit()) + ") ");
        this->getYAxis()->setLabelFontSize(10); // large x-axis label
        this->getYAxis()->setTickLabelFontSize(10); // and larger y-axis tick labels

        // autoscale the plot so the graph is contained
        this->zoomToFit();

        // get the upper bound for drawing the frame line
        this->fixed_max_y = *std::max_element(y_values.begin(), y_values.end());
        InitializeFrameLine(index_sub_plot_xmin + 0);
    }
}

void EngineeringPlot::set_plotting_track_frames(std::vector<PlottingTrackFrame> frames, int num_unique)
{
    track_frames = frames;
    number_of_tracks = num_unique;
}

std::vector<double> EngineeringPlot::get_individual_x_track(size_t i)
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

std::vector<double> EngineeringPlot::get_individual_y_track_irradiance(size_t i)
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

std::vector<double> EngineeringPlot::get_individual_y_track_azimuth(size_t i)
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

std::vector<double> EngineeringPlot::get_individual_y_track_boresight_azimuth(size_t i)
{
    std::vector<double> y_values;
    num_frames = static_cast<unsigned int>(osm_frames_ref->size());

    for (size_t i = 0; i < num_frames; i++)
    {
        y_values.push_back(osm_frames_ref->at(i).data.az_el_boresight[0]);
    }

    return y_values;
}

std::vector<double> EngineeringPlot::get_individual_y_track_boresight_elevation(size_t i)
{
    std::vector<double> y_values;
    num_frames = static_cast<unsigned int>(osm_frames_ref->size());

    for (size_t i = 0; i < num_frames; i++)
    {
        y_values.push_back(osm_frames_ref->at(i).data.az_el_boresight[1]);
    }

    return y_values;
}

std::vector<double> EngineeringPlot::get_individual_y_track_elevation(size_t i)
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

// void EngineeringPlot::set_xaxis_units(Enums::PlotUnit unit_choice)
// {
//     x_axis_units = unit_choice;
//     switch (x_axis_units)
//     {
//     case Enums::Frames:
//         x_title = "Frame #";
//         break;
//     case Enums::Seconds_Past_Midnight:
//         x_title = "Seconds Past Midnight";
//         break;
//     case Enums::Seconds_From_Epoch:
//         x_title = "Seconds Past Epoch";
//         break;
//     default:
//         break;
//     }
// }

std::vector<double> EngineeringPlot::get_x_axis_values(unsigned int start_idx, unsigned int end_idx)
{
    switch (x_axis_units)
    {
    case Enums::Frames:
    {
        std::vector<double> x_values(end_idx - start_idx + 1);
        std::iota(std::begin(x_values), std::end(x_values), start_idx + 1);
        return x_values;
    }
    case Enums::Seconds_Past_Midnight:
        return std::vector<double>(past_midnight.begin() + start_idx, past_midnight.begin() + end_idx + 1);
    case Enums::Seconds_From_Epoch:
        return std::vector<double>(past_epoch.begin() + start_idx, past_epoch.begin() + end_idx + 1);
    default:
        return std::vector<double>();
    }
}

double EngineeringPlot::get_single_x_axis_value(int x_index)
{
    switch (x_axis_units)
    {
    case Enums::Frames:
        return x_index + 1;
    case Enums::Seconds_Past_Midnight:
        return past_midnight[x_index];
    case Enums::Seconds_From_Epoch:
        return past_epoch[x_index];
    default:
        return 0;
    }
}

double EngineeringPlot::get_max_x_axis_value()
{
    switch (x_axis_units)
    {
    case Enums::Frames:
        return num_frames;
    case Enums::Seconds_Past_Midnight:
        return past_midnight[past_midnight.size() - 1];
    case Enums::Seconds_From_Epoch:
        return past_epoch[past_epoch.size() - 1];
    default:
        return 0;
    }
}

std::vector<Quantity> EngineeringPlot::get_params()
{
    return my_quantities;
}

void EngineeringPlot::InitializeFrameLine(double frameline_x)
{
    // Define data points for the line
    QVector<double> xData = {frameline_x, frameline_x};
    QVector<double> yData = {0, this->fixed_max_y};

    // Make data available to JKQTPlotter by adding it to the internal datastore.
    frameLineColumnX = ds->addCopiedColumn(xData, "frameline_X");
    size_t frameLineColumnY = ds->addCopiedColumn(yData, "frameline_Y");

    // Create the line graph and set its data
    JKQTPXYLineGraph *lineGraph = new JKQTPXYLineGraph();
    lineGraph->setXColumn(frameLineColumnX);
    lineGraph->setYColumn(frameLineColumnY);

    this->addGraph(lineGraph);
}

void EngineeringPlot::PlotCurrentFrameline(int frame)
{
    if (plot_current_marker && show_frame_line)
    {
        int frameline_x = frame + index_sub_plot_xmin + 1; // the chart itself represents data in base-1, so add one here to base-0 index
        QVector<double> updatedXData = {(double)frameline_x, (double)frameline_x};
        ds->setColumnData(frameLineColumnX, updatedXData);
        emit this->plotter->plotUpdated();
    }
}

void EngineeringPlot::toggle_yaxis_log(bool input)
{
    // yaxis_is_log = input;
}

void EngineeringPlot::toggle_yaxis_scientific(bool input)
{
    // yaxis_is_scientific = input;
}

void EngineeringPlot::toggle_xaxis_fixed_pt(bool input)
{
    // xaxis_is_fixed_pt = input;
}

void EngineeringPlot::SetPlotClassification(QString classification)
{
    plot_classification = classification;
    this->plotter->setPlotLabel(plot_classification);
}

void EngineeringPlot::ToggleFrameLine()
{
    show_frame_line = ! show_frame_line;
    this->getGraphs().at(1)->setVisible(show_frame_line);
    emit this->plotter->plotUpdated();
}

void EngineeringPlot::UpdateManualPlottingTrackFrames(std::vector<ManualPlottingTrackFrame> frames, std::set<int> track_ids)
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
}

void EngineeringPlot::AddSeriesWithColor(std::vector<double> x_values, std::vector<double> y_values, int track_id)
{
    QString title = "Track " + QString::number(track_id);

    DeleteGraphIfExists(title);

    QVector<double> X(x_values.begin(), x_values.end());
    QVector<double> Y(y_values.begin(), y_values.end());

    // make data available to JKQTPlotter by adding it to the internal datastore.
    size_t columnX=ds->addCopiedColumn(X, "x");
    size_t columnY=ds->addCopiedColumn(Y, "y");

    // create a graph in the plot, which plots the dataset X/Y:
    graph=new JKQTPXYLineGraph(this);
    graph->setXColumn(columnX);
    graph->setYColumn(columnY);
    graph->setTitle("Track " + QString::number(track_id));
    //graph->setSymbolSize(5);
    graph->setSymbolLineWidth(1);
    graph->setColor(manual_track_colors[track_id]);

    // Streamline the line style for the graph
    graph->setLineStyle(Qt::SolidLine);
    graph->setSymbolType(JKQTPNoSymbol);

    // add the graph to the plot, so it is actually displayed
    this->addGraph(graph);
}

void EngineeringPlot::DeleteGraphIfExists(const QString& titleToFind) {

    int index = 0;
    bool graph_exists = false;
    qDebug() << "Deleting " << titleToFind;

    for (auto it = this->getGraphs().begin(); it != this->getGraphs().end(); it++, index++) { // Iterate over all plots (graphs)
        QString title = (*it)->getTitle();
        if (title == titleToFind) {
            graph_exists = true;
            break;
        }
    }

    if (graph_exists)
        this->getGraphs().removeAt(index);
}

void EngineeringPlot::RecolorManualTrack(int track_id, QColor new_color)
{
    manual_track_colors[track_id] = new_color;
}

void EngineeringPlot::RecolorOsmTrack(QColor color)

{
    emit updatePlots();
}
void EngineeringPlot::HandlePlayerButtonClick()
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

void EngineeringPlot::EditPlotText()
{
    bool ok;
    QString input_text = QInputDialog::getText(0, "Plot Header Text", "Input Plot Header Text", QLineEdit::Normal, this->getPlotter()->getPlotLabel(), &ok);

    if (ok)
    {
        this->getPlotter()->setPlotLabel(input_text);
    }
}

void EngineeringPlot::copyStateFrom(const EngineeringPlot &other) {

    // Get the datastore from the source plotter
    JKQTPDatastore* srcDatastore = other.get_data_store();

    // Step 1: Clear existing data in the destination datastore
    ds->clear();

    // Step 2: Access the data for X and Y columns from the source datastore
    // Assuming getColumnData (or a similar method) is available instead of getColumn

    QString name0 = srcDatastore->getColumnName(0);
    QString name1 = srcDatastore->getColumnName(1);

    QVector<double> srcXData = srcDatastore->getData(0, &name0);  // Get X data from source datastore
    QVector<double> srcYData = srcDatastore->getData(1, &name1);

    // Step 3: Add the copied columns to the destination datastore (ds)
    size_t dstXColumn = ds->addCopiedColumn(srcXData, "x");  // Add X data to destination datastore (ds)
    size_t dstYColumn = ds->addCopiedColumn(srcYData, "y");  // Add Y data to destination datastore (ds)

    // Step 4: Copy the graph configuration from the source to the destination plot
    this->clearGraphs();  // Clear existing graphs in the destination plotter

    // Step 5: Create a new graph for the destination plotter
    auto* srcGraph = dynamic_cast<JKQTPXYLineGraph*>(other.graph);  // Get the first graph from the source
    if (srcGraph) {
        auto* dstGraph = new JKQTPXYLineGraph(this);  // Create a new graph in the destination plotter

        // Set the X and Y columns in the new graph to the copied columns
        dstGraph->setXColumn(dstXColumn);
        dstGraph->setYColumn(dstYColumn);

        // Copy other properties from the source graph
        dstGraph->setTitle(srcGraph->getTitle());
        dstGraph->setSymbolSize(srcGraph->getSymbolSize());
        dstGraph->setSymbolLineWidth(srcGraph->getSymbolLineWidth());
        dstGraph->setColor(srcGraph->getLineColor());
        dstGraph->setSymbolColor(srcGraph->getSymbolColor());

        // Add the new graph to the destination plot
        this->addGraph(dstGraph);
    }

    // Step 6: Autoscale the plot to fit the new data
    this->zoomToFit();

    this->plotter->setPlotLabel(other.getPlotter()->getPlotLabel());
}
