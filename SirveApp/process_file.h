#pragma once


#ifndef PROCESS_FILE_H
#define PROCESS_FILE_H

#include <iostream>
#include <string>

#include <qstring.h>

#include "abir_reader.h"
#include <qfiledialog.h>

struct AbpFileMetadata {
	QString osm_path, image_path, directory_path, file_name, info_msg, error_msg;
	AbpFileMetadata() {
		error_msg = "ABP Files Not Yet Configured.";
	}
};

class ProcessFile : public QWidget
{
	Q_OBJECT
public:
	ABIRData abir_data;

    ProcessFile();
    ~ProcessFile();

    bool VerifyPath(QString path);
    AbpFileMetadata LocateAbpFiles(QString candidate_image_path);
    ABIRDataResult LoadImageFile(QString image_path, int first_frame, int last_frame, double version);

private:

};



#endif // PROCESS_FILE_H
