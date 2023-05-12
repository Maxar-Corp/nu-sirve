#include "workspace.h"

Workspace::Workspace() {
    QDir().mkdir(WORKSPACE_FOLDER);
};

Workspace::~Workspace() 
{
};

void Workspace::save_state(QString image_path, int start_frame, int end_frame) {
    //Inspiration: https://forum.qt.io/topic/65874/create-json-using-qjsondocument
    QJsonObject json_object;
    json_object.insert("image_path", image_path);
    json_object.insert("start_frame", start_frame);
    json_object.insert("end_frame", end_frame);
    QJsonDocument json_document(json_object);

    QFile file(QString(WORKSPACE_FOLDER) + "/" + QString(WORKSPACE_FILE));
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    file.write(json_document.toJson());
    file.close();
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

    file.close();
    return WorkspaceValues { image_path, start_frame, end_frame };
};
