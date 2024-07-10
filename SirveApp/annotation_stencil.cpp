#include "annotation_stencil.h"
#include <QMouseEvent>
#include <QStyleOption>
#include <QPainter>
#include <QWidget>

AnnotationStencil::AnnotationStencil(QWidget *parent)
    : QWidget(parent), _drag_active(true)
{
    setFixedSize(100,20);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    current_data = new AnnotationInfo();
    current_data->text = "";
}

AnnotationStencil::~AnnotationStencil()
{

}

void AnnotationStencil::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        _drag_active = true;
        _drag_position = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void AnnotationStencil::mouseMoveEvent(QMouseEvent *event)
{
    if (_drag_active)
    {
        move(event->globalPos() - _drag_position);
        event->accept();
    }
}

void AnnotationStencil::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        _drag_active = false;
        event->accept();
    }

    if (event->button() == Qt::RightButton)
    {
        // TO DO: show dialog to confirm annotation positioning in the video display.

        // If YES:
        // 1. Update the X,Y position in current_data.
        // 2. Hide the annotation stencil.
        //
        // If NO:
        // 1. Hide the annotation stencil.
    }
}

void AnnotationStencil::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::blue, 2));
    painter.drawRect(0, 0, width() - 1, height() - 1);
    painter.drawText(rect(), Qt::AlignCenter, current_data->text);
}

void AnnotationStencil::UpdateText(QString text)
{
    current_data->text = text;
}

void AnnotationStencil::AnnotationPositioned()
{

}
