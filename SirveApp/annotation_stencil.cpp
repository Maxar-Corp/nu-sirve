#include "annotation_stencil.h"
#include <QMouseEvent>
#include <QStyleOption>
#include <QPainter>
#include <QWidget>

AnnotationStencil::AnnotationStencil(QWidget *parent)
    : QWidget(parent), _drag_active(true)
{
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
        emit mouseMoved(event->globalPos() - _drag_position);
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
    painter.setPen(QPen(Qt::yellow, 2));
    painter.drawRect(0, 0, width() - 1, height() - 1);
    painter.drawText(rect(), Qt::AlignCenter, current_data->text);
}

void AnnotationStencil::InitializeData(AnnotationInfo data)
{
    const int data_height = 20;
    current_data->color = data.color;
    current_data->font_size = data.font_size;
    current_data->text = data.text;
    int total_width = data.text.length() * data.font_size;
    setFixedSize(total_width, data_height);
}


void AnnotationStencil::AnnotationPositioned()
{

}
