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

class VideoContainer;
namespace fs = std::filesystem;

struct WorkspaceValues {
    QString image_path;
    int start_frame, end_frame;
    double timing_offset;
    std::vector<processingState> all_states;
    std::vector<AnnotationInfo> annotations;
    std::vector<Classification> classifications;
};

class Workspace {
    public:
        Workspace();
        Workspace(QString workspace_directory);
        ~Workspace();

    void SaveState(const QString& full_workspace_file_path, const QString& image_path, int start_frame, int end_frame,
        double timing_offset, const VideoContainer& all_states, const std::vector<AnnotationInfo>& annotations,
        const std::vector<Classification>& classifications);

        static WorkspaceValues LoadState(const QString& workspace_name);

        static QStringList get_workspace_names(const QString& workspace_folder);
};

#endif //WORKSPACE_H
