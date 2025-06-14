#ifndef BINARY_READER2_H
#define BINARY_READER2_H

#include <fstream>

#include "QObject"

class BinaryReader : public QObject
{
public:
    BinaryReader() = default;
    ~BinaryReader() override;

    template <typename T, std::size_t N>
    bool CanRead(T (&data)[N]);

    virtual bool Open(const QString& filename);
    virtual bool Close();
    [[nodiscard]] bool IsOpen() const;

    template <typename T>
    T Read(bool big_endian = false);

    template <typename T>
    size_t Read(T* data, uint32_t num_elements, bool big_endian = false);

    template <typename T, std::size_t N>
    size_t ReadArray(T (&data)[N], bool big_endian = false);

    template <typename T>
    std::vector<T> ReadVector(int num_values, bool big_endian);

    static constexpr bool IsBigEndian();
    uint64_t FileSize() const;

    void Seek(int64_t offset, int way) const;
    uint64_t Tell() const;

protected:
    [[nodiscard]] static constexpr bool NeedSwap(bool big_endian) ;

    template <typename T>
    static void SwapOrder(T* value);

    FILE* stream_ = nullptr;
};

constexpr bool BinaryReader::IsBigEndian()
{
    return Q_BYTE_ORDER == Q_BIG_ENDIAN;
}

constexpr bool BinaryReader::NeedSwap(bool big_endian)
{
    return big_endian ? !IsBigEndian() : IsBigEndian();
}

template <typename T, std::size_t N>
bool BinaryReader::CanRead(T(& data)[N])
{
    if (!stream_)
    {
        return false;
    }

    auto pos = Tell();
    if (pos == -1)
    {
        return false;
    }

    auto sz = FileSize();

    auto remaining = sz - pos;

    return remaining >= sizeof(T) * N;
}

template <typename T>
T BinaryReader::Read(bool big_endian)
{
    T data;

    auto elements_read = fread(&data, sizeof(T), 1, stream_);
    if (elements_read != 1)
    {
        throw std::runtime_error("Failed to read the expected number of elements from the stream.");
    }

    if (NeedSwap(big_endian))
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

    if (NeedSwap(big_endian))
    {
        for (auto i = 0u; i < elements_read; i++)
        {
            SwapOrder<T>(&data[i]);
        }
    }

    return elements_read;
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

    for (auto i = 0; i < num_values; i++)
    {
        if (NeedSwap(big_endian))
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
