#include "process_file.h"

Process_File::Process_File()
{
}

Process_File::~Process_File()
{
}

QString Process_File::load_osm_file()
{
	/*
	return empty string if the files were loaded correctly

	returns QString with an error message if there was an error during the loading process
	*/
	
	info_msg = QString("File Load Status: \n");
	
	// -----------------------------------------------------------------------------
	// get image path
	
	QString candidate_image_path = QFileDialog::getOpenFileName(this, ("Open File"), "", ("Image File(*.abpimage)"));
	
	// if no image path is selected then reset image path and return
	int compare = QString::compare(candidate_image_path, "", Qt::CaseInsensitive);
	if (compare == 0) {
		info_msg.append("No file was selected \n");
		return QString("No image file was selected");
	}

	info_msg.append("Image file selected \n");

	// -----------------------------------------------------------------------------
	// check image file is valid

	bool valid_image_extension = candidate_image_path.endsWith(".abpimage", Qt::CaseInsensitive);
	bool image_file_exists = check_path(candidate_image_path);

	if (!valid_image_extension || !image_file_exists) {
		info_msg.append("File with .abpimage extension not found \n");
		return QString("Image file issue");
	}
	info_msg.append("File found with correct extension \n");

	// -----------------------------------------------------------------------------
	// get and check if a valid osm file exists

	QString candidate_osm_path = candidate_image_path;
	candidate_osm_path.replace(QString(".abpimage"), QString(".abposm"), Qt::CaseInsensitive);

	if (!check_path(candidate_osm_path)) {
		info_msg.append("Corresponding file with .abposm extension not found \n");
		return QString("No OSM file found that matches the image file name");
	}
	
	info_msg.append("Corresponding OSM file found with correction extension \n");

	// -----------------------------------------------------------------------------
	// read osm file

	INFO << "Importing file " << candidate_osm_path.toStdString();

	bool check_read = read_osm_file(candidate_osm_path);
	if (!check_read) {
		WARN << "File Processing: OSM load process quit early. File not loaded correctly";
		info_msg.append("Error while reading OSM file. Close program and open logs for details \n");
		return QString("Error while reading OSM file. Close program and open logs for details.");
	}

	image_path = candidate_image_path;
	osm_path = candidate_osm_path;

	// -----------------------------------------------------------------------------
	// get file name to display
	int index_file_start, index_file_end;
	index_file_start = osm_path.lastIndexOf("/");
	index_file_end = osm_path.lastIndexOf(".");

	directory_path = osm_path.left(index_file_start);;
	file_name = QString("");

	file_name.append(osm_path.mid(index_file_start + 1, index_file_end - index_file_start - 1));

	return QString();
}

bool Process_File::read_osm_file(QString path)
{
	
	QByteArray array = path.toLocal8Bit();
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
