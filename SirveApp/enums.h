#ifndef ENUMS_H
#define ENUMS_H

#include <QObject>
#include <QString>

class Enums : public QObject
{
    Q_OBJECT
public:
    enum PlotType {
        Azimuth,
        Boresight_Azimuth,
        Boresight_Elevation,
        Elevation,
        FovX,
        FovY,
        SumCounts,
        Frames,
        Seconds_From_Epoch,
        Seconds_Past_Midnight,
        Undefined_PlotType
    };

    enum PlotUnit {
        Degrees,
        Radians,
        Undefined_PlotUnit
    };

    enum ClassificationType {
        Plot,
        VideoDisplay,
        Undefined_ClassificationType
    };

    static PlotType getPlotTypeByIndex(int index);
    static PlotUnit getPlotUnitByIndex(int index);
    static ClassificationType getClassificationTypeByIndex(int index);

    static QString plotTypeToString(PlotType plotType);
    static QString plotUnitToString(PlotUnit plotUnit);

    static int getPlotTypeIndexFromString(const QString &value);
    static int getPlotUnitIndexFromString(const QString &value);
    static int getClassificationTypeIndexFromString(const QString &value);

    Q_ENUM(PlotType)
    Q_ENUM(PlotUnit)
    Q_ENUM(ClassificationType)
};

#endif // ENUMS_H
