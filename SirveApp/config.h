#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FOLDER        "config"
#define CONFIG_FILE          "config.json"

#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QDir>

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

struct ConfigValues {
    double version;
    int max_used_bits;
    QString workspace_folder;
};

namespace configreader
{
        ConfigValues load();
}

#endif //CONFIG_H
