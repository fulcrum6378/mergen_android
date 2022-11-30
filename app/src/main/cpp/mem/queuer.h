#ifndef MEM_QUEUER_H
#define MEM_QUEUER_H

#include <map>
#include <mutex>
#include <thread>

class Queuer : /*public*/ std::thread {
public:
    Queuer();

    ~Queuer();

    std::map<int64_t, std::map<int8_t, int8_t[]>> input;
    std::mutex lock;
    //std::map<int8_t, ???> manifest;

private:
    bool active_{false};

    void Run();

    void Handle();
};

#endif //MEM_QUEUER_H
