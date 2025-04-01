#include "classification.h"

Classification CreateClassificationFromJson(const QJsonObject & json_obj)
{
    Classification temp{};
    temp.text = json_obj.value("text").toString();
    temp.type = json_obj.value("type").toString();

    return temp;
}

// Ensure only one of each classification type exists on the classification list before we write out the workspace file.
bool UpdateClassificationIfExists(const QString type, const QString text, std::vector<Classification> *classification_list)
{
    bool object_exists = false;

    for (auto it = classification_list->begin(); it != classification_list->end(); it++) {
        if (it->type == type) {
            it->text = text;
            object_exists = true;
        }
    }

    return object_exists;
}
