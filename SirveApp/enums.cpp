#include "enums.h"
#include <QMetaEnum>
#include <stdexcept>

Enums::PlotType Enums::getPlotTypeByIndex(int index) {
    if (index < 0 || index > static_cast<int>(PlotType::Seconds_Past_Midnight)) {
        throw std::out_of_range("Index out of range for PlotType enum");
    }
    return static_cast<PlotType>(index);
}

Enums::PlotUnit Enums::getPlotUnitByIndex(int index) {
    if (index < 0 || index > static_cast<int>(PlotUnit::None)) {
        throw std::out_of_range("Index out of range for PlotUnit enum");
    }
    return static_cast<PlotUnit>(index);
}

Enums::ClassificationType Enums::getClassificationTypeByIndex(int index) {
    if (index < 0 || index > static_cast<int>(ClassificationType::VideoDisplay)) {
        throw std::out_of_range("Index out of range for ClassificationType enum");
    }
    return static_cast<ClassificationType>(index);
}

QString Enums::plotTypeToString(PlotType plotType) {
    const QMetaObject &metaObj = Enums::staticMetaObject;
    int index = metaObj.indexOfEnumerator("PlotType");
    QMetaEnum metaEnum = metaObj.enumerator(index);
    return metaEnum.valueToKey(plotType);
}

QString Enums::plotUnitToString(PlotUnit plotUnit) {
    const QMetaObject &metaObj = Enums::staticMetaObject;
    int index = metaObj.indexOfEnumerator("PlotUnit");
    QMetaEnum metaEnum = metaObj.enumerator(index);
    return metaEnum.valueToKey(plotUnit);
}

int Enums::getPlotTypeIndexFromString(const QString &value) {
    const QMetaObject &metaObj = Enums::staticMetaObject;
    int index = metaObj.indexOfEnumerator("PlotType");
    QMetaEnum metaEnum = metaObj.enumerator(index);
    int enumValue = metaEnum.keyToValue(value.toUtf8().constData());
    if (enumValue == -1) {
        throw std::invalid_argument(QString("Invalid PlotType value: %1").arg(value).toStdString());
    }
    return enumValue;
}

int Enums::getPlotUnitIndexFromString(const QString &value) {
    const QMetaObject &metaObj = Enums::staticMetaObject;
    int index = metaObj.indexOfEnumerator("PlotUnit");
    QMetaEnum metaEnum = metaObj.enumerator(index);
    int enumValue = metaEnum.keyToValue(value.toUtf8().constData());
    if (enumValue == -1) {
        throw std::invalid_argument(QString("Invalid PlotUnit value: %1").arg(value).toStdString());
    }
    return enumValue;
}

int Enums::getClassificationTypeIndexFromString(const QString &value) {
    const QMetaObject &metaObj = Enums::staticMetaObject;
    int index = metaObj.indexOfEnumerator("ClassificationType");
    QMetaEnum metaEnum = metaObj.enumerator(index);
    int enumValue = metaEnum.keyToValue(value.toUtf8().constData());
    if (enumValue == -1) {
        throw std::invalid_argument(QString("Invalid ClassificationType value: %1").arg(value).toStdString());
    }
    return enumValue;
}
