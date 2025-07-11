
#ifndef QUANTITY_H
#define QUANTITY_H

#include <QString>
#include "enums.h"  // where PlotUnit is declared

class Quantity {
public:
    Quantity();
    Quantity(QString name, Enums::PlotUnit unit);

    QString getName() const;
    Enums::PlotUnit getUnit() const;

    QString getFormattedUnit() const;
    QString getFullLabel() const;

private:
    QString plotName;
    Enums::PlotUnit plotUnit;
};
#endif