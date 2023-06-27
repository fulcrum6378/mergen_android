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
    LoadManifest();
    while (active_) {
        //LOGW("QUEUER: I AM ACTIVE!!!");
        lock.lock();
        if (!input.empty()) Handle();
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Queuer::LoadManifest() {
    // C++ has no split function; instead do "s.substr(0, s.find(delimiter));" on std::string
    manifest = std::map<uint8_t, Sense>();
    manifest[0] = Sense{
            0, SenseType::REW, 1, 0, 1,
            0, 0, 0, {0, 0, 0}
    };
    manifest[1] = Sense{
            1, SenseType::VIS, 518400, 1, 518401,
            .75, .5, .8, {9, 26, 5}
    };
    manifest[2] = Sense{
            2, SenseType::AUD, 96000, 518401, 614401,
            .5, .95, -1, {9, 3, 3}
    };
    /*manifest[3] = Sense{
            3, SenseType::HPT,
    };*/
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
