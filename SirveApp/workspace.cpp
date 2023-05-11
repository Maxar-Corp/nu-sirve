#include "workspace.h"

Workspace::Workspace() {
    QDir().mkdir(WORKSPACE_FOLDER);
};

Workspace::~Workspace() 
{
};

void Workspace::save_state(QString image_path) {
    //Inspiration: https://forum.qt.io/topic/65874/create-json-using-qjsondocument
    QJsonObject json_object;
    json_object.insert("image_path", image_path);
    QJsonDocument json_document(json_object);

    QFile file(QString(WORKSPACE_FOLDER) + "/" + QString(WORKSPACE_FILE));
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    file.write(json_document.toJson());
    file.close();
};

QString Workspace::load_state() {
     //Inspiration: https://stackoverflow.com/questions/60723466/how-to-write-and-read-in-json-file-using-qt
    QFile file(QString(WORKSPACE_FOLDER) + "/" + QString(WORKSPACE_FILE));
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QString dataString=file.readAll();
    QJsonDocument doc= QJsonDocument::fromJson(dataString.toUtf8());
    QJsonObject data_object = doc.object();

    QString image_path = data_object.value(QString("image_path")).toString();

    file.close();
    return image_path;
};
