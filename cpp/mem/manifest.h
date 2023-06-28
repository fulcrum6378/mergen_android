#ifndef MEM_MANIFEST_H
#define MEM_MANIFEST_H

#include <cstdint>
#include <map>

enum class ActType : int8_t {
    AUD, HPT, MOV, REW, VIS
};

struct Interaction {
    uint8_t id;
    ActType type;
    float x, y, z; // -1..1
    uint16_t width, height, depth; // in pixels
};

class Manifest {
public:
    static std::map<uint8_t, Interaction> *senses;
    static std::map<uint8_t, Interaction> *controls;

    static void create();

    static void destroy();
};

/**
 * The two constructs {class, struct} are identical in C++ except that in structs the default
 * accessibility is public, whereas in classes the default is private.
 */

#endif //MEM_MANIFEST_H
