#pragma once

#define WORKSPACE_FOLDER        "workspace"
#define WORKSPACE_FILE          "workspace.json"

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "process_file.h"
#include "processing_state.h"
#include "annotation_info.h"

struct WorkspaceValues {
    QString image_path;
    int start_frame, end_frame;
    std::vector<processing_state> all_states;
    std::vector<annotation_info> annotations;
};

class Workspace {
    public:
        Workspace();
        ~Workspace();

        void Workspace::save_state(QString image_path, int start_frame, int end_frame, const std::vector<processing_state> all_states, const std::vector<annotation_info> annotations);
        WorkspaceValues Workspace::load_state(QString workspace_name);
};