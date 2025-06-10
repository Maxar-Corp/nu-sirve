
#ifndef QUANTITY_H
#define QUANTITY_H

#include <QString>
#include "enums.h"

class Quantity
{
public:
    Quantity();
    Quantity(QString name, Enums::PlotUnit unit);

    QString getName() const;
    Enums::PlotUnit getUnit() const;

private:

    QString plotName;
    Enums::PlotUnit plotUnit;
};

#endif // QUANTITY_H
