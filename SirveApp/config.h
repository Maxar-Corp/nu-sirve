#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FOLDER        "config"
#define CONFIG_FILE          "config.json"

#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>

struct ConfigValues {
    double version;
    int max_used_bits;
};

namespace configreader
{
        ConfigValues load();
}

#endif //CONFIG_H