#include "binary_reader.h"

BinaryReader::~BinaryReader()
{
    Close();
}

bool BinaryReader::Open(const QString& filename)
{
    Close();

    auto array = filename.toLocal8Bit();
    auto* pfilename = array.constData();

    auto result = fopen_s(&stream_, pfilename, "rb");
    if (result != 0)
    {
        return result;
    }
    
    return true;
}

bool BinaryReader::Close()
{
    if (!IsOpen())
    {
        return false;
    }

    fclose(stream_);

    stream_ = nullptr;

    return true;
}

bool BinaryReader::IsOpen() const
{
    return stream_ != nullptr;
}

uint64_t BinaryReader::FileSize() const
{
    if (!IsOpen())
    {
        return 0;
    }

    auto save = _ftelli64(stream_);
    _fseeki64(stream_, 0, SEEK_END);
    auto size = _ftelli64(stream_);
    _fseeki64(stream_, save, SEEK_SET);

    return size;
}

void BinaryReader::Seek(int64_t offset, int way) const
{
    if (!IsOpen())
    {
        throw std::runtime_error("File is not open.");
    }

    auto result = _fseeki64(stream_, offset, way);
    if (result != 0)
    {
        throw std::runtime_error("Failed to seek in file.");
    }
}

uint64_t BinaryReader::Tell() const
{
    if (!IsOpen())
    {
        throw std::runtime_error("File is not open.");
    }

    return _ftelli64(stream_);
}
