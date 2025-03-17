#include "process_file.h"
#include <QOpenGLContext>
#include <qfileinfo.h>

ProcessFile::ProcessFile()
{
}

ProcessFile::~ProcessFile()
{
}

AbpFileMetadata ProcessFile::LocateAbpFiles(const QString& candidate_image_path)
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
	bool image_file_exists = VerifyPath(candidate_image_path);

    if (!valid_image_extension || !image_file_exists)
    {
		abp_data.error_msg = QString("File with .abpimage extension not found");
		return abp_data;
	}
	abp_data.info_msg.append("File found with correct extension \n");

	// -----------------------------------------------------------------------------
	// get and check if a valid osm file exists
	QString candidate_osm_path = candidate_image_path;
	candidate_osm_path.replace(QString(".abpimage"), QString(".abposm"), Qt::CaseInsensitive);

    if (!VerifyPath(candidate_osm_path))
    {
        abp_data.error_msg = QString(
            "No corresponding file found with .abposm extension that matches the image file name");
		return abp_data;
	}
	abp_data.info_msg.append("Corresponding OSM file found with correction extension \n");

	// -----------------------------------------------------------------------------
	// get file name to display
	int index_file_start, index_file_end;
	index_file_start = candidate_osm_path.lastIndexOf("/");
	index_file_end = candidate_osm_path.lastIndexOf(".");

	QString directory_path = candidate_osm_path.left(index_file_start);

	// QString file_name = QString("File: ");
	QString file_name = candidate_osm_path.mid(index_file_start + 1, index_file_end - index_file_start - 1);

	abp_data.image_path = candidate_image_path;
	abp_data.osm_path = candidate_osm_path;
	abp_data.directory_path = directory_path;
	abp_data.file_name = file_name;
	return abp_data;
}

bool ProcessFile::VerifyPath(const QString& path)
{
	QFileInfo check_file(path);
	bool file_isFile = check_file.isFile();
	bool file_exists = check_file.exists();

	return file_exists && file_isFile;
}

bool ProcessFile::LoadImageFile(const QString& image_path, int first_frame, int last_frame, double version)
{	
	if (first_frame < 0 || last_frame < 0)
	{
        return false;
	}

	unsigned int frame_start = first_frame;
	unsigned int frame_end = last_frame;

	QByteArray array = image_path.toLocal8Bit();
	char* buffer = array.data();

    ABIRReader reader;
    connect(&reader, &ABIRReader::advanceFrame, this, &ProcessFile::HandleProgressBarUpdate);

    if (!reader.Open(buffer, version))
    {
        return false;
	}

    frames_ = reader.ReadFrames(frame_start, frame_end, false);    
    return frames_ != nullptr;
}

void ProcessFile::HandleProgressBarUpdate(int frame_index)
{
    emit forwardProgress(frame_index);
}

