#include "osm_plotter.h"


OSMPlotter::OSMPlotter(QWidget *parent): QChartView(parent){

    chart = new QChart();

	chart_view = new QChartView(chart);
	chart_view->setRubberBand(QChartView::RectangleRubberBand);

    max_frames = 1;
    min_frames = 0;
    max_radiance = 1;
    min_radiance = 0;

    max_num_ticks = 10;
    min_num_ticks = 3;

}

OSMPlotter::~OSMPlotter(){

	delete chart;
}

void OSMPlotter::Add_Scatter_Series(char* series_name, std::vector<double> x, std::vector<double> y,double marker_size){

    QScatterSeries *series = new QScatterSeries();
    series->setName(QString(series_name));
    series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    series->setMarkerSize(marker_size);

    uint num_data_pts = x.size();

    for(uint i = 0; i < num_data_pts; i++){
        series->append(x[i], y[i]);
    }

    chart->addSeries(series);

}

void OSMPlotter::Add_Line_Series(const char* series_name, std::vector<unsigned int> x, std::vector<double> y){

    QLineSeries *series = new QLineSeries();
    series->setName(QString(series_name));

    QColor base_color(colors.GetCurrentColor());
    series->setColor(base_color);

    uint num_data_pts = x.size();
    int num_breaks = 0;

    for(uint i = 0; i < num_data_pts; i++){
        if(i == 0){ //If first pt in series then continue...
            series->append(x[i], y[i]);

        }
        else if (x[i] - x[i - 1] > 1.1) { //if current point is greater than 1 frame away then start new series...
            chart->addSeries(series);

            if (num_breaks > 0)
                Remove_Series_Legend();
            num_breaks++;

            series = new QLineSeries();
            series->setColor(base_color);
            series->append(x[i], y[i]);
        }
        else { //if continuation of current series...
            series->append(x[i], y[i]);

            if(x[i] > max_frames){
                Set_Max_Frames(x[i]);
            }
            if(y[i] > max_radiance){
                Set_Max_Radiance(y[i]);
            }
        }
    }

    colors.GetNextColor();
    chart->addSeries(series);
    if (num_breaks > 0)
        Remove_Series_Legend();
}

void OSMPlotter::Remove_Series_Legend(){
    int num_legend_entries = chart->legend()->markers().size();
    if (num_legend_entries > 1){
        QLegendMarker *last_entry = chart->legend()->markers().last();
        last_entry->setVisible(false);
    }

}

void OSMPlotter::Add_Attributes(char* chart_title, bool display_legend){

    chart->setTitle(QString(chart_title));

    SetAxes();

    chart->setDropShadowEnabled(false);

    chart->legend()->setVisible(false);
    //chart->legend()->setAlignment(Qt::AlignRight);
    //chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

}

void OSMPlotter::SetAxes(){

    chart->createDefaultAxes();
    QAbstractAxis *x_axis = chart->axes(Qt::Horizontal)[0];
    QAbstractAxis *y_axis = chart->axes(Qt::Vertical)[0];

    QString x_label_title("Frame #");
    QString y_label_title("Radiance");

    double tick_spacing_frames = FindTickSpacing(max_frames);
    double max_limit_frame = std::floor(max_frames / tick_spacing_frames) * tick_spacing_frames + 0.5 * tick_spacing_frames;

    double tick_spacing_radiance = FindTickSpacing(max_radiance);
    double max_limit_radiance = std::floor(max_radiance / tick_spacing_radiance) * tick_spacing_radiance + 0.5 * tick_spacing_radiance;

    x_axis->setMin(min_frames);
    x_axis->setMax(max_limit_frame);
    x_axis->setTitleText(x_label_title);

    y_axis->setMin(min_radiance);
    y_axis->setMax(max_limit_radiance);
    y_axis->setLabelsVisible(false);
    //y_axis->setTitleText(y_label_title);
}

double OSMPlotter::FindTickSpacing(double value){
    double min_spacing = value / max_num_ticks;
    double magnitude_spacing = std::pow(10, std::floor(std::log10(min_spacing)));
    double residual = min_spacing / magnitude_spacing;

    double tick_spacing;
    if(residual > 5){
        tick_spacing = magnitude_spacing * 10;
    }
    else if(residual > 2){
        tick_spacing = magnitude_spacing * 5;
    }
    else if (residual > 1) {
        tick_spacing = magnitude_spacing * 2;
    }
    else {
        tick_spacing = magnitude_spacing * 1;
    }

    double num_ticks = std::floor(value / tick_spacing);
    if(num_ticks < min_num_ticks){
        tick_spacing = tick_spacing / 2;
    }

    return tick_spacing;
}

void OSMPlotter::Set_Max_Frames(unsigned int value){
    max_frames = value;
}

void OSMPlotter::Set_Max_Radiance(double value){
    max_radiance = value;
}

