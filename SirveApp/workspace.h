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
#include "classification.h"

class StateManager;
namespace fs = std::filesystem;

struct WorkspaceValues {
    QString image_path;
    int start_frame = 0, end_frame = 0;
    double timing_offset = 0.0;
    std::vector<ProcessingState> all_states;
    std::vector<AnnotationInfo> annotations;
    std::vector<Classification> classifications;
};

class Workspace {
    public:
        Workspace() = default;
        Workspace(QString workspace_directory);
        ~Workspace();


    void SaveState(const QString& full_workspace_file_path, const QString& image_path, int start_frame, int end_frame,
        double timing_offset, const StateManager& all_states, const std::vector<AnnotationInfo>& annotations,
        const std::vector<Classification>& classifications);

        static WorkspaceValues LoadState(const QString& workspace_name);

        static QStringList get_workspace_names(const QString& workspace_folder);
};

#endif //WORKSPACE_H
