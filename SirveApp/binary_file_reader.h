#pragma once

#include "qobject.h"
#ifndef BINARY_READER_H
#define BINARY_READER_H

#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>

class BinaryFileReader : public QObject
{
    public:

        inline BinaryFileReader();
        inline ~BinaryFileReader();

        FILE *fp;

        template<typename T> inline void            ByteSwap(void * v, int num_bytes);
        inline double                               ReadDoubleFromBinary(FILE *fp, int bigendian);
        inline std::vector<double>                  ReadMultipleDoubleValues(int num_values, bool bigendian = false);
        template<typename T> inline size_t          ReadMultipleValues(T& data, bool bigendian = false, uint64_t force_num_elements = 0);
        template<typename T> inline std::vector<T>  ReadMultipleValuesIntoVector(int number_values, bool convert_endian = false);
        template<typename T> inline T               ReadValue(bool convert_endian = false);

    private:

        template<typename T> inline T               ByteSwap(T value);
        inline bool                                 IsSystemBigEndian();
        inline double                               SwapEndianness(double value);
};

#endif
