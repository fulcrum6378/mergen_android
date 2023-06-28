#ifndef MANIFEST_H
#define MANIFEST_H

#include <cstdint>

enum class SenseType : int8_t {
    AUD, HPT, MOV [[maybe_unused]], REW, VIS
};

struct Sense {
    uint8_t id;
    SenseType type;
    int32_t bufferSize, bufferStart, bufferEnd;
    float x, y, z;
    int32_t dimensions[3];
};

std::map<uint8_t, Sense> manifest;

void fuck() {
    manifest[0] = Sense{
            0, SenseType::REW, 1, 0, 1,
            0, 0, 0, {
                    0, 0, 0}
    };
    manifest[1] = Sense{
            1, SenseType::VIS, 518400, 1, 518401,
            .75, .5, .8, {9, 26, 5}
    };
    manifest[2] = Sense{
            2, SenseType::AUD, 96000, 518401, 614401,
            .5, .95, -1, {9, 3, 3}
    };
    manifest[3] = Sense{
            3, SenseType::HPT, 100,
    };
}

const Sense *senses = new Sense[]{
        Sense{
                0, SenseType::REW, 1, 0, 1,
                0, 0, 0, {0, 0, 0}
        },
        Sense{
                1, SenseType::VIS, 518400, 1, 518401,
                .75, .5, .8, {9, 26, 5}
        },
        Sense{
                2, SenseType::AUD, 96000, 518401, 614401,
                .5, .95, -1, {9, 3, 3}
        },
        Sense{3, SenseType::HPT, 100,},
};

/**
 * The two constructs {class, struct} are identical in C++ except that in structs the default
 * accessibility is public, whereas in classes the default is private.
 */

#endif //MANIFEST_H
