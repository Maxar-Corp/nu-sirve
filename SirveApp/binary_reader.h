#ifndef BINARY_READER_H
#define BINARY_READER_H

#include <fstream>
#include "SirveApp.h"

class BinaryReader : public QObject
{
public:
    BinaryReader() = default;

    ~BinaryReader() override;

    virtual bool Open(const char* filename);
    bool Close();
    [[nodiscard]] bool IsOpen() const;

    template <typename T>
    T Read(bool big_endian = false);

    template <typename T>
    size_t Read(T* data, uint32_t num_elements, bool big_endian = false);

    template <typename T, std::size_t N>
    size_t ReadArray(T (&data)[N], bool big_endian = false);

    template <typename T>
    std::vector<T> ReadVector(int num_values, bool big_endian);

    static bool IsBigEndian();
    uint64_t FileSize();

    void Seek(int64_t offset, std::ios_base::seekdir way);
    uint64_t Tell();

protected:
    template <typename T>
    static T SwapOrder(T value);

    std::ifstream stream_;
};

template <typename T>
T BinaryReader::Read(bool big_endian)
{
    T data;

    stream_.read(reinterpret_cast<char*>(&data), sizeof(T));
    if (stream_.fail())
    {
        throw std::runtime_error("Failed to read from file.");
    }

    if (big_endian && !IsBigEndian())
    {
        data = SwapOrder<T>(data);
    }

    return data;
}

template <typename T>
size_t BinaryReader::Read(T* data, uint32_t num_elements, bool big_endian)
{
    if (num_elements == 0)
    {
        throw std::invalid_argument("Total bytes is less than the size of one element.");
    }

    stream_.read(reinterpret_cast<char*>(data), sizeof(T) * num_elements);
    if (stream_.fail())
    {
        throw std::runtime_error("Failed to read from file");
    }

    if (big_endian && !IsBigEndian())
    {
        for (auto i = 0; i < num_elements; i++)
        {
            data[i] = SwapOrder<T>(data[i]);
        }
    }

    return num_elements;
}

template <typename T, std::size_t N>
size_t BinaryReader::ReadArray(T (&data)[N], bool big_endian)
{
    return Read(data, N, big_endian);
}

template <typename T>
std::vector<T> BinaryReader::ReadVector(int num_values, bool big_endian)
{
    std::vector<T> data(num_values);

    stream_.read(reinterpret_cast<char*>(&data[0]), sizeof(T) * num_values);
    if (stream_.fail())
    {
        throw std::runtime_error("Failed to read from file.");
    }

    auto isBigEndian = IsBigEndian();

    for (auto i = 0; i < num_values; i++)
    {
        if (big_endian && !isBigEndian)
        {
            data[i] = SwapOrder<T>(data[i]);
        }
    }

    return data;
}

template <typename T>
T BinaryReader::SwapOrder(T value)
{
    auto* pbytes = reinterpret_cast<uint8_t*>(&value);
    std::reverse(pbytes, pbytes + sizeof(T));

    return value;
}

#endif // BINARY_READER_H
