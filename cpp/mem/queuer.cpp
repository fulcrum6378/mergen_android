#include "queuer.h"
#include "../global.h"

Queuer::Queuer() : std::thread(&Queuer::Run, this) {
    active_ = true;
    detach(); // don't use "join()"
    /*#include <sstream>
    std::stringstream ss;
    ss << std::this_thread::get_id();
    LOGE("%s", ss.str().c_str());*/
} // all executed in the main thread.

void Queuer::Run() {
    while (active_) {
        //LOGW("QUEUER: I AM ACTIVE!!!");
        lock.lock();
        if (!input.empty()) Handle();
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Current time in microseconds
int64_t Queuer::Now() {
    return std::chrono::system_clock::now().time_since_epoch().count();
}

void Queuer::Input(uint8_t sense, uint8_t *ref, int64_t time) {
}

void Queuer::Handle() {
    int8_t i = -1;
    bool found;
    do {
        i++;
        found = input[i].size() == 3;
    } while (i < input.size() && !found);

    input[i];
}

Queuer::~Queuer() {
    active_ = false;
}
