#include "classification.h"

Classification CreateClassificationFromJson(const QJsonObject & json_obj)
{
    Classification temp = {};
    temp.text = json_obj.value("text").toString();

    return temp;
}
