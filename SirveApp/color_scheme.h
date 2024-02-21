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
    QString GetCurrentColor();
	QString Get_Color(int index);
    QString GetNextColor();
	void reset_colors();

    static QStringList GetTrackColors();

private:
    uint index;
    std::vector<QString> colors;
};


#endif // COLOR_SCHEME_H
