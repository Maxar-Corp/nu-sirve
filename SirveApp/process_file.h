#pragma once


#ifndef PROCESS_FILE_H
#define PROCESS_FILE_H

#include <iostream>
#include <string>

#include <qstring.h>

#include "abir_reader.h"
#include "application_data.h"
#include "logging.h"
#include <qfiledialog.h>

class Process_File : public QWidget
{
	Q_OBJECT
public:
	ABIR_Data abir_data;

	unsigned int frame_start, frame_end;

	Process_File();
	~Process_File();

	AbpFileMetadata locate_abp_files(QString candidate_image_path);

	bool check_path(QString path);
	std::vector<std::vector<uint16_t>> load_image_file(QString image_path, int first_frame, int last_frame, double version);

private:

};



#endif // PROCESS_FILE_H