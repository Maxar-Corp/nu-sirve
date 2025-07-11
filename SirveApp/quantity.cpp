#include "quantity.h"

Quantity::Quantity() {}

Quantity::Quantity(QString name, Enums::PlotUnit unit)
    : plotName(name), plotUnit(unit) {}

QString Quantity::getName() const  {
    return plotName;
}

Enums::PlotUnit Quantity::getUnit() const  {
    return plotUnit;
}

QString plotUnitToScientificLabel(Enums::PlotUnit unit) {
    switch (unit) {
        case Enums::PlotUnit::Counts:             return "Counts";
        case Enums::PlotUnit::Degrees:            return "Degrees";
        case Enums::PlotUnit::FrameNumber:        return "";
        case Enums::PlotUnit::Microns:            return "μm";
        case Enums::PlotUnit::Radians:            return "Radians";
        case Enums::PlotUnit::Seconds:            return "s";
        case Enums::PlotUnit::W_m2_str:           return "W / m^{2} \\cdot sr";
        case Enums::PlotUnit::Undefined_PlotUnit: return "Undefined";
        default:                           return "Unknown";
    }
}

QString Quantity::getFormattedUnit() const {
    return plotUnitToScientificLabel(plotUnit);
}

QString Quantity::getFullLabel() const {
    return QString("%1 [%2]").arg(getName(), getFormattedUnit());
}

