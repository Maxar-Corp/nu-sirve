#include <QPushButton>
#include <QLegendMarker>
#include <functional>
#include "jkqtplotter/graphs/jkqtpscatter.h"
#include "plot_engineering_data.h"
#include "enums.h"
#include "SirveApp.h"


EngineeringPlot::EngineeringPlot(std::vector<Frame> const &osm_frames, QString plot_title, std::vector<Quantity> quantities) : JKQTPlotter()
{
    osm_frames_ref = &osm_frames;

    plotTitle = plot_title;
    plot_classification = "EDIT CLASSIFICATION";
    my_quantities = quantities;

    plotYType = Enums::getPlotTypeByIndex(Enums::getPlotTypeIndexFromString(quantities.at(0).getName()));
    plotXType = Enums::getPlotTypeByIndex(Enums::getPlotTypeIndexFromString(quantities.at(1).getName()));
    num_frames = static_cast<unsigned int>(osm_frames.size());

    EngineeringData *engineeringData = new EngineeringData(osm_frames);

    this->past_midnight = engineeringData->get_seconds_from_midnight();
    this->past_epoch = engineeringData->get_seconds_from_epoch();

    plot_all_data = true;
    plot_primary_only = false;

    x_axis_units = plotXType;

    // these next two variables represent the scope of all data that has been read in (Hence, the 'full scope').
    // they are base-zero indices, so we use the 'index_' prefix.
    index_full_scope_xmin = 0;
    index_full_scope_xmax = num_frames - 1;

    ds = this->getDatastore();
    ds->clear();

    // Set up frame line buttonology
    show_frame_line = x_axis_units == Enums::Frames || x_axis_units == Enums::Seconds_From_Epoch || x_axis_units == Enums::Seconds_Past_Midnight;

    actToggleFrameLine=new QAction(QIcon(":icons/jkqtp_frameline.png"), tr("Toggle Frame Line"), this);
    actToggleFrameLine->setToolTip(tr("Toggle the frame line for this plot"));

    toolbar->addAction(this->get_action_toggle_frameline());

    // Set up data scope toggler
    actToggleDataScope = new QAction(QIcon(":icons/full-data.png"), tr("Toggle Data Scope"), this);
    actToggleDataScope->setToolTip(tr("Toggle full vs. user-selected data."));

    toolbar->addAction(this->get_action_toggle_datascope());

    actToggleGraphStyle = new QAction(QIcon(":icons/solid-style.png"), tr("Toggle Plot Type"), this);
    actToggleGraphStyle->setToolTip("Toggle between scatter plot and line plot types.");

    toolbar->addAction(this->get_action_toggle_graphstyle());

    connect(actToggleDataScope, SIGNAL(triggered()), this, SLOT(ToggleDataScope()));
    connect(actToggleFrameLine, SIGNAL(triggered()), this, SLOT(ToggleFrameLine()));
    connect(actToggleGraphStyle, SIGNAL(triggered()), this, SLOT(ToggleGraphStyle()));

    if (plotYType == Enums::PlotType::SumCounts)
    {
        qDebug() << "SUM COUNTS";
        actToggleLinearLog = new QAction(QIcon(":icons/solid-style.png"), tr("Toggle Plot Mode"), this);
        actToggleLinearLog->setToolTip("Toggle between linear and log Y axis.");
        toolbar->addAction(this->get_action_toggle_linearlog());
        connect(actToggleLinearLog, SIGNAL(triggered()), this, SLOT(ToggleLinearLog()));
    }

    connect(actMouseMoveToolTip, SIGNAL(triggered()), this, SLOT(ToggleGraphTickSymbol()));

    connect(this, &JKQTPlotter::contextActionTriggered, this, &EngineeringPlot::onJPContextActionTriggered);

    this->setExclusionString("frameline");
    this->setMinimumWidth(200);
    this->plotter->setWidgetWidth(100);
    this->setToolbarAlwaysOn(true);
}

void EngineeringPlot::onJPContextActionTriggered(const QString& actionName) {
    if (actionName == "Snap It" && ! get_show_full_scope()) {

        if (get_quantity_unit_by_axis(1) == Enums::PlotUnit::Seconds)
        {
            double fraction = double(snap_x - get_single_x_axis_value(partial_scope_original_min_x)) / double(get_single_x_axis_value(partial_scope_original_max_x) - get_single_x_axis_value(partial_scope_original_min_x));
            emit frameNumberChanged(fraction * (partial_scope_original_max_x - partial_scope_original_min_x));
        }
        else
        {
            emit frameNumberChanged(snap_x - partial_scope_original_min_x);
        }
    }
}

