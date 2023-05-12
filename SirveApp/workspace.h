#pragma once

#define WORKSPACE_FOLDER        "workspace"
#define WORKSPACE_FILE          "workspace.json"

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include "process_file.h"

struct WorkspaceValues {
    QString image_path;
    int start_frame, end_frame;
};

class Workspace {
    public:
        Workspace();
        ~Workspace();

        void Workspace::save_state(QString image_path, int start_frame, int end_frame);
        WorkspaceValues Workspace::load_state();
};