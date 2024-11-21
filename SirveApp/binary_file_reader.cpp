#ifndef BINARY_READER_CPP
#define BINARY_READER_CPP

#include "binary_file_reader.h"
#include "qdebug.h"

#include <iomanip>  // For std::hex and std::setfill
#include <cstddef>

BinaryFileReader::BinaryFileReader()
{


}

BinaryFileReader::~BinaryFileReader()
{
	//delete fp;
}

template<typename T>
void BinaryFileReader::ByteSwap(void *v, int num_bytes) {

	//https://stackoverflow.com/questions/4949144/how-to-byteswap-a-double

	int number_of_bytes = num_bytes; // sizeof(v);
	int last_index = number_of_bytes - 1;

	char *in = new char[number_of_bytes];
	char *out = new char[number_of_bytes];

	//char in[number_of_bytes], out[number_of_bytes];
	memcpy(in, v, number_of_bytes);

	for (size_t i = 0; i < number_of_bytes; i++)
	{
		out[i] = in[last_index - i];
	}

	memcpy(v, out, number_of_bytes);

	delete[] in;
	delete[] out;
}

template<typename T>
std::vector<T> BinaryFileReader::ReadMultipleValuesIntoVector(int number_values, bool convert_endian) {

	std::vector<T> data_vector;
	data_vector.reserve(number_values);

	for (int i = 0; i < number_values; i++)
	{
		T value;
		value = ReadValue<T>(convert_endian);
		data_vector.push_back(value);
	}

	return data_vector;
}

template<typename T>
T BinaryFileReader::ReadValue(bool convert_endian) {

    bool is_double = std::is_same<T, double>::value;

    if (is_double & convert_endian) {

        std::vector<double> out = ReadMultipleDoubleValues(1, convert_endian);
        return out[0];
    }

    //qDebug() << "About to call...";

    T data[1];
    size_t result = ReadMultipleValues(data, convert_endian);

    //qDebug() << "GOT HERE 4";
    //qDebug() << "result = " << result;
    //qDebug() << data[0];

    return data[0];
}

std::vector<double> BinaryFileReader::ReadMultipleDoubleValues(int num_values, bool bigendian)
{

	std::vector<double> out;

	for (int i = 0; i < num_values; i++)
	{
        out.push_back(ReadDoubleFromBinary(fp, bigendian));
	}

	return out;
}

template <typename T>
T BinaryFileReader::ByteSwap(T value) {
    uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&value);
    std::reverse(bytePtr, bytePtr + sizeof(T));
    return value;
}

template <typename T>
void printBytes(const T* data, size_t size) {
    const unsigned char* byteData = reinterpret_cast<const unsigned char*>(data);
    QString byteString;

    for (size_t i = 0; i < size * sizeof(T); ++i) {
        // Append each byte as a two-digit hexadecimal number
        byteString.append(QString("%1 ").arg(byteData[i], 2, 16, QChar('0')).toUpper());
    }

    //qDebug() << byteString.trimmed(); // Output the bytes
}

template <typename T>
size_t BinaryFileReader::ReadMultipleValues(T& data, bool bigendian, uint64_t force_num_elements) {
    if (!data || !fp) {
        throw std::invalid_argument("Null pointer provided for data or stream.");
    }

    // Calculate the total size of the array
    size_t total_bytes = sizeof(data);
    //qDebug() << "total_bytes=" << total_bytes;
    const uint64_t bytes_per_element = sizeof(data[0]);
    //qDebug() << "bytes_per_element=" << bytes_per_element;
    uint64_t num_elements = total_bytes / bytes_per_element;
    //qDebug() << "num_elements=" << num_elements;

    if (force_num_elements > 0) {
        num_elements = force_num_elements;
    }

    if (num_elements == 0) {
        throw std::invalid_argument("Total bytes is less than the size of one element.");
    }

    //qDebug() << "Initially, data is this: " << data;
    //printBytes(data, num_elements);

    // Read the data using fread
    size_t read_count = fread(data, bytes_per_element, num_elements, fp);

    //qDebug() << "read_count=" << read_count;

    //printBytes(data, num_elements);
    //qDebug() << "done";

    if (read_count != num_elements) {
        throw std::runtime_error("Failed to read the expected number of elements from the stream.");
    }

    // Perform byte swapping if bigendian is true
    if (bigendian) {
        for (size_t i = 0; i < num_elements; i++)
            ByteSwap<T>(&data[i], bytes_per_element);
    }

    // return the expected number of elements from the stream
    return read_count;
}

// Utility to determine system endianness.
bool BinaryFileReader::isSystemBigEndian() {
    uint16_t test = 0x0102;
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&test);
    return bytes[0] == 0x01; // Big-endian if the most significant byte is first.
}

// Swap the byte order of a double value.
double BinaryFileReader::swapEndianness(double value) {
    union {
        double d;
        uint8_t bytes[sizeof(double)];
    } source, dest;

    source.d = value;
    for (size_t i = 0; i < sizeof(double); ++i) {
        dest.bytes[i] = source.bytes[sizeof(double) - 1 - i];
    }

    return dest.d;
}

// Reads a double value from a binary file stream, accounting for endianness.
double BinaryFileReader::ReadDoubleFromBinary(FILE *fp, int bigendian) {
    if (!fp) {
        throw std::invalid_argument("Invalid file pointer.");
    }

    double value = 0.0;
    size_t bytesRead = fread(&value, sizeof(double), 1, fp);

    if (bytesRead != 1) {
        if (feof(fp)) {
            throw std::runtime_error("End of file reached unexpectedly.");
        } else {
            throw std::runtime_error("Failed to read a double value from the binary file.");
        }
    }

    // Swap byte order if file's endianness doesn't match the system's
    if (bigendian != isSystemBigEndian()) {
        value = swapEndianness(value);
    }

    return value;
}

#endif
