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
        LOGW("QUEUER: I AM ACTIVE!!!");
        lock.lock();
        if (!input.empty()) Handle();
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Queuer::LoadManifest() {
    // C++ has no split function; instead do "s.substr(0, s.find(delimiter));" on std::string
    manifest = std::map<int8_t, Sense>();
    manifest[0] = Sense{
            SenseType::REW, 1, 0, 1,
            0, 0, 0, 0, 0, 0
    };
    manifest[1] = Sense{
            SenseType::VIS, 518400, 1, 518401,
            .75, .5, .8, 9, 26, 5
    };
    manifest[2] = Sense{
            SenseType::AUD, 192, 518401, 518594,
            .5, .95, -1, 9, 3, 3
    };
    /*manifest[3] = Sense{
            SenseType::HPT,
    };*/
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
