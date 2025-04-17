#include "color_scheme.h"
#include <qmap.h>

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
        "green",
        "blue",
        "cyan",
        "magenta",
        "orange",
        "yellow",
        "gray",
        "violet",
        "black",
        "white",
        "darkRed",
        "darkGreen",
        "darkBlue",
        "darkCyan",
        "darkMagenta",
        "darkYellow",
        "darkGray",
        "lightGray"
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

QMap<QString, QString> ColorScheme::cursorColors = {
    {"auto detect", ""},
    {"black", ":icons/crosshair-black.png"},
    {"blue", ":icons/crosshair-blue.png"},
    {"cyan", ":icons/crosshair-cyan.png"},
    {"green", ":icons/crosshair-green.png"},
    {"magenta", ":icons/crosshair-magenta.png"},
    {"orange", ":icons/crosshair-orange.png"},
    {"red", ":icons/crosshair-red.png"},
    {"violet", ":icons/crosshair-violet.png"},
    {"white", ":icons/crosshair-white.png"},
    {"yellow", ":icons/crosshair-golden.png"},
};

QString ColorScheme::get_cursor_icon_path(QString &color)
{
    return cursorColors.value(color, ":icons/crosshair-golden.png");
}
