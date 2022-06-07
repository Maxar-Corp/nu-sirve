#include "process_file.h"

Process_File::Process_File()
{
	valid_image = false;
	valid_osm = false;
}

Process_File::~Process_File()
{
}

int Process_File::load_osm_file()
{
	/*
	
	return 1, if file loaded correctly
	returns 2-5 to correspond to errors during the loading process
	
	*/
	
	info_msg = QString("File Load Status: \n");
	QString old_image_path = image_path;
	QString old_osm_path = osm_path;
	
	// -----------------------------------------------------------------------------
	// get image path
	
	image_path = QFileDialog::getOpenFileName(this, ("Open File"), "", ("Image File(*.abpimage)"));
	
	// if no image path is selected then reset image path and return
	int compare = QString::compare(image_path, "", Qt::CaseInsensitive);
	if (compare == 0) {
		image_path = old_image_path;
		return 2;
	}

	info_msg.append("Image file selected \n");

	// -----------------------------------------------------------------------------
	// check image file is valid

	bool valid_image_extension = image_path.endsWith(".abpimage", Qt::CaseInsensitive);
	bool image_file_exists = check_path(image_path);

	if (valid_image_extension && image_file_exists) {
		info_msg.append("Image file found with correct extension \n");
		valid_image = true;
	}
	else
	{
		info_msg.append("Image file not found \n");
		image_path = old_image_path;
		return 3;
	}

	// -----------------------------------------------------------------------------
	// get osm path

	osm_path = image_path;
	osm_path.replace(QString(".abpimage"), QString(".abposm"), Qt::CaseInsensitive);

	// -----------------------------------------------------------------------------
	// check osm file is valid

	bool valid_osm_extension = osm_path.endsWith(".abposm", Qt::CaseInsensitive);
	bool osm_file_exists = check_path(osm_path);
	
	if (valid_osm_extension && osm_file_exists) {
		info_msg.append("OSM file found with correction extension \n");
	}
	else
	{
		osm_path = old_osm_path;
		info_msg.append("Corresponding OSM file not found \n");

		valid_osm = false;
		image_path = old_image_path;
		osm_path = old_osm_path;
		return 4;
	}

	// -----------------------------------------------------------------------------
	// read osm file

	INFO << "Importing file " << osm_path.toStdString();

	bool check_read = read_osm_file();
	if (!check_read) {
		WARN << "File Processing: OSM load process quit early. File not loaded correctly";
		
		image_path = old_image_path;
		osm_path = old_osm_path;

		return 5;
	}

	valid_osm = true;

	// -----------------------------------------------------------------------------
	// get file name to display
	int index_file_start, index_file_end;
	index_file_start = osm_path.lastIndexOf("/");
	index_file_end = osm_path.lastIndexOf(".");

	directory_path = osm_path.left(index_file_start);;
	file_name = QString("File: ");

	file_name.append(osm_path.mid(index_file_start + 1, index_file_end - index_file_start - 1));

	return 1;
}

bool Process_File::read_osm_file()
{
	
	QByteArray array = osm_path.toLocal8Bit();
	char* buffer = array.data();
	
	int check = osm_data.LoadFile(buffer, false);
	if (check != 0)
		return false;
	
	return true;
}

bool Process_File::check_path(QString path)
{
	
	QString info_msg("");

	QFileInfo check_file(path);
	bool file_isFile = check_file.isFile();
	bool file_exists = check_file.exists();

	return file_exists && file_isFile;

}

std::vector<std::vector<uint16_t>> Process_File::load_image_file(int first_frame, int last_frame, double version)
{

	std::vector<std::vector<uint16_t>> video_frames_16bit;

	if (first_frame < 0 || last_frame < 0)
		return video_frames_16bit;

	frame_start = first_frame;
	frame_end = last_frame;
	file_version = version;

	try
	{

	QByteArray array = image_path.toLocal8Bit();
	char* buffer = array.data();

	std::vector<unsigned int> frame_numbers{ frame_start, frame_end };
	int check_value = abir_data.File_Setup(buffer, version);

	if (check_value < 0)
		return video_frames_16bit;

	video_frames_16bit = abir_data.Get_Data_and_Frames(frame_numbers, false);

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
