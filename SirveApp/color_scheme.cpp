#include "color_scheme.h"

ColorScheme::ColorScheme(){

    colors.push_back(QString("#4e79a7")); //blue
    colors.push_back(QString("#f28e2b")); //orange
    colors.push_back(QString("#e15759")); //red
    colors.push_back(QString("#76b7b2")); //aqua
    colors.push_back(QString("#59a14f")); //green
    colors.push_back(QString("#edc948")); //yellow
    colors.push_back(QString("#b07aa1")); //purple
    colors.push_back(QString("#ff9da7")); //pink
    colors.push_back(QString("#9c755f")); //brown
    colors.push_back(QString("#bab0ac")); //grey

    index = 0;
    num_colors = colors.size();
}

QString ColorScheme::GetCurrentColor(){
    return colors[index];
}

QString ColorScheme::Get_Color(int index) {
	return colors[index];
}

QString ColorScheme::GetNextColor(){

    index++;
    if (index == num_colors){
        index = 0;
    }

    return colors[index];
}

void ColorScheme::reset_colors()
{
	index = 0;
}
