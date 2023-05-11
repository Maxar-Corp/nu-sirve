#include "config.h"

namespace configreader {
    ConfigValues load() {
        QFile file(QString(CONFIG_FOLDER) + "/" + QString(CONFIG_FILE));
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        QString dataString=file.readAll();
        QJsonDocument doc= QJsonDocument::fromJson(dataString.toUtf8());
        QJsonObject data_object = doc.object();
        
        //defaults
        int max_used_bits = 14;
        double version = 0;

        if (data_object.contains("version")) {
            version = data_object.value("version").toDouble();
        }
        if (data_object.contains("max number of bits")) {
            max_used_bits = data_object.value("max number of bits").toInt();
        }

        file.close();
        return ConfigValues { version, max_used_bits };
    }
}