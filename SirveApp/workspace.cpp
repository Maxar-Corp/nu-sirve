#include "workspace.h"

Workspace::Workspace() {
    QDir().mkdir(WORKSPACE_FOLDER);
};

Workspace::~Workspace() 
{
};

void Workspace::save_state(QString image_path, int start_frame, int end_frame, std::vector<processing_state> all_states) {
    //Inspiration: https://forum.qt.io/topic/65874/create-json-using-qjsondocument
    QJsonObject json_object;
    json_object.insert("image_path", image_path);
    json_object.insert("start_frame", start_frame);
    json_object.insert("end_frame", end_frame);

    QJsonArray states;
    for (auto state : all_states)
    {
        QJsonObject state_object = state_to_json(state);
        states.push_back(state_object);
    }
    json_object.insert("processing_states", states);

    QJsonDocument json_document(json_object);

    QFile file(QString(WORKSPACE_FOLDER) + "/" + QString(WORKSPACE_FILE));
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    file.write(json_document.toJson());
    file.close();
};

QJsonObject Workspace::state_to_json(const processing_state & state)
{
    QJsonObject state_object;
    switch (state.method)
    {
        case Processing_Method::original:
            state_object.insert("method", "Original");
            break;
        case Processing_Method::background_subtraction:
            state_object.insert("method", "Background Subtraction");
            state_object.insert("bgs_relative_start_frame", state.bgs_relative_start_frame);
            state_object.insert("bgs_num_frames", state.bgs_num_frames);
            break;
        case Processing_Method::deinterlace:
            state_object.insert("method", "Deinterlace");
            state_object.insert("deint_type", QString::number(static_cast<int>(state.deint_type)));
            break;
        case Processing_Method::non_uniformity_correction:
            state_object.insert("method", "NUC");
            state_object.insert("nuc_start_frame", state.nuc_start_frame);
            state_object.insert("nuc_stop_frame", state.nuc_stop_frame);
            state_object.insert("nuc_file_path", state.nuc_file_path);
            break;
        default:
            state_object.insert("method", "error");
    }
    return state_object;
};

WorkspaceValues Workspace::load_state() {
     //Inspiration: https://stackoverflow.com/questions/60723466/how-to-write-and-read-in-json-file-using-qt
    QFile file(QString(WORKSPACE_FOLDER) + "/" + QString(WORKSPACE_FILE));
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
        processing_state state = json_to_state(json_obj.toObject());
        states.push_back(state);
    }

    file.close();
    return WorkspaceValues { image_path, start_frame, end_frame, states };
};

processing_state Workspace::json_to_state(const QJsonObject & json_obj)
{
    QString method = json_obj.value("method").toString();

    if (method == "Original")
    {
        return processing_state { Processing_Method::original };
    }
    if (method == "Background Subtraction")
    {
        processing_state temp = { Processing_Method::background_subtraction };
        temp.bgs_relative_start_frame = json_obj.value("bgs_relative_start_frame").toInt();
        temp.bgs_num_frames = json_obj.value("bgs_num_frames").toInt();
        return temp;
    }
    if (method == "Deinterlace")
    {
        processing_state temp = { Processing_Method::deinterlace };
        temp.deint_type = static_cast<deinterlace_type>(json_obj.value("deint_type").toInt());
        return temp;
    }
    if (method == "NUC")
    {
        processing_state temp = { Processing_Method::non_uniformity_correction };
        temp.nuc_start_frame = json_obj.value("nuc_start_frame").toInt();
        temp.nuc_stop_frame = json_obj.value("nuc_stop_frame").toInt();
        temp.nuc_file_path = json_obj.value("nuc_file_path").toInt();
        return temp;
    }

    throw "Unexpected";
}