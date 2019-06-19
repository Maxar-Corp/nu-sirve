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
	info_msg = QString("File Load Status\n");

	osm_path = QFileDialog::getOpenFileName(this, ("Open File"), "", ("OSM (*.abposm)"));
	QFileInfo osm_file_data(osm_path);

	info_msg.append("OSM file selected \n");

	bool valid_extension = osm_path.endsWith(".abposm", Qt::CaseInsensitive);
	bool osm_file_exists = osm_file_data.isFile();
	
	if (osm_file_exists && valid_extension) {
		info_msg.append("OSM file has proper extension \n");
	}
	else
	{
		info_msg.append("OSM file does not have right extension or is not file \n");
		
		valid_osm = false;
		return false;
	}

	read_osm_file();
	valid_osm = true;

	int index_file_start, index_file_end;
	index_file_start = osm_path.lastIndexOf("/");
	index_file_end = osm_path.lastIndexOf(".");

	directory_path = QString("Directory Path: ");
	file_name = QString("File: ");

	directory_path.append(osm_path.left(index_file_start));
	file_name.append(osm_path.mid(index_file_start + 1, index_file_end - index_file_start - 1));

	image_path = osm_path;
	image_path.replace(QString(".abposm"), QString(".abpimage"), Qt::CaseInsensitive);

	valid_image = check_image_path();
	
	if (valid_image) {
		info_msg.append("Corresponding image file found in same directory \n");
	}
	else
	{
		info_msg.append("Corresponding image file not found in same directory \n");
		return false;
	}		

	return true;
}

bool Process_File::read_osm_file()
{
	
	QByteArray array = osm_path.toLocal8Bit();
	char* buffer = array.data();
	
	osm_data.LoadFile(buffer, false);
	
	return true;
}

bool Process_File::check_image_path()
{
	
	QString info_msg("");

	QFileInfo check_image_file(image_path);
	bool image_file_isFile = check_image_file.isFile();
	bool image_file_exists = check_image_file.exists();

	return image_file_exists && image_file_isFile;

}

std::vector<std::vector<uint16_t>> Process_File::load_image_file(int first_frame, int last_frame, double version)
{

	std::vector<std::vector<uint16_t>> video_frames_16bit;

	if (first_frame < 0 || last_frame < 0)
		return video_frames_16bit;

	frame_start = first_frame;
	frame_end = last_frame;
	file_version = version;

	QByteArray array = image_path.toLocal8Bit();
	char* buffer = array.data();

	std::vector<unsigned int> frame_numbers{ frame_start, frame_end };
	abir_data.File_Setup(buffer, 4.2);

	video_frames_16bit = abir_data.Get_Data_and_Frames(frame_numbers, false);

	return video_frames_16bit;
}
