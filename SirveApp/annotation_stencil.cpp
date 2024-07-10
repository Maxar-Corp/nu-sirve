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
}

void AnnotationStencil::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::red, 2));
    painter.drawRect(0, 0, width() - 1, height() - 1);

    //if (current_data != NULL)
    //    painter.drawText(rect(), Qt::AlignCenter, current_data->text);
}

void AnnotationStencil::UpdateText(QString text)
{
    current_data->text = text;
}

void AnnotationStencil::AnnotationPositioned()
{

}
