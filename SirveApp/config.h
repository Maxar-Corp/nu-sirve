#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FOLDER        "config"
#define CONFIG_FILE          "config.json"

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>

#include "application_data.h"

namespace configreader 
{
        ConfigValues load();
}

#endif //CONFIG_H