void EngineeringPlot::DisableDataScopeButton(bool value)
{
    QToolButton* toggle_button = FindToolButtonForAction(toolbar, actToggleDataScope);
    toggle_button->setDisabled(value);
}

QToolButton* EngineeringPlot::FindToolButtonForAction(QToolBar* toolbar, QAction* action) {
    const QList<QToolButton*> buttons = toolbar->findChildren<QToolButton*>();
    for (QToolButton* button : buttons) {
        if (button->defaultAction() == action) {
            return button;
        }
    }
    return nullptr; // Not found
}

QAction *EngineeringPlot::get_action_toggle_graphstyle() const {
    return this->actToggleGraphStyle;
}

QAction *EngineeringPlot::get_action_toggle_frameline() const {
    return this->actToggleFrameLine;
}

QAction *EngineeringPlot::get_action_toggle_datascope() const {
    return this->actToggleDataScope;
}

QAction *EngineeringPlot::get_action_toggle_linearlog() const {
    return this->actToggleLinearLog;
}

Enums::PlotUnit EngineeringPlot::get_quantity_unit_by_axis(int axis_index)
{
    return my_quantities[axis_index].getUnit();
}

EngineeringPlot::~EngineeringPlot()
{
}

void EngineeringPlot::print_ds(JKQTPDatastore* _ds)
{
    qDebug() << "Contents of Data Store:";
    if (_ds->getMaxRows() > 0) {
        for (int i = 0; i < 10; i++)
        {
            QString row = *new QString();
            for (int j=0; j < _ds->getColumnCount(); j++)
            {
                double val = _ds->get(j, i);

                QString number_string = QString::number(val);

                if (val <= 0 || val > 100000 || number_string.contains('e'))
                    number_string = "NaN";

                row += number_string + " ";
            }
            qDebug() << row;
        }
    }
}

FuncType EngineeringPlot::DeriveFunctionPointers(Enums::PlotType type)
{
    std::function<std::vector<double>(size_t)> func;

    if (type == Enums::PlotType::Azimuth)
    {
        func = std::bind(&EngineeringPlot::get_individual_y_track_azimuth, this, std::placeholders::_1);
    }
    else if (type == Enums::PlotType::Boresight_Azimuth)
    {
        func = std::bind(&EngineeringPlot::get_individual_y_track_boresight_azimuth, this, std::placeholders::_1);
    }
    else if (type == Enums::PlotType::Elevation)
    {
        func = std::bind(&EngineeringPlot::get_individual_y_track_elevation, this, std::placeholders::_1);
    }
    else if (type == Enums::PlotType::FovX)
    {
        func = std::bind(&EngineeringPlot::get_individual_y_track_fov_x, this, std::placeholders::_1);
    }
    else if (type == Enums::PlotType::FovY)
    {
        func = std::bind(&EngineeringPlot::get_individual_y_track_fov_y, this, std::placeholders::_1);
    }
    else if (type == Enums::PlotType::Boresight_Elevation)
    {
        func = std::bind(&EngineeringPlot::get_individual_y_track_boresight_elevation, this, std::placeholders::_1);
    }
    else if (type == Enums::PlotType::SumCounts)
    {
        func = std::bind(&EngineeringPlot::get_individual_y_track_irradiance, this, std::placeholders::_1);
    }
    else
    {
        func = std::bind(&EngineeringPlot::get_individual_x_track, this, std::placeholders::_1);
    }

    return func;
}

void EngineeringPlot::PlotChart()
{
    int plot_number_tracks = 1;

    std::function<std::vector<double>(size_t)> func_x, func_y;

    func_x = DeriveFunctionPointers(plotXType);
    func_y = DeriveFunctionPointers(plotYType);

    PlotSirveQuantities(func_x, func_y, plot_number_tracks);

    this->getPlotter()->setPlotLabel(plot_classification);
}

void EngineeringPlot::GetTrackValues(int &track_id, std::vector<double> &x_values, std::vector<double> &y_values)
{
    for (size_t i = 0; i < manual_track_frames.size(); i++) {
        std::map<int, ManualPlottingTrackDetails>::iterator it =
            manual_track_frames[i].tracks.find(track_id);
        if (it != manual_track_frames[i].tracks.end()) {
            x_values.push_back(get_single_x_axis_value(i));

            if (my_quantities.at(0).getName() == "Azimuth") {
                y_values.push_back(it->second.azimuth);
            } else if (my_quantities.at(0).getName() == "Elevation") {
                y_values.push_back(it->second.elevation);
            } else if (my_quantities.at(0).getName() == "SumCounts") {
                y_values.push_back(it->second.sum_relative_counts);
            }
        }
    }
}

