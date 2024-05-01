#include "config.h"

namespace configReaderWriter {

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

QJsonObject getDataObject() {

    QString currentPath = QDir::currentPath();
    std::string baseFolder = findPath(currentPath.toStdString(), CONFIG_FOLDER);
    QFile file(QString::fromStdString(baseFolder) + "/" + CONFIG_FOLDER + "/" + QString(CONFIG_FILE));
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QString dataString=file.readAll();
    QJsonDocument doc= QJsonDocument::fromJson(dataString.toUtf8());
    file.close();

    return  doc.object();
}

ConfigValues load() {

    QJsonObject data_object = getDataObject();

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

    return ConfigValues { version, max_used_bits, workspace_folder };
}

void writeJsonObjectToFile(const QJsonObject &jsonObject, const QString &filePath) {
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

void saveWorkspaceFolder(QString workspace_folder) {

    QJsonObject data_object = getDataObject();
    data_object["workspace folder"] = workspace_folder;

    QString currentPath = QDir::currentPath();
    std::string baseFolder = findPath(currentPath.toStdString(), CONFIG_FOLDER);
    QString filePath = QString::fromStdString(baseFolder) + "/" + CONFIG_FOLDER + "/" + QString(CONFIG_FILE);

    writeJsonObjectToFile(data_object, filePath);
}
}
