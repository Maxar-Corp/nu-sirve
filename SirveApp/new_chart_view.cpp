#include "new_chart_view.h"

#include <QLineSeries>
#include <QRubberBand>
#include <QValueAxis>

// Generic plotting functions
NewChartView::NewChartView(QChart* chart)
    :QChartView(chart), rubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{
    newchart = chart;
    newchart->setBackgroundBrush(QBrush(QColor(200, 200, 200)));

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
    for (QAbstractAxis * abstract_axis : axes_list)
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
