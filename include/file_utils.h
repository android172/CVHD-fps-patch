#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

const int  __n__            = 1;
const bool is_little_endian = *(char*) &__n__ == 1;

#include <QFile>

template<typename T>
T read_type(QFile& file) {
    uchar size = sizeof(T);

    auto buffer = file.read(size);
    if (is_little_endian) {
        for (uchar i = 0; i < size / 2; i++) {
            auto j    = size - 1 - i;
            auto t    = buffer[i];
            buffer[i] = buffer[j];
            buffer[j] = t;
        }
    }

    return *((T*) buffer.data());
}

template<typename T>
T peek_type(QFile& file) {
    uchar size = sizeof(T);

    auto buffer = file.peek(size);
    if (is_little_endian) {
        for (uchar i = 0; i < size / 2; i++) {
            auto j    = size - 1 - i;
            auto t    = buffer[i];
            buffer[i] = buffer[j];
            buffer[j] = t;
        }
    }

    return *((T*) buffer.data());
}

template<typename T>
void write_type(QFile& file, T value) {
    uchar size = sizeof(T);
    char  buffer[16];

    for (auto i = 0; i < size; i++)
        buffer[i] = *((char*) &value + i);

    if (is_little_endian) {
        for (uchar i = 0; i < size / 2; i++) {
            auto j    = size - 1 - i;
            auto t    = buffer[i];
            buffer[i] = buffer[j];
            buffer[j] = t;
        }
    }

    file.write(buffer, size);
}

#endif // __FILE_UTILS_H__