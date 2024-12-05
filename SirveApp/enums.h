#ifndef ENUMS_H
#define ENUMS_H


#include <stdexcept>
#include <QObject>
#include <QMetaEnum>

class Enums : public QObject
{
    Q_OBJECT
public:

    enum PlotType {
        Azimuth,    // 0
        Elevation,  // 1
        Irradiance  // 2
    };

    enum PlotUnit {
        Frames,
        Seconds_Past_Midnight,
        Seconds_From_Epoch
    };

    // Function to get an enum by index
    static PlotType getPlotTypeByIndex(int index) {
        if (index < 0 || index >= static_cast<int>(PlotType::Irradiance) + 1) {
            throw std::out_of_range("Index out of range for PlotType enum");
        }
        return static_cast<PlotType>(index);
    }

    // Converts PlotType enum to QString
    static QString plotTypeToString(PlotType plotType) {
        const QMetaObject &metaObj = Enums::staticMetaObject;
        int index = metaObj.indexOfEnumerator("PlotType");
        QMetaEnum metaEnum = metaObj.enumerator(index);
        return metaEnum.valueToKey(plotType);
    }

    // Function to get the zero-based index from the enum value string
    static int getPlotTypeIndexFromString(const QString &value) {
        const QMetaObject &metaObj = Enums::staticMetaObject;
        int index = metaObj.indexOfEnumerator("PlotType");
        QMetaEnum metaEnum = metaObj.enumerator(index);

        int enumValue = metaEnum.keyToValue(value.toUtf8().constData());
        if (enumValue == -1) {
            throw std::invalid_argument(QString("Invalid PlotType value: %1").arg(value).toStdString());
        }
        return enumValue; // Since enums are zero-based by definition, this is also the zero-based index.
    }

    // Function to get an enum by index
    static PlotUnit getPlotUnitByIndex(int index) {
        if (index < 0 || index >= static_cast<int>(PlotUnit::Seconds_From_Epoch) + 1) {
            throw std::out_of_range("Index out of range for PlotType enum");
        }
        return static_cast<PlotUnit>(index);
    }

    // Converts PlotUnit enum to QString
    static QString plotUnitToString(PlotUnit plotUnit) {
        const QMetaObject &metaObj = Enums::staticMetaObject;
        int index = metaObj.indexOfEnumerator("PlotUnit");
        QMetaEnum metaEnum = metaObj.enumerator(index);
        return metaEnum.valueToKey(plotUnit);
    }

    // Function to get the zero-based index from the enum value string
    static int getPlotUnitIndexFromString(const QString &value) {
        const QMetaObject &metaObj = Enums::staticMetaObject;
        int index = metaObj.indexOfEnumerator("PlotUnit");
        QMetaEnum metaEnum = metaObj.enumerator(index);

        int enumValue = metaEnum.keyToValue(value.toUtf8().constData());
        if (enumValue == -1) {
            throw std::invalid_argument(QString("Invalid PlotUnit value: %1").arg(value).toStdString());
        }
        return enumValue; // Since enums are zero-based by definition, this is also the zero-based index.
    }

 Q_ENUM(PlotType)
 Q_ENUM(PlotUnit)
};

#endif // ENUMS_H
