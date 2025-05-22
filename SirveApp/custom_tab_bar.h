#ifndef CUSTOM_TAB_BAR_H
#define CUSTOM_TAB_BAR_H


#include <QTabBar>

class CustomTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit CustomTabBar(QWidget* parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
};

#endif // CUSTOMTABBAR_H
