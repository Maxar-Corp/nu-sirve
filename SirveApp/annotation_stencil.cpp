#include "annotation_stencil.h"
#include <QMouseEvent>
#include <QStyleOption>
#include <QPainter>
#include <QWidget>
#include <QFontMetrics>

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

        // Adjust the anno. position to offset the shift imposed by the zoom correction code:
        QPoint offset = QPoint(this->width() * .005, this->height() * .75);
        emit mouseReleased(event->globalPos() - _drag_position + offset);
    }
}

void AnnotationStencil::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::yellow, 2));
    QFont font("Arial", current_data->font_size);

    font.setPointSize(current_data->font_size);
    painter.setFont(font);

    painter.drawText(rect(), Qt::AlignCenter, current_data->text);
}

void AnnotationStencil::InitializeData(AnnotationInfo data)
{
    current_data->color = data.color;
    current_data->font_size = data.font_size;
    current_data->text = data.text;

    // Calculate the size needed to display the text and set accordingly
    QFont font("Arial", current_data->font_size);
    QFontMetrics fontMetrics(font);
    int textWidth = fontMetrics.horizontalAdvance(current_data->text);
    int textHeight = fontMetrics.height();

    setFixedSize(textWidth, textHeight);
    setFont(font);
}
