#include "config.h"

namespace configReaderWriter {

    std::string get_immediate_parent_folder_path(const std::string& filePath) {

        fs::path fullPath = fs::absolute(filePath);
        fs::path parentPath = fullPath.parent_path();

        return parentPath.string();
    }

    bool folder_exists(const std::string& currentDir, const std::string& folderName) {

        std::string folderPath = currentDir + "/" + folderName;
        return fs::exists(folderPath) && fs::is_directory(folderPath);
    }

    std::string LocateConfigFileFolder(const std::string& filePath,  const std::string& folderName) {

        if (folder_exists(filePath, folderName))
            return filePath;
        else {
            std::string parentPath = get_immediate_parent_folder_path(filePath);
            if (!folder_exists(parentPath, folderName))
                parentPath = get_immediate_parent_folder_path(parentPath);

            return parentPath;
        }
    }

    QJsonObject ExtractConfigDataJsonObject() {

        QString currentPath = QDir::currentPath();
        std::string baseFolder = LocateConfigFileFolder(currentPath.toStdString(), CONFIG_FOLDER);
        QFile file(QString::fromStdString(baseFolder) + "/" + CONFIG_FOLDER + "/" + QString(CONFIG_FILE));
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        QString dataString=file.readAll();
        QJsonDocument doc= QJsonDocument::fromJson(dataString.toUtf8());
        file.close();

        return  doc.object();
    }

    ConfigValues ExtractWorkspaceConfigValues() {

        QJsonObject data_object = ExtractConfigDataJsonObject();

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
            QFileInfo fileInfo_0(data_object.value("workspace folder").toString());
            if(fileInfo_0.isRelative()){
                QString currentPath = QDir::currentPath();
                workspace_folder = currentPath + "/" + fileInfo_0.baseName();
            }
            else{
                workspace_folder = fileInfo_0.absolutePath();
            }
        }

        return ConfigValues { version, max_used_bits, workspace_folder };
    }

    void WriteJsonObjectToFile(const QJsonObject &jsonObject, const QString &filePath) {

        // Convert QJsonObject to QJsonDocument
        QJsonDocument jsonDoc(jsonObject);

        // Serialize QJsonDocument to QByteArray
        QByteArray byteArray = jsonDoc.toJson();

        // Write QByteArray to file
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(byteArray);
            file.close();
            qDebug() << "JSON object written to file:" << filePath;
        } else {
            qDebug() << "Error opening file for writing:" << filePath;
        }
    }

    void SaveWorkspaceFolder(QString workspace_folder) {

        QJsonObject config_data = ExtractConfigDataJsonObject();
        config_data["workspace folder"] = workspace_folder;

        QString current_path = QDir::currentPath();

        std::string baseFolder = LocateConfigFileFolder(current_path.toStdString(), CONFIG_FOLDER);

        QString filePath = QString::fromStdString(baseFolder) + "/" + CONFIG_FOLDER + "/" + QString(CONFIG_FILE);

        WriteJsonObjectToFile(config_data, filePath);
    }
}
