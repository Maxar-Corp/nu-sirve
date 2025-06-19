

#ifndef PROCESS_FILE_H
#define PROCESS_FILE_H

#include <QWidget>
#include <qstring.h>

#include "abir_reader.h"

struct AbpFileMetadata
{
	QString osm_path;
    QString image_path;
    QString directory_path;
    QString file_name;
    QString info_msg;
    QString error_msg = "ABP Files Not Yet Configured.";
    QString warning_msg;
    ABPVersion version;
    ABPFileType file_type = ABPFileType::UNKNOWN;
};

class ProcessFile : public QWidget
{
	Q_OBJECT
public:

    ProcessFile();
    ~ProcessFile() override;

    static bool VerifyPath(const QString& path);
    AbpFileMetadata LocateAbpFiles(const QString& candidate_image_path);
    ABIRFrames::Ptr LoadImageFile(const QString& image_path, int first_frame, int last_frame);

public slots:
    void HandleProgressBarUpdate(int frame_index);

signals:
    void forwardProgress(int);
};

#endif // PROCESS_FILE_H
