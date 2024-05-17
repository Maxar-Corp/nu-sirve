#pragma once

#ifndef COLOR_SCHEME_H
#define COLOR_SCHEME_H

#include <QString>
#include <QStringList>
#include <vector>

class ColorScheme
{
public:
    ColorScheme();

    void ResetColors();

    QString get_current_color();
	QString get_color(int index);
    QString get_next_color();

    static QStringList get_track_colors();

private:
    uint index;
    std::vector<QString> colors;
};

#endif // COLOR_SCHEME_H
