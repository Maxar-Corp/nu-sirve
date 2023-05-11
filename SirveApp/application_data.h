#pragma once
#ifndef APPLICATION_DATA_H
#define APPLICATION_DATA_H

#include <qstring.h>

struct AbpFileMetadata {
	QString osm_path, image_path, directory_path, file_name, info_msg, error_msg;
	AbpFileMetadata() {
		error_msg = "ABP Files Not Yet Configured.";
	}
};

#endif //APPLICATION_DATA_H