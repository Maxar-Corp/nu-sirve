

#ifndef PROCESS_FILE_H
#define PROCESS_FILE_H

#include <QWidget>
#include <qstring.h>

#include "abir_reader.h"

struct AbpFileMetadata
{
	QString osm_path, image_path, directory_path, file_name, info_msg, error_msg;
    AbpFileMetadata()
    {
		error_msg = "ABP Files Not Yet Configured.";
	}
};

class ProcessFile : public QWidget
{
	Q_OBJECT
public:

    ProcessFile();
    ~ProcessFile() override;

    bool VerifyPath(const QString& path);
    AbpFileMetadata LocateAbpFiles(const QString& candidate_image_path);
    ABIRFrames::Ptr LoadImageFile(const QString& image_path, int first_frame, int last_frame, ABPFileType file_type);

public slots:
    void HandleProgressBarUpdate(int frame_index);

signals:
    void forwardProgress(int);
};

#endif // PROCESS_FILE_H
