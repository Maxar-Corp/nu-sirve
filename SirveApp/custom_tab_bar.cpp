#include "custom_tab_bar.h"
#include <QToolTip>
#include <QMouseEvent>

CustomTabBar::CustomTabBar(QWidget* parent)
    : QTabBar(parent)
{
    setMouseTracking(true);  // Enable mouse tracking to receive mouse move events without clicking
}

void CustomTabBar::mouseMoveEvent(QMouseEvent* event)
{
    QString tooltipText = "Double right-click in right-side empty area to add new tab!";
    QToolTip::showText(event->globalPos(), tooltipText, this);

    QTabBar::mouseMoveEvent(event);
}

void CustomTabBar::leaveEvent(QEvent* event)
{
    QToolTip::hideText();
    QTabBar::leaveEvent(event);
}
