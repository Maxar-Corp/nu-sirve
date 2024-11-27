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
        azimuth,    // 0
        elevation,  // 1
        irradiance  // 2
    };

    // Function to get an enum by index
    static PlotType getPlotTypeByIndex(int index) {
        if (index < 0 || index >= static_cast<int>(PlotType::irradiance) + 1) {
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

 Q_ENUM(PlotType)
};

#endif // ENUMS_H
