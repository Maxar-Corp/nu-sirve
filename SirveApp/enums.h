#ifndef ENUMS_H
#define ENUMS_H

#include <stdexcept>
#include <QObject>
#include <QMetaEnum>

class Enums : public QObject
{
    Q_OBJECT
public:

    enum ClassificationType {
        Plot,
        VideoDisplay
    };

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
    static ClassificationType getClassificationTypeByIndex(int index) {
        if (index < 0 || index >= static_cast<int>(ClassificationType::VideoDisplay) + 1) {
            throw std::out_of_range("Index out of range for ClassificationType enum");
        }
        return static_cast<ClassificationType>(index);
    }

    // Function to get the zero-based index from the enum value string
    static int getClassificationTypeIndexFromString(const QString &value) {
        const QMetaObject &metaObj = Enums::staticMetaObject;
        int index = metaObj.indexOfEnumerator("ClassificationType");
        QMetaEnum metaEnum = metaObj.enumerator(index);

        int enumValue = metaEnum.keyToValue(value.toUtf8().constData());
        if (enumValue == -1) {
            throw std::invalid_argument(QString("Invalid ClassificationType value: %1").arg(value).toStdString());
        }
        return enumValue; // Since enums are zero-based by definition, this is also the zero-based index.
    }

    Q_ENUM(PlotType)
    Q_ENUM(PlotUnit)
};

#endif // ENUMS_H

