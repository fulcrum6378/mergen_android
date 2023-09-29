#ifndef MEM_MANIFEST_H
#define MEM_MANIFEST_H

#include <cstdint>
#include <map>

enum class ActType : uint8_t {
    AUD, HPT, MOV, REW, VIS
};

struct Interaction {
    /** Input/sense IDs must be positive and Output/{control/expression} IDs must be negative. */
    int8_t id;
    [[maybe_unused]] ActType type;
    /** Range -1..1 ; 0 means centre */
    [[maybe_unused]] float x, y, z;
    /** In pixels */
    [[maybe_unused]] uint16_t width, height, depth;
};

class Manifest {
public:
    static std::map<int8_t, Interaction> *input;
    static std::map<int8_t, Interaction> *output;

    static void create();

    static void destroy();
};

/**
 * The two constructs {class, struct} are identical in C++ except that in structs the default
 * accessibility is public, whereas in classes the default is private.
 */

#endif //MEM_MANIFEST_H
