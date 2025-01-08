#include "workspace.h"

Workspace::Workspace(QString workspace_folder)
{
    QDir().mkdir(workspace_folder);
};

Workspace::~Workspace() 
{
};

QStringList Workspace::get_workspace_names(QString workspace_folder)
{
    return QDir(workspace_folder).entryList(QStringList() << "*.json");
};

void Workspace::SaveState(QString full_workspace_file_path, QString image_path, int start_frame, int end_frame, std::vector<processingState> all_states, std::vector<AnnotationInfo> annotations, std::vector<Classification> classifications)
{
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

    QJsonArray classos;
    for (auto classification: classifications)
    {
        classos.push_back(classification.to_json());
    }

    json_object.insert("annotations", annos);
    json_object.insert("classifications", classos);

    QJsonDocument json_document(json_object);

    QFile file((QString(full_workspace_file_path)));
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    file.write(json_document.toJson());
    file.close();
};

WorkspaceValues Workspace::LoadState(QString workspace_name) {
     //Inspiration: https://stackoverflow.com/questions/60723466/how-to-write-and-read-in-json-file-using-qt
    QFile file(workspace_name);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QString dataString=file.readAll();
    QJsonDocument doc= QJsonDocument::fromJson(dataString.toUtf8());
    QJsonObject data_object = doc.object();
    
    QString image_path = data_object.value("image_path").toString();
    int start_frame = data_object.value("start_frame").toInt();
    int end_frame = data_object.value("end_frame").toInt();

    std::vector<processingState> states;
    for (auto json_obj : data_object.value("processing_states").toArray())
    {
        processingState state = create_processing_state_from_json(json_obj.toObject());
        states.push_back(state);
    }

    std::vector<AnnotationInfo> annotations;
    for (auto json_obj : data_object.value("annotations").toArray())
    {
        AnnotationInfo anno = CreateAnnotationInfoFromJson(json_obj.toObject());
        annotations.push_back(anno);
    }

    std::vector<Classification> classifications;
    for (auto json_obj : data_object.value("classifications").toArray())
    {
        Classification classification = CreateClassificationFromJson(json_obj.toObject());
        classifications.push_back(classification);
    }

    file.close();
    return WorkspaceValues { image_path, start_frame, end_frame, states, annotations };
};
