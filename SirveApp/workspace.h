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

struct WorkspaceValues {
    QString image_path;
    int start_frame, end_frame;
};

class Workspace {
    public:
        Workspace();
        ~Workspace();

        void Workspace::save_state(QString image_path, int start_frame, int end_frame, std::vector<processing_state> all_states);
        WorkspaceValues Workspace::load_state();
    private:
        QJsonObject state_to_json(const processing_state & state);
};