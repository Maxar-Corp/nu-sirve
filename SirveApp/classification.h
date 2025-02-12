#pragma once

#include "qjsonobject.h"
#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

class Classification
{
    public: QString text;
    public: QString type;

    Classification() {}

    // Constructor
    Classification(const QString& text, const QString& type)
        : text(text), type(type) {}

    QJsonObject to_json()
    {
        QJsonObject classification_object;
        classification_object.insert("text", text);
        classification_object.insert("type", type);

        return classification_object;
    }
};

Classification CreateClassificationFromJson(const QJsonObject & json_obj);

bool UpdateClassificationIfExists(const QString type, const QString text, std::vector<Classification> *classification_list);

#endif // CLASSIFICATION_H
