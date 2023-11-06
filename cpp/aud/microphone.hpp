#ifndef AUD_MICROPHONE_H
#define AUD_MICROPHONE_H

#include <fstream>

#define AUD_IN_SAVE true

class Microphone {
private:
    std::ofstream test;

public:
    Microphone();

    bool Start();

    bool Stop();

    ~Microphone();
};

#endif //AUD_MICROPHONE_H
