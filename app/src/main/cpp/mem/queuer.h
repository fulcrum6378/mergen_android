#ifndef MEM_QUEUER_H
#define MEM_QUEUER_H

#include <map>
#include <mutex>
#include <thread>

struct Sense;

class Queuer : std::thread {
public:
    Queuer();

    ~Queuer();

    std::map<int64_t, std::map<int8_t, int8_t[]>> input;
    std::mutex lock;

private:
    bool active_{false};
    std::map<int8_t, Sense> manifest;

    void Run();

    void LoadManifest();

    void Handle();
};

enum class SenseType : int8_t {
    REW, AUD, HPT, MOV, VIS
};

/**
 * The two constructs are identical in C++ except that in structs the default accessibility is public,
 * whereas in classes the default is private.
 */
struct Sense {
    SenseType type;
    int32_t bufferSize, bufferStart, bufferEnd;
    float x, y, z;
    int32_t dimensions[3];
};

#endif //MEM_QUEUER_H
