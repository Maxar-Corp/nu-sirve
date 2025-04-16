#ifndef NEW_CHART_VIEW_H
#define NEW_CHART_VIEW_H

#include <QtCharts/QChartView>

struct ChartState
{
    qreal xMin;
    qreal xMax;
    qreal yMin;
    qreal yMax;

    float scale_factor_maxx;
    float scale_factor_minx;

    float scale_factor_maxy;
    float scale_factor_miny;
};


class NewChartView : public QChartView {

    Q_OBJECT
private:
    bool is_frameline_moving;
    ChartState savedChartState;

public:
    NewChartView(QChart *chart);
    void clearSeriesByName(const QString &seriesName);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void apply_nice_numbers();

    QChart *new_chart;
    bool is_zoomed;
    ChartState get_chart_state();
    void set_chart_state(ChartState chartState);

signals:
    void rubberBandChanged(const QRect &rect);
    void updatePlots();
    void updateFrameLine();

public slots:
    void UpdateChartFramelineStatus(bool status);

private:
    QRubberBand *rubberBand;
    QPoint origin;
};

#endif // NEW_CHART_VIEW_H
