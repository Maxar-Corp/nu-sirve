#include "color_scheme.h"

ColorScheme::ColorScheme()
{
    colors.push_back(QString("#ff4e79a7")); //blue
    colors.push_back(QString("#fff28e2b")); //orange
    colors.push_back(QString("#ffe15759")); //red
    colors.push_back(QString("#ff76b7b2")); //aqua
    colors.push_back(QString("#ff59a14f")); //green
    colors.push_back(QString("#ffedc948")); //yellow
    colors.push_back(QString("#ffb07aa1")); //purple
    colors.push_back(QString("#ffff9da7")); //pink
    colors.push_back(QString("#ff9c755f")); //brown
    colors.push_back(QString("#ffbab0ac")); //grey

    index = 0;
}

QStringList ColorScheme::get_track_colors()
{
    return {
        "red",
        "orange",
        "yellow",
        "green",
        "blue",
        "violet",
        "black",
        "white",
    };
}

QString ColorScheme::get_current_color(){
    return colors[index];
}

QString ColorScheme::get_color(int index) {
	return colors[index];
}

QString ColorScheme::get_next_color()
{
    index++;
    if (index == colors.size())
    {
        index = 0;
    }

    return colors[index];
}

void ColorScheme::ResetColors()
{
	index = 0;
}
