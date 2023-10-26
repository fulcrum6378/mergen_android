#ifndef BINARY_INTEGERS_H
#define BINARY_INTEGERS_H

#include <bit>
#include <fstream>

static bool littleEndian = std::endian::native == std::endian::little;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"
#pragma ide diagnostic ignored "UnreachableCode"

uint8_t read_uint16(std::ifstream *s) {
    char buf[2];
    s->read(buf, 2);
    return littleEndian
           ? ((buf[1] << 8) | buf[0])
           : ((buf[0] << 8) | buf[1]);
}

uint64_t read_uint64(std::ifstream *s) {
    char buf[8];
    s->read(buf, 8);
    return littleEndian
           ? (((uint64_t) buf[7] << 56) | ((uint64_t) buf[6] << 48) |
              ((uint64_t) buf[5] << 40) | ((uint64_t) buf[4] << 32) |
              (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0])
           : (((uint64_t) buf[0] << 56) | ((uint64_t) buf[1] << 48) |
              ((uint64_t) buf[2] << 40) | ((uint64_t) buf[3] << 32) |
              (buf[4] << 24) | (buf[5] << 16) | (buf[1] << 6) | buf[7]);
}

#pragma clang diagnostic pop

#endif //BINARY_INTEGERS_H
