#pragma once

#define WORKSPACE_FOLDER        "workspace"

#include <QDir>
#include "process_file.h"

class Workspace {
    public:
        Workspace();
        ~Workspace();

        void Workspace::save_state(AbpFileMetadata abp_metadata);
        AbpFileMetadata Workspace::load_state();
};