void EngineeringPlot::PlotSirveTracks(int override_track_id)
{
    for (int track_id : manual_track_ids) {
        std::vector<double> x_values, y_values;

        GetTrackValues(track_id, x_values, y_values);

        size_t columnX, columnY;

        if (!TrackExists(track_id)) {
            AddTrack(x_values, y_values, track_id, columnX, columnY);
            AddTypedGraph(graph_type, columnX, columnY, track_id, "Track " + QString::number(track_id));
        } else {
            if (track_id == override_track_id)
                ReplaceTrack(x_values, y_values, track_id);
            DeleteGraphIfExists("Track " + QString::number(track_id));
            LookupTrackColumnIndexes(track_id, columnX, columnY);
            AddTypedGraph(graph_type, columnX, columnY, track_id, "Track " + QString::number(track_id));
            emit this->plotter->plotUpdated();
        }
    }
}

void EngineeringPlot::AddTypedGraph(Enums::GraphType graph_type, size_t columnX, size_t columnY,
                                            std::optional<int> track_id, std::optional<QString> graph_title)
{
    QColor graph_color = track_id.has_value() ? manual_track_colors[track_id.value()] : colors.get_current_color();

    if (graph_type == Enums::GraphType::Scatter)
    {
        graph = new JKQTPXYScatterGraph(plotter);
        graph->setXColumn(columnX);
        graph->setYColumn(columnY);

        JKQTPGraphSymbols customSymbol=JKQTPRegisterCustomGraphSymbol(
            [graph_color](QPainter& p) {
                const double w=p.pen().widthF();
                p.setPen(QPen(graph_color, w));
                p.drawEllipse(QPointF(0.33/2.0, 0.33/4.0), 0.33/2.0, 0.33/2.0);
            });

        if (auto scatter_graph = dynamic_cast<JKQTPXYScatterGraph*>(graph)) {
            scatter_graph->setSymbolSize(3.0);
            scatter_graph->setSymbolType(customSymbol);
        }
    }
    else
    {
        graph=new JKQTPXYLineGraph(this);
        graph->setXColumn(columnX);
        graph->setYColumn(columnY);

        if (auto line_graph = dynamic_cast<JKQTPXYLineGraph*>(graph))
        {
            line_graph->setSymbolSize(0.1);
            line_graph->setSymbolLineWidth(1);
            line_graph->setColor(graph_color);
            line_graph->setSymbolColor(QColor::fromRgb(255,20,20));
            line_graph->setLineStyle(Qt::SolidLine);
        }
    }

    // Use the osm data plotTitle (provided on instantiation), unless a Track Title has been passed
    QString title = graph_title.has_value() ? graph_title.value() : plotTitle;
    graph->setTitle(title);

    this->addGraph(graph);
}

void EngineeringPlot::PlotSirveQuantities(std::function<std::vector<double>(size_t)> get_x_func, std::function<std::vector<double>(size_t)> get_y_func, size_t plot_number_tracks)
{
    x_osm_values = get_x_func(0);
    y_osm_values = get_y_func(0);

    graph_type = get_quantity_unit_by_axis(1) == Enums::PlotUnit::Degrees ? Enums::GraphType::Scatter : Enums::GraphType::Line;

    QVector<double> X;
    QVector<double> Y;

    if (graph_type != Enums::GraphType::Scatter)
    {
        X = QVector<double>(x_osm_values.begin(), x_osm_values.end());
        Y = QVector<double>(y_osm_values.begin(), y_osm_values.end());
    } else
    {
        std::vector<double> filtered_x, filtered_y;
        set_show_full_scope(false);
        DefineSubSet(filtered_x, filtered_y);
        X = QVector<double>(filtered_x.begin(), filtered_x.end());
        Y = QVector<double>(filtered_y.begin(), filtered_y.end());
    }

    size_t columnX=ds->addCopiedColumn(X, Enums::plotTypeToString(plotXType));
    size_t columnY=ds->addCopiedColumn(Y, Enums::plotTypeToString(plotYType));

    AddTypedGraph(graph_type, columnX, columnY);

    QString unitsXAxis = Enums::plotUnitToString(my_quantities[1].getUnit()).contains("Undefined") ? "" :  " (" + Enums::plotUnitToString(my_quantities[1].getUnit()) + ") ";
    this->getXAxis()->setAxisLabel(my_quantities[1].getName().replace('_', ' ') + unitsXAxis);

    QString unitsYAxis = Enums::plotUnitToString(my_quantities[0].getUnit()).contains("Undefined") ? "" :  " (" + Enums::plotUnitToString(my_quantities[0].getUnit()) + ") ";
    this->getYAxis()->setAxisLabel(my_quantities[0].getName().replace('_', ' ') + unitsYAxis);
    this->getYAxis()->setLabelFontSize(10); // large x-axis label
    this->getYAxis()->setTickLabelFontSize(10); // and larger y-axis tick labels

    this->zoomToFit();

    connect(this->plotter->actZoomAll, SIGNAL(triggered()), this, SLOT(HomeZoomIn()));

    this->fixed_max_y = *std::max_element(y_osm_values.begin(), y_osm_values.end()); // get the upper bound for drawing the frame line

    InitializeFrameLine(index_full_scope_xmin);
}

