#include "workspace.h"

Workspace::Workspace() {
    QDir().mkdir(WORKSPACE_FOLDER);
};

Workspace::~Workspace() 
{
};

QStringList Workspace::get_workspace_names()
{
    return QDir(WORKSPACE_FOLDER).entryList(QStringList() << "*.json");
};

void Workspace::save_state(QString workspace_name, QString image_path, int start_frame, int end_frame, std::vector<processing_state> all_states, std::vector<annotation_info> annotations, std::vector<short> bad_pixel_map)
{
    //Inspiration: https://forum.qt.io/topic/65874/create-json-using-qjsondocument
    QJsonObject json_object;
    json_object.insert("image_path", image_path);
    json_object.insert("start_frame", start_frame);
    json_object.insert("end_frame", end_frame);

    QJsonArray states;
    for (auto state : all_states)
    {
        QJsonObject state_object = state.to_json();
        states.push_back(state_object);
    }
    json_object.insert("processing_states", states);

    QJsonArray annos;
    for (auto annotation : annotations)
    {
        annos.push_back(annotation.to_json());
    }
    json_object.insert("annotations", annos);

    if (bad_pixel_map.size() > 1)
    {
        QJsonArray indeces;
        for (auto i = 0; i < bad_pixel_map.size(); i++)
        {
            if (bad_pixel_map[i] == 1)
            {
                indeces.push_back(i);
            }
        }
        json_object.insert("bad_pixel_indeces", indeces);
    }

    QJsonDocument json_document(json_object);

    QFile file(QString(WORKSPACE_FOLDER) + "/" + workspace_name);
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    file.write(json_document.toJson());
    file.close();
};

WorkspaceValues Workspace::load_state(QString workspace_name) {
     //Inspiration: https://stackoverflow.com/questions/60723466/how-to-write-and-read-in-json-file-using-qt
    QFile file(QString(WORKSPACE_FOLDER) + "/" + workspace_name);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QString dataString=file.readAll();
    QJsonDocument doc= QJsonDocument::fromJson(dataString.toUtf8());
    QJsonObject data_object = doc.object();
    
    QString image_path = data_object.value("image_path").toString();
    int start_frame = data_object.value("start_frame").toInt();
    int end_frame = data_object.value("end_frame").toInt();

    std::vector<processing_state> states;
    for (auto json_obj : data_object.value("processing_states").toArray())
    {
        processing_state state = create_processing_state_from_json(json_obj.toObject());
        states.push_back(state);
    }

    std::vector<annotation_info> annotations;
    for (auto json_obj : data_object.value("annotations").toArray())
    {
        annotation_info anno = create_annotation_info_from_json(json_obj.toObject());
        annotations.push_back(anno);
    }

    std::vector<int> bad_pixels;
    for (auto json_obj : data_object.value("bad_pixel_indeces").toArray())
    {
        bad_pixels.push_back(json_obj.toInt());
    }

    file.close();
    return WorkspaceValues { image_path, start_frame, end_frame, states, annotations, bad_pixels };
};