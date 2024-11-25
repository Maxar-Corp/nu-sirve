#ifndef PLOT_TYPES_H
#define PLOT_TYPES_H


#include <stdexcept>
class Enums
{
public:

    enum PlotTypes {
        azimuth,    // 0
        elevation,  // 1
        irradiance  // 2
    };

    // Function to get an enum by index
    static PlotTypes getPlotTypeByIndex(int index) {
        if (index < 0 || index >= static_cast<int>(PlotTypes::irradiance) + 1) {
            throw std::out_of_range("Index out of range for PlotType enum");
        }
        return static_cast<PlotTypes>(index);
    }
};

#endif // PLOT_TYPES_H