int EngineeringPlot::get_index_full_scope_xmin() const
{
    return index_full_scope_xmin;
}

int EngineeringPlot::get_index_full_scope_xmax() const
{
    return index_full_scope_xmax;
}

int EngineeringPlot::get_index_partial_scope_xmin() const
{
    return index_partial_scope_xmin;
}

int EngineeringPlot::get_index_partial_scope_xmax() const
{
    return index_partial_scope_xmax;
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
            y_values.push_back(track_frames[track_frame_index].details[i].sum_relative_counts);
        }
    }

    return y_values;
}

std::vector<double> EngineeringPlot::get_individual_y_track_sum_relative_counts(size_t i)
{
    std::vector<double> y_values;
    for (int track_frame_index = 0; track_frame_index < track_frames.size(); track_frame_index += 1)
    {
        if (i < track_frames[track_frame_index].details.size())
        {
            y_values.push_back(track_frames[track_frame_index].details[i].sum_relative_counts);
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

std::vector<double> EngineeringPlot::get_individual_y_track_fov_x(size_t i)
{
    std::vector<double> y_values;
    num_frames = static_cast<unsigned int>(osm_frames_ref->size());

    for (size_t i = 0; i < num_frames; i++)
    {
        y_values.push_back(osm_frames_ref->at(i).data.i_fov_x * 1e6);
    }

    return y_values;
}

std::vector<double> EngineeringPlot::get_individual_y_track_fov_y(size_t i)
{
    std::vector<double> y_values;
    num_frames = static_cast<unsigned int>(osm_frames_ref->size());

    for (size_t i = 0; i < num_frames; i++)
    {
        y_values.push_back(osm_frames_ref->at(i).data.i_fov_y * 1e6);
    }

    return y_values;
}

// The next three functions return the X value(s) associated with a range of indices or a single index.

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
        return std::vector<double>(past_midnight.begin() + start_idx + 1, past_midnight.begin() + end_idx + 1);
    case Enums::Seconds_From_Epoch:
        return std::vector<double>(past_epoch.begin() + start_idx + 1, past_epoch.begin() + end_idx + 1);
    default:
        return std::vector<double>();
    }
}

void EngineeringPlot::set_index_partial_scope_xmin(int value)
{
    index_partial_scope_xmin = value;
}

void EngineeringPlot::set_index_partial_scope_xmax(int value)
{
    index_partial_scope_xmax = value;
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

double EngineeringPlot::get_min_x_axis_value()
{
    switch (x_axis_units)
    {
    case Enums::Frames:
        return 1;
    case Enums::Seconds_Past_Midnight:
        return past_midnight[0];
    case Enums::Seconds_From_Epoch:
        return past_epoch[0];
    default:
        double min_value = 0;
        auto min_it = std::min_element(x_osm_values.begin(), x_osm_values.end());
        if (min_it != x_osm_values.end()) {
            min_value = *min_it;
        } else {
            std::cout << "Vector is empty." << std::endl;
        }
        return min_value;
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
        double max_value = 0;
        auto max_it = std::max_element(x_osm_values.begin(), x_osm_values.end());
        if (max_it != x_osm_values.end()) {
            max_value = *max_it;
        } else {
            std::cout << "Vector is empty." << std::endl;
        }
        return max_value;
    }
}

std::vector<Quantity> EngineeringPlot::get_params()
{

    return my_quantities;
}

bool EngineeringPlot::get_plot_primary_only()
{
    return plot_primary_only;
}

QString EngineeringPlot::get_plot_title() const
{
    return plotTitle;
}

bool EngineeringPlot::get_show_full_scope()
{
    return plotter->show_full_scope;
}

int EngineeringPlot::get_subinterval_min() const
{
    return this->getXAxis()->getMin();
}

int EngineeringPlot::get_subinterval_max() const
{
    return this->getXAxis()->getMax();
}

void  EngineeringPlot::mousePressEvent(QMouseEvent* event)  {

    if (event->button() == Qt::RightButton) {
        snap_x = plotter->p2x(event->pos().x() / magnification);
    }

    JKQTPlotter::mousePressEvent(event);
}

void EngineeringPlot::set_plot_primary_only(bool value)
{
    plot_primary_only = value;
}

void EngineeringPlot::set_plotting_track_frames(std::vector<PlottingTrackFrame> frames, int num_unique)
{
    track_frames = frames;
    number_of_tracks = num_unique;
}

void EngineeringPlot::set_pre_image(double min_x, double max_x)
{
    partial_scope_original_min_x = min_x;
    partial_scope_original_max_x = max_x;
}

void EngineeringPlot::set_show_full_scope(bool value)
{
    plotter->show_full_scope =value;
}

void EngineeringPlot::set_data_scope_icon(QString type)
{
    actToggleDataScope->setIcon(QIcon(":icons/"+type+"-data.png"));
}

void EngineeringPlot::set_graph_style_icon(QString style)
{
    actToggleGraphStyle->setIcon(QIcon(":icons/"+style+"-style.png"));
}

void EngineeringPlot::set_graph_mode_icon(QString mode)
{
    actToggleLinearLog->setIcon(QIcon(":icons/"+mode+"-mode.png"));
}

void EngineeringPlot::set_sub_plot_xmin(int value)
{
    plotter->sub_plot_xmin = value;
}

void EngineeringPlot::set_sub_plot_xmax(int value)
{ 
    plotter->sub_plot_xmax = value;
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

void EngineeringPlot::InitializeFrameLine(double frameline_x)
{
    QVector<double> xData = {frameline_x, frameline_x};
    QVector<double> yData = {-SirveAppConstants::BigNumber, SirveAppConstants::BigNumber};

    frameLineColumnX = ds->addCopiedColumn(xData, "frameline_X");
    size_t frameLineColumnY = ds->addCopiedColumn(yData, "frameline_Y");

    JKQTPXYLineGraph *lineGraph = new JKQTPXYLineGraph();
    lineGraph->setXColumn(frameLineColumnX);
    lineGraph->setYColumn(frameLineColumnY);
    lineGraph->setTitle("Frame Line");

    this->addGraph(lineGraph);
    this->getGraphs().at(1)->setVisible(show_frame_line);
}

void EngineeringPlot::PlotCurrentFrameline(int frame)
{
    if (show_frame_line)
    {
        double frameline_x;

        if (get_quantity_unit_by_axis(1) == Enums::PlotUnit::Seconds)
        {
            frameline_x = get_single_x_axis_value(frame + partial_scope_original_min_x - 1);
        } else
        {
            frameline_x = frame + index_full_scope_xmin + 1; // the chart itself represents data in base-1, so add one here to base-0 index
        }

        QVector<double> updatedXData = {(double)frameline_x, (double)frameline_x};
        ds->setColumnData(frameLineColumnX, updatedXData);
        emit this->plotter->plotUpdated();
    }
}

void EngineeringPlot::SetPlotClassification(QString classification)
{
    plot_classification = classification;
    this->plotter->setPlotLabel(plot_classification);
}

void EngineeringPlot::DefineFullPlotInterval()
{
    full_plot_xmin = get_min_x_axis_value();
    full_plot_xmax = get_max_x_axis_value();
}

void EngineeringPlot::DefinePlotSubInterval(int min, int max)
{
    index_full_scope_xmin = min; // for the frameline offset
    index_full_scope_xmax = max; // for the workspace, et. al.

    std::function<std::vector<double>(size_t)> func_y = DeriveFunctionPointers(plotYType);
    std::vector<double> y_values = func_y(1);

    if (!y_values.empty()) {
        auto [min_element, max_element] = std::minmax_element(y_values.begin(), y_values.end());
        plotter->sub_plot_ymin = *min_element;
        plotter->sub_plot_ymax = *max_element;
    }
}

void EngineeringPlot::HomeZoomIn()
{
    // Here we check the X axis' units.  If they are degrees, then this plot was
    // instantiated without a linear X axis, and therefore it does not have a "sub interval"
    // the same way as plots that are measured in Frames or Seconds do.  These non-linear plots
    // instead have 2D subsets, the elements (x,y's) of which depend upon the linear range of
    // start-to-stop frames that the user selected in the SIRVE gui.
    if (get_quantity_unit_by_axis(1) == Enums::PlotUnit::Degrees)
    {
        plotter->getYAxis()->setMin(0); // this bound could be improved.
        plotter->getYAxis()->setMax(fixed_max_y);
        SetPlotterXAxisMinMax(get_min_x_axis_value(), get_max_x_axis_value());
    }
    else if (plotter->show_full_scope)
    {
        this->zoomToFit();
    }
    else
    {
        SetPlotterXAxisMinMax(plotter->sub_plot_xmin, plotter->sub_plot_xmax);
        SetPlotterYAxisMinMax(plotter->sub_plot_ymin, plotter->sub_plot_ymax);
    }
    redrawPlot();
}

void EngineeringPlot::RecordYAxisMinMax()
{
    plotter->sub_plot_ymax = plotter->getYAxis()->getMax();
    plotter->sub_plot_ymin = plotter->getYAxis()->getMin();
}

void EngineeringPlot::SetPlotterXAxisMinMax(int min, int max)
{
    if (plotter) {
        plotter->getXAxis()->setMin(min);
        plotter->getXAxis()->setMax(max);
    }
}

void EngineeringPlot::SetPlotterYAxisMinMax(int min, int max)
{
    if (plotter) {
        plotter->getYAxis()->setMin(min);
        plotter->getYAxis()->setMax(max);
    }
}

void EngineeringPlot::SetupSubRange(int min_x, int max_x)
{
    partial_scope_original_min_x = min_x;
    partial_scope_original_max_x = max_x;

    double transformed_min_x = min_x;
    double transformed_max_x = max_x;

    if (x_axis_units==Enums::PlotType::Seconds_From_Epoch || x_axis_units==Enums::PlotType::Seconds_Past_Midnight)
    {
        transformed_min_x = get_single_x_axis_value(min_x);
        transformed_max_x = get_single_x_axis_value(max_x);
    }

    set_sub_plot_xmin((int)transformed_min_x);
    set_sub_plot_xmax((int)transformed_max_x);
    SetPlotterXAxisMinMax((int)transformed_min_x, (int)transformed_max_x);
    redrawPlot();

    RecordYAxisMinMax();
    set_data_scope_icon("partial");
    set_graph_style_icon("solid");
    DisableDataScopeButton(false);
}

void EngineeringPlot::DefineSubSet(std::vector<double> &filtered_x, std::vector<double> &filtered_y)
{
    double min_index = plotter->show_full_scope ? get_index_full_scope_xmin() : get_index_partial_scope_xmin();
    double max_index = plotter->show_full_scope ? get_index_full_scope_xmax() : get_index_partial_scope_xmax();

    for (size_t i = min_index; i <= max_index && i < x_osm_values.size(); ++i) {

        try {
            double x = x_osm_values.at(i);
            double y = y_osm_values.at(i);
            filtered_x.push_back(x);
            filtered_y.push_back(y);
        } catch (const std::out_of_range& e) {
            qDebug() << "Error fetching value for index " << i;
        }
    }
}

void EngineeringPlot::ToggleDataScope()
{
    plotter->show_full_scope = ! plotter->show_full_scope;
    plotter->show_full_scope ? this->zoomToFit() : SetPlotterXAxisMinMax(plotter->sub_plot_xmin, plotter->sub_plot_xmax);

    if (my_quantities.at(1).getUnit() == Enums::PlotUnit::Degrees)
    {
        DeleteGraphIfExists(plotTitle);
        ds->clear();

        std::vector<double> filtered_x, filtered_y;

        DefineSubSet(filtered_x, filtered_y);

        QVector<double> X(filtered_x.begin(), filtered_x.end());
        QVector<double> Y(filtered_y.begin(), filtered_y.end());

        size_t columnX=ds->addCopiedColumn(X, Enums::plotTypeToString(plotXType));
        size_t columnY=ds->addCopiedColumn(Y, Enums::plotTypeToString(plotYType));

        AddTypedGraph(graph_type, columnX, columnY);
    }

    RecordYAxisMinMax();
    redrawPlot();

    plotter->show_full_scope ? actToggleDataScope->setIcon(QIcon(":icons/full-data.png")) : actToggleDataScope->setIcon(QIcon(":icons/partial-data.png"));
}

void EngineeringPlot::ToggleFrameLine()
{
    show_frame_line = ! show_frame_line;
    this->getGraphs().at(1)->setVisible(show_frame_line);
    emit this->plotter->plotUpdated();
}

void EngineeringPlot::ToggleGraphStyle()
{
    DeleteGraphIfExists(plotTitle);
    DeleteGraphIfExists("Frame Line");
    DeleteAllTrackGraphs();

    auto frameline_x = ds->get(frameLineColumnX,0);
    ds->clear();

    if (graph_type == Enums::GraphType::Line)
    {
        graph_type = Enums::GraphType::Scatter;

        QVector<double> X(x_osm_values.begin(), x_osm_values.end());
        QVector<double> Y(y_osm_values.begin(), y_osm_values.end());

        size_t columnX=ds->addCopiedColumn(X, Enums::plotTypeToString(plotXType));
        size_t columnY=ds->addCopiedColumn(Y, Enums::plotTypeToString(plotYType));

        AddTypedGraph(graph_type, columnX, columnY);
        set_graph_style_icon("dot");
    } else
    {
        graph_type = Enums::GraphType::Line;

        QVector<double> X(x_osm_values.begin(), x_osm_values.end());
        QVector<double> Y(y_osm_values.begin(), y_osm_values.end());

        size_t columnX=ds->addCopiedColumn(X, Enums::plotTypeToString(plotXType));
        size_t columnY=ds->addCopiedColumn(Y, Enums::plotTypeToString(plotYType));

        AddTypedGraph(graph_type, columnX, columnY);
        set_graph_style_icon("solid");
    }

    show_frame_line = x_axis_units == Enums::Frames || x_axis_units == Enums::Seconds_From_Epoch || x_axis_units == Enums::Seconds_Past_Midnight;
    InitializeFrameLine(frameline_x);
    PlotSirveTracks(-1);
    emit this->plotter->plotUpdated();
}

void EngineeringPlot::ToggleLinearLog()
{
    bool yAxisIsLogarithmic = getPlotter()->getYAxis()->getLogAxis();
    getPlotter()->getYAxis()->setLogAxis(!yAxisIsLogarithmic);
    getPlotter()->zoomToFit(true, true, false, false);

    int x_min, x_max;
    x_min = plotter->show_full_scope ? full_plot_xmin : get_subinterval_min();
    x_max = plotter->show_full_scope ? full_plot_xmax : get_subinterval_max();

    DefinePlotSubInterval(x_min, x_max);
    applyLinearLogStyling();

    if (!plotter->show_full_scope)
        SetPlotterXAxisMinMax(plotter->sub_plot_xmin, plotter->sub_plot_xmax);

    yAxisIsLogarithmic ? actToggleLinearLog->setIcon(QIcon(":icons/linear-mode.png")) : actToggleLinearLog->setIcon(QIcon(":icons/log-mode.png"));
}

void EngineeringPlot::AddGraph(int track_id, size_t &columnX, size_t &columnY)
{
    graph=new JKQTPXYLineGraph(this);

    graph->setXColumn(columnX);
    graph->setYColumn(columnY);
    graph->setTitle("Track " + QString::number(track_id));
    dynamic_cast<JKQTPXYLineGraph*>(graph)->setSymbolLineWidth(1);
    dynamic_cast<JKQTPXYLineGraph*>(graph)->setColor(manual_track_colors[track_id]);
    dynamic_cast<JKQTPXYLineGraph*>(graph)->setLineStyle(Qt::SolidLine);
    dynamic_cast<JKQTPXYLineGraph*>(graph)->setSymbolType(JKQTPNoSymbol);

    this->addGraph(graph);
}

void EngineeringPlot::DeleteGraphIfExists(const QString& titleToFind)
{
    int index = 0;
    bool graph_exists = false;

    for (auto it = this->getGraphs().begin(); it != this->getGraphs().end(); it++, index++) {
        if ((*it)->getTitle() == titleToFind) {
            graph_exists = true;
            break;
        }
    }

    if (graph_exists)
    {
        this->getGraphs().removeAt(index);
    }
}

void EngineeringPlot::DeleteAllTrackGraphs()
{
    for (int i = this->getGraphs().count()-1; i >=0; i--)
    {
        QString title = this->getGraphs().at(i)->getTitle();
        if (title.contains("Track")) {
            this->getGraphs().removeAt(i);
        }
    }
}

void EngineeringPlot::RecolorManualTrack(int track_id, QColor new_color)
{
    manual_track_colors[track_id] = new_color;
}

void EngineeringPlot::UpdateManualPlottingTrackFrames(std::vector<ManualPlottingTrackFrame> frames, const std::set<int>& track_ids)
{
    manual_track_frames = std::move(frames);
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

// Track backend operations.  Should we put in a separate class?

void EngineeringPlot::AddTrack(std::vector<double> x_values, std::vector<double> y_values, int track_id, size_t &columnX, size_t &columnY)
{
    QVector<double> X(x_values.begin(), x_values.end());
    QVector<double> Y(y_values.begin(), y_values.end());

    QString titleX = "Track " + QString::number(track_id) + " x";
    QString titleY = "Track " + QString::number(track_id) + " y";

    columnX=ds->addCopiedColumn(X, titleX);
    columnY=ds->addCopiedColumn(Y, titleY);
}

std::vector<size_t> &EngineeringPlot::DeleteTrack(int track_id)
{
    const int index_of_frameline_y_column = 3; // base zero, so 3 is the 4th column
    const int index_of_deleted_track_x_column = index_of_frameline_y_column + track_id * 2 -1;

    JKQTPDatastore *ds = getDatastore();
    JKQTPDatastore *ds2 = new JKQTPDatastore();

    std::vector<size_t> *new_column_indexes = new std::vector<size_t>();

    int column_count = ds->getColumnCount();
    int next_ds2_index = 0;

    for (int i = 0; i < column_count; i++)
    {
        QString column_name = ds->getColumnName(i);

        static QRegularExpression re("\\s(\\d+)\\s");

        if (!(re.match(column_name).hasMatch() && re.match(column_name).captured(1).toInt() == track_id)) {

            size_t new_column_index = ds2->addColumn(ds->getRows(i), ds->getColumnName(i));

            if (i > index_of_frameline_y_column && i != index_of_deleted_track_x_column && i != index_of_deleted_track_x_column+1) {
                new_column_indexes->push_back(new_column_index);
            }

            for (int j = 0; j < ds->getRows(i); j++) {
                ds2->set(next_ds2_index, j, ds->get(i,j));
            }
            next_ds2_index++;
        }
    }

    column_count = ds2->getColumnCount();
    ds->clear();

    for (int i = 0; i < column_count; i++)
    {
        QString column_name = ds2->getColumnName(i);
        ds->addColumn(ds2->getRows(i), column_name);
        for (int j = 0; j < ds2->getRows(i); j++)
        {
            ds->set(i, j, ds2->get(i,j));
        }
    }

    return *new_column_indexes;
}

void EngineeringPlot::LookupTrackColumnIndexes(int track_id,  size_t &columnX, size_t &columnY)
{
    QString titleX = "Track " + QString::number(track_id) + " x";
    QString titleY = "Track " + QString::number(track_id) + " y";

    QList<QString> names = ds->getColumnNames();

    columnX=(size_t)names.indexOf(titleX);
    columnY=(size_t)names.indexOf(titleY);
}

void EngineeringPlot::ReplaceTrack(std::vector<double> x, std::vector<double> y, int track_id)
{
    QList<QString> names = ds->getColumnNames();
    QString x_column_to_search_for = "Track " + QString::number(track_id) + " x";
    int col_index_found = -1;

    for (int j=0; j < ds->getColumnCount(); j++)
    {
        if (QString(names[j]) == x_column_to_search_for)
        {
            col_index_found = j;
        }
    }

    ds->resizeColumn(col_index_found, x.size());
    ds->resizeColumn(col_index_found+1, x.size());

    for (size_t row_index = 0; row_index < x.size(); ++row_index)
    {
        ds->set(col_index_found, row_index, x[row_index]);
        ds->set(col_index_found+1, row_index, y[row_index]);
    }
}

void EngineeringPlot::RestoreTrackGraphs(std::vector<size_t> &new_column_indexes)
{
    const int firstIndexAfterFramelineColummns = 4;

    for (int i = firstIndexAfterFramelineColummns; i < new_column_indexes.size(); i+=2)
    {
        static QRegularExpression re("\\s(\\d+)\\s");
        QString column_name = ds->getColumnNames()[i];
        int track_id = re.match(column_name).hasMatch() && re.match(column_name).captured(1).toInt();
        AddGraph(track_id, new_column_indexes[i], new_column_indexes[i+1]);
    }
}

bool EngineeringPlot::TrackExists(int track_id) const
{
    QString titleX = "Track " + QString::number(track_id) + " x";
    QList<QString> names = ds->getColumnNames();

    return names.contains(titleX);
}
