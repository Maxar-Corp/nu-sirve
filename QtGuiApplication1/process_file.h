#pragma once


#ifndef PROCESS_FILE_H
#define PROCESS_FILE_H

#include <iostream>
#include <string>

#include <qstring.h>

#include "abir_reader.h"
#include "osm_reader.h"
#include <qfiledialog.h>


class Process_File : public QWidget
{
	Q_OBJECT
public:

	QString osm_path, image_path, directory_path, file_name, info_msg;
	bool valid_osm, valid_image;

	OSMReader osm_data;
	ABIR_Data abir_data;

	unsigned int frame_start, frame_end;
	double file_version;

	Process_File();
	~Process_File();

	bool load_osm_file();
	bool read_osm_file();

	bool check_image_path();
	std::vector<std::vector<uint16_t>> load_image_file(int first_frame, int last_frame, double version);

private:

};



#endif // PROCESS_FILE_H