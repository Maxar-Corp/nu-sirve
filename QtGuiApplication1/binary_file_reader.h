#pragma once

#ifndef BINARY_READER_H
#define BINARY_READER_H

#include <iostream>
#include <fstream>
//#include <filesystem>
#include <vector>
#include <cmath>


class BinaryFileReader
{
public:

	inline BinaryFileReader();
	inline ~BinaryFileReader();

	FILE *fp;

	//Helper Functions
    template<typename T> inline size_t FindArraySize(T data);

    template<typename T> inline size_t ReadMultipleValues(T &data, bool convert_endian = false, uint64_t force_num_elements = 0);
    template<typename T> inline std::vector<T> ReadMultipleValuesIntoVector(int number_values, bool convert_endian = false);
    template<typename T> inline T ReadValue(bool convert_endian = false);

	inline std::vector<double> ReadMultipleDoubleValues(int num_values, bool convert_endian = false);
	template<typename T> inline void ByteSwap(void * v, int num_bytes);
	inline double ReadDoubleFromBinary(FILE *fp, int bigendian);
};



BinaryFileReader::BinaryFileReader()
{


}

BinaryFileReader::~BinaryFileReader()
{
	//delete fp;
}

template<typename T>
size_t BinaryFileReader::FindArraySize(T data)
{

	int size_total_array = sizeof(data);
	int size_element = sizeof(data[0]);
	size_t num_elements = size_total_array / size_element;

	//return *(&data + 1) - data;
	return num_elements;
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
}

template<typename T>
size_t BinaryFileReader::ReadMultipleValues(T &data, bool convert_endian, uint64_t force_num_elements) {

	const uint64_t total_bytes = sizeof(data);
	const uint64_t bytes_per_element = sizeof(data[0]);
	uint64_t num_elements = total_bytes / bytes_per_element;

	if (force_num_elements > 0) {
		num_elements = force_num_elements;
	}

	size_t read_code = fread(data, bytes_per_element, num_elements, fp);

	if (convert_endian) {

		for (size_t i = 0; i < num_elements; i++)
		{			
			ByteSwap<T>(&data[i], bytes_per_element);
		}
	}

	return read_code;
}

template<typename T>
std::vector<T> BinaryFileReader::ReadMultipleValuesIntoVector(int number_values, bool convert_endian) {

	std::vector<T> data_vector;
	data_vector.reserve(number_values);
	//bool is_double = std::is_same_v<T, double>;

	//if (is_double & convert_endian) {
	//	std::vector<T> double_out;
	//	double_out = ReadMultipleDoubleValues(3, true);

	//	return double_out;
	//}
	//else {	// If not double or double but no bit-swapping necessary...

	for (int i = 0; i < number_values; i++)
	{
		T value;
		value = ReadValue<T>(convert_endian);
		data_vector.push_back(value);
	}
	//}
	return data_vector;
}

std::vector<double> BinaryFileReader::ReadMultipleDoubleValues(int num_values, bool convert_endian)
{

	std::vector<double> out;

	for (int i = 0; i < num_values; i++)
	{
		double data = ReadDoubleFromBinary(fp, convert_endian);
		out.push_back(data);
	}

	return out;
}

template<typename T>
T BinaryFileReader::ReadValue(bool convert_endian) {

	bool is_double = std::is_same<T, double>::value;;

	if (is_double & convert_endian) {

		std::vector<double> out = ReadMultipleDoubleValues(1, convert_endian);
		return out[0];
	}

	T data[1];
	ReadMultipleValues(data, convert_endian);

	return data[0];
}

double BinaryFileReader::ReadDoubleFromBinary(FILE *fp, int bigendian)
{
	//Function help from https://github.com/MalcolmMcLean/ieee754/blob/master/ieee754.c

	unsigned char buff[8];
	int i;
	double fnorm = 0.0;
	unsigned char temp;
	int sign;
	int exponent;
	double bitval;
	int maski, mask;
	int expbits = 11;
	int significandbits = 52;
	int shift;
	double answer;

	/* read the data */
	for (i = 0; i < 8; i++)
		buff[i] = fgetc(fp);
	/* just reverse if not big-endian*/
	if (!bigendian)
	{
		for (i = 0; i < 4; i++)
		{
			temp = buff[i];
			buff[i] = buff[8 - i - 1];
			buff[8 - i - 1] = temp;
		}
	}
	sign = buff[0] & 0x80 ? -1 : 1;
	/* exponet in raw format*/
	exponent = ((buff[0] & 0x7F) << 4) | ((buff[1] & 0xF0) >> 4);

	/* read in the mantissa. Top bit is 0.5, the successive bits half*/
	bitval = 0.5;
	maski = 1;
	mask = 0x08;
	for (i = 0; i < significandbits; i++)
	{
		if (buff[maski] & mask)
			fnorm += bitval;

		bitval /= 2.0;
		mask >>= 1;
		if (mask == 0)
		{
			mask = 0x80;
			maski++;
		}
	}
	/* handle zero specially */
	if (exponent == 0 && fnorm == 0)
		return 0.0;

	shift = exponent - ((1 << (expbits - 1)) - 1); /* exponent = shift + bias */
	/* nans have exp 1024 and non-zero mantissa */
	if (shift == 1024 && fnorm != 0)
		return std::sqrt(-1.0);
	/*infinity*/
	if (shift == 1024 && fnorm == 0)
	{

#ifdef INFINITY
		return sign == 1 ? INFINITY : -INFINITY;
#endif
		return	(sign * 1.0) / 0.0;
	}
	if (shift > -1023)
	{
		answer = std::ldexp(fnorm + 1.0, shift);
		return answer * sign;
	}
	else
	{
		/* denormalised numbers */
		if (fnorm == 0.0)
			return 0.0;
		shift = -1022;
		while (fnorm < 1.0)
		{
			fnorm *= 2;
			shift--;
		}
		answer = std::ldexp(fnorm, shift);
		return answer * sign;
	}
}



#endif
