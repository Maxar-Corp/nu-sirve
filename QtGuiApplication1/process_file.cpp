#include "process_file.h"

Process_File::Process_File()
{
	valid_image = false;
	valid_osm = false;
}

Process_File::~Process_File()
{
}

bool Process_File::load_osm_file()
{
	info_msg = QString("File Load Status: \n");

	// -----------------------------------------------------------------------------
	// get image path
	image_path = QFileDialog::getOpenFileName(this, ("Open File"), "", ("Image File(*.abpimage)"));
	//QFileInfo osm_file_data(image_path);

	info_msg.append("Image file selected \n");

	bool valid_image_extension = image_path.endsWith(".abpimage", Qt::CaseInsensitive);
	bool image_file_exists = check_path(image_path);

	// -----------------------------------------------------------------------------
	// get osm path
	osm_path = image_path;
	osm_path.replace(QString(".abpimage"), QString(".abposm"), Qt::CaseInsensitive);

	//QFileInfo image_file_data(osm_path);

	bool valid_osm_extension = osm_path.endsWith(".abposm", Qt::CaseInsensitive);
	bool osm_file_exists = check_path(osm_path);

	// -----------------------------------------------------------------------------
	// get file name to display
	int index_file_start, index_file_end;
	index_file_start = osm_path.lastIndexOf("/");
	index_file_end = osm_path.lastIndexOf(".");

	directory_path = osm_path.left(index_file_start);;
	file_name = QString("File: ");

	file_name.append(osm_path.mid(index_file_start + 1, index_file_end - index_file_start - 1));

	// -----------------------------------------------------------------------------
	// check image file is valid

	if (valid_image_extension && image_file_exists) {
		info_msg.append("Image file found with correct extension \n");
	}
	else
	{
		info_msg.append("Image file not found \n");
		return false;
	}

	// -----------------------------------------------------------------------------
	// check osm file is valid
	if (valid_osm_extension && osm_file_exists) {
		info_msg.append("OSM file found with correction extension \n");
	}
	else
	{
		info_msg.append("Corresponding OSM file not found \n");

		valid_osm = false;
		return false;
	}

	// -----------------------------------------------------------------------------
	// read osm file

	INFO << "Importing file " << osm_path.toStdString();

	bool check_read = read_osm_file();
	if (!check_read) {
		WARN << "File Processing: OSM load process quit early. File not loaded correctly";
		return false;
	}

	valid_osm = true;

	return true;
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
