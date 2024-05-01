#include "config.h"

namespace configreader {

    std::string getParentFolderPath(const std::string& filePath) {

        fs::path fullPath = fs::absolute(filePath);
        fs::path baseFolderPath = fullPath.parent_path();
        return baseFolderPath.string();
    }

    bool folderExists(const std::string& currentDir, const std::string& folderName) {

        std::string folderPath = currentDir + "/" + folderName;
        return fs::exists(folderPath) && fs::is_directory(folderPath);
    }

    std::string findPath(const std::string& filePath,  const std::string& folderName) {

        std::string parentPath = getParentFolderPath(filePath);
        if (!folderExists(parentPath, folderName))
            parentPath = getParentFolderPath(parentPath);
        return parentPath;
    }

    ConfigValues load() {

        QString currentPath = QDir::currentPath();
        std::string baseFolder = findPath(currentPath.toStdString(), CONFIG_FOLDER);
        QFile file(QString::fromStdString(baseFolder) + "/" + CONFIG_FOLDER + "/" + QString(CONFIG_FILE));
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        QString dataString=file.readAll();
        QJsonDocument doc= QJsonDocument::fromJson(dataString.toUtf8());
        QJsonObject data_object = doc.object();
        
        //defaults
        int max_used_bits = 14;
        double version = 0;
        QString workspace_folder = "workspace";

        if (data_object.contains("version")) {
            version = data_object.value("version").toDouble();
        }
        if (data_object.contains("max number of bits")) {
            max_used_bits = data_object.value("max number of bits").toInt();
        }
        if (data_object.contains("workspace folder")) {
            workspace_folder = (QString)data_object.value("workspace folder").toString();
        }

        file.close();
        return ConfigValues { version, max_used_bits, workspace_folder };
    }
}
