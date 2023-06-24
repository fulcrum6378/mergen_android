#ifndef MEM_QUEUER_H
#define MEM_QUEUER_H

#include <map>
#include <mutex>
#include <thread>

struct Sense;

class Queuer : std::thread {
public:
    Queuer();

    static int64_t Now();

    void Input(uint8_t sense, uint8_t *ref, int64_t time);

    ~Queuer();

private:
    bool active_{false};
    std::map<uint8_t, Sense> manifest;
    std::map<int64_t, std::map<uint8_t, int8_t[]>> input;
    std::mutex lock;

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
    uint8_t id;
    SenseType type;
    int32_t bufferSize, bufferStart, bufferEnd;
    float x, y, z;
    int32_t dimensions[3];
};

#endif //MEM_QUEUER_H
