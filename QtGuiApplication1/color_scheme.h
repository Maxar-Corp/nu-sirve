#pragma once

#ifndef COLOR_SCHEME_H
#define COLOR_SCHEME_H

#include <QString>
#include <vector>

class ColorScheme
{

public:

    ColorScheme();
    QString GetCurrentColor();
    QString GetNextColor();
    QString GetPreviousColor();

private:
    uint index;
    uint num_colors;
    std::vector<QString> colors;

};


#endif // COLOR_SCHEME_H
