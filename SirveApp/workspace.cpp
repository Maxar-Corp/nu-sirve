#include "workspace.h"

Workspace::Workspace() {
    QDir().mkdir(WORKSPACE_FOLDER);
};

Workspace::~Workspace() 
{
};

void Workspace::save_state(AbpFileMetadata abp_metadata) {
};

AbpFileMetadata Workspace::load_state() {
    return AbpFileMetadata();
};
