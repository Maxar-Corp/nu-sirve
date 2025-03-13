#ifndef BINARY_READER2_H
#define BINARY_READER2_H

#include <fstream>

#include "QObject"

class BinaryReader : public QObject
{
public:
    BinaryReader() = default;
    ~BinaryReader() override;

    virtual bool Open(const QString& filename);
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

    void Seek(int64_t offset, int way);
    uint64_t Tell();

protected:
    template <typename T>
    static void SwapOrder(T* value);

    FILE* stream_ = nullptr;
};

template <typename T>
T BinaryReader::Read(bool big_endian)
{
    T data;

    auto elements_read = fread(&data, sizeof(T), 1, stream_);
    if (elements_read != 1)
    {
        throw std::runtime_error("Failed to read the expected number of elements from the stream.");
    }

    if (big_endian && !IsBigEndian())
    {
        SwapOrder<T>(&data);
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

    auto elements_read = fread(data, sizeof(T), num_elements, stream_);
    if (elements_read != num_elements)
    {
        throw std::runtime_error("Failed to read the expected number of elements from the stream.");
    }

    if (big_endian && !IsBigEndian())
    {
        for (auto i = 0; i < num_elements; i++)
        {
            SwapOrder<T>(&data[i]);
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

    auto elements_read = fread(&data[0], sizeof(T), num_values, stream_);
    if (elements_read != num_values)
    {
        throw std::runtime_error("Failed to read the expected number of elements from the stream.");
    }

    auto isBigEndian = IsBigEndian();

    for (auto i = 0; i < num_values; i++)
    {
        if (big_endian && !isBigEndian)
        {
            SwapOrder<T>(&data[i]);
        }
    }

    return data;
}

template <typename T>
void BinaryReader::SwapOrder(T* value)
{
    auto* pbytes = reinterpret_cast<uint8_t*>(value);
    std::reverse(pbytes, pbytes + sizeof(T));
}

#endif // BINARY_READER2_H
