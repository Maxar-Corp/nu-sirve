#include "workspace.h"

#include "state_manager.h"

Workspace::Workspace(QString workspace_folder)
{
    QDir().mkdir(workspace_folder);
};

Workspace::~Workspace() 
{
};

QStringList Workspace::get_workspace_names(const QString& workspace_folder)
{
    return QDir(workspace_folder).entryList(QStringList() << "*.json");
};

void Workspace::SaveState(const QString& full_workspace_file_path, const QString& image_path, int start_frame, int end_frame,
    double timing_offset, const StateManager& all_states, const std::vector<AnnotationInfo>& annotations,
    const std::vector<Classification>& classifications)
{
    QJsonObject json_object;
    json_object.insert("image_path", image_path);
    json_object.insert("start_frame", start_frame);
    json_object.insert("end_frame", end_frame);
    json_object.insert("timing_offset", timing_offset);

    QJsonArray states;
    for (auto state : all_states)
    {
        states.push_back(state.ToJson());
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

    QFile file(full_workspace_file_path);
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    file.write(json_document.toJson());
    file.close();
};

WorkspaceValues Workspace::LoadState(const QString& workspace_name) {
     //Inspiration: https://stackoverflow.com/questions/60723466/how-to-write-and-read-in-json-file-using-qt
    QFile file(workspace_name);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QString dataString=file.readAll();
    QJsonDocument doc= QJsonDocument::fromJson(dataString.toUtf8());
    QJsonObject data_object = doc.object();
    
    QString image_path = data_object.value("image_path").toString();
    int start_frame = data_object.value("start_frame").toInt();
    int end_frame = data_object.value("end_frame").toInt();
    double timing_offset = data_object.value("timing_offset").toDouble();

    std::vector<ProcessingState> states;
    for (auto json_obj : data_object.value("processing_states").toArray())
    {
        ProcessingState state = ProcessingState::FromJson(json_obj.toObject());
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
    return WorkspaceValues { image_path, start_frame, end_frame, timing_offset, states, annotations, classifications };
};
