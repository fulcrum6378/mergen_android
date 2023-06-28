#ifndef MEM_QUEUER_H
#define MEM_QUEUER_H

#include <map>
#include <mutex>
#include <thread>

#include "manifest.h"

class Queuer : std::thread {
public:
    Queuer();

    static int64_t Now();

    void Input(uint8_t sense, uint8_t *ref, int64_t time);

    ~Queuer();

private:
    bool active_{false};
    std::map<int64_t, std::map<uint8_t, int8_t[]>> input;
    std::mutex lock;

    void Run();

    void Handle();
};

#endif //MEM_QUEUER_H
