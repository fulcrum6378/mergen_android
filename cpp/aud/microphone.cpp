#include "../global.hpp"
#include "microphone.hpp"

Microphone::Microphone() {}

bool Microphone::Start() {
#if AUD_IN_SAVE
    std::string path = cacheDir + "aud.pcm";
    remove(path.c_str());
    test = std::ofstream(path, std::ios::binary);
#endif
    return true;
}

bool Microphone::Stop() {
#if AUD_IN_SAVE
    test.close();
#endif
    return true;
}

Microphone::~Microphone() {}
