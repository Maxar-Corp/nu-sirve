#pragma once

#define WORKSPACE_FOLDER        "workspace"
#define WORKSPACE_FILE          "workspace.json"

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include "process_file.h"

class Workspace {
    public:
        Workspace();
        ~Workspace();

        void Workspace::save_state(QString image_path);
        QString Workspace::load_state();
};