#pragma once

#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include "processing_state.h"
#include "annotation_info.h"
#include "support/qthelpers.h"

namespace fs = std::filesystem;

struct WorkspaceValues {
    QString image_path;
    int start_frame, end_frame;
    std::vector<processingState> all_states;
    std::vector<AnnotationInfo> annotations;
};

class Workspace {
    public:
        Workspace();
        Workspace(QString workspace_directory);
        ~Workspace();

        void Workspace::SaveState(QString full_workspace_file_path, QString image_path, int start_frame, int end_frame, const std::vector<processingState> all_states, const std::vector<AnnotationInfo> annotations);
        WorkspaceValues Workspace::LoadState(QString workspace_name);
        void Workspace::UpdateWorkspaceDirectory(QString workspace_directory);

        QStringList Workspace::get_workspace_names(QString workspace_folder);
};

#endif //WORKSPACE_H
