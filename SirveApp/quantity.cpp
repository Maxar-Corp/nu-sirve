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
