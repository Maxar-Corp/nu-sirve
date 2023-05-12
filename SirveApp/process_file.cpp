#include "process_file.h"

Process_File::Process_File()
{
}

Process_File::~Process_File()
{
}

AbpFileMetadata Process_File::locate_abp_files(QString candidate_image_path)
{
	/*
	The returned struct will have error_msg populated if there was an error
	Otherwise, the returned struct will have all the values populated except error_msg, which will be empty
	*/
	AbpFileMetadata abp_data;
	abp_data.error_msg = QString();
	abp_data.info_msg = QString("File Load Status: \n");
	
	// -----------------------------------------------------------------------------
	// check abpimage file is valid
	bool valid_image_extension = candidate_image_path.endsWith(".abpimage", Qt::CaseInsensitive);
	bool image_file_exists = check_path(candidate_image_path);

	if (!valid_image_extension || !image_file_exists) {
		abp_data.error_msg = QString("File with .abpimage extension not found");
		return abp_data;
	}
	abp_data.info_msg.append("File found with correct extension \n");

	// -----------------------------------------------------------------------------
	// get and check if a valid osm file exists
	QString candidate_osm_path = candidate_image_path;
	candidate_osm_path.replace(QString(".abpimage"), QString(".abposm"), Qt::CaseInsensitive);

	if (!check_path(candidate_osm_path)) {
		abp_data.error_msg = QString("No corresponding file found with .abposm extension that matches the image file name");
		return abp_data;
	}
	abp_data.info_msg.append("Corresponding OSM file found with correction extension \n");

	// -----------------------------------------------------------------------------
	// get file name to display
	int index_file_start, index_file_end;
	index_file_start = candidate_osm_path.lastIndexOf("/");
	index_file_end = candidate_osm_path.lastIndexOf(".");

	QString directory_path = candidate_osm_path.left(index_file_start);

	QString file_name = QString("File: ");
	file_name.append(candidate_osm_path.mid(index_file_start + 1, index_file_end - index_file_start - 1));

	abp_data.image_path = candidate_image_path;
	abp_data.osm_path = candidate_osm_path;
	abp_data.directory_path = directory_path;
	abp_data.file_name = file_name;
	return abp_data;
}

bool Process_File::check_path(QString path)
{
	QFileInfo check_file(path);
	bool file_isFile = check_file.isFile();
	bool file_exists = check_file.exists();

	return file_exists && file_isFile;
}

std::vector<std::vector<uint16_t>> Process_File::load_image_file(QString image_path, int first_frame, int last_frame, double version)
{

	std::vector<std::vector<uint16_t>> video_frames_16bit;

	if (first_frame < 0 || last_frame < 0)
		return video_frames_16bit;

	unsigned int frame_start = first_frame;
	unsigned int frame_end = last_frame;

	try
	{
		QByteArray array = image_path.toLocal8Bit();
		char* buffer = array.data();

		int check_value = abir_data.File_Setup(buffer, version);

		if (check_value < 0)
			return video_frames_16bit;

		video_frames_16bit = abir_data.Get_Data_and_Frames(frame_start, frame_end, false);

		INFO << "Number of frames imported: " << video_frames_16bit.size();
	}
	catch (const std::exception& e)
	{
		INFO << "File Processing: Exception occurred when loading video data: " << e.what();
	}
	catch (...)
	{
		INFO << "File Processing: An OS/CPU level error occurred when loading video data and could not be handled. Save log for further investigation.";
	}

	return video_frames_16bit;
}
