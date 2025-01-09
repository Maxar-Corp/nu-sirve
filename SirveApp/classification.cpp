#include "classification.h"

Classification CreateClassificationFromJson(const QJsonObject & json_obj)
{
    Classification temp{};
    temp.text = json_obj.value("text").toString();
    temp.type = json_obj.value("type").toString();

    return temp;
}

// Ensure that only one of each classification type exists on the classification list before we write out the workspace file.
void DeleteClassificationIfExists(const QString type, std::vector<Classification> *classification_list)
{
    for (const auto& element : *classification_list) {
        qDebug() << "found an element of type " << element.type;

        // Dereference the pointer and call erase
        classification_list->erase(
            std::remove_if(classification_list->begin(), classification_list->end(), [&type](const Classification& c) {
                return c.type == type;
            }),
            classification_list->end()
            );

    }
}
