#include "binary_reader.h"

BinaryReader::~BinaryReader()
{
    Close();
}

bool BinaryReader::Open(const char* filename)
{
    Close();

    stream_.open(filename);
    if (!stream_.is_open())
    {
        return false;
    }

    return true;
}

bool BinaryReader::Close()
{
    if (!IsOpen())
    {
        return false;
    }

    stream_.close();

    return true;
}

bool BinaryReader::IsOpen() const { return stream_.is_open(); }

bool BinaryReader::IsBigEndian()
{
    int16_t test = 0x0102;
    auto bytes = reinterpret_cast<uint8_t*>(&test);
    return bytes[0] == 0x01;
}

uint64_t BinaryReader::FileSize()
{
    if (!IsOpen())
    {
        return 0;
    }

    auto save = stream_.tellg();
    stream_.seekg(0, std::ios::end);
    auto size = stream_.tellg();
    stream_.seekg(save);

    return size;
}

void BinaryReader::Seek(int64_t offset, std::ios_base::seekdir way)
{
    if (!IsOpen())
    {
        throw std::runtime_error("File is not open.");
    }

    stream_.seekg(offset, way);
    if (stream_.fail())
    {
        throw std::runtime_error("Failed to seek in file.");
    }
}

uint64_t BinaryReader::Tell()
{
    return stream_.tellg();
}
