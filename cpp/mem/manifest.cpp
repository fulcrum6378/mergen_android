#include "manifest.h"

std::map<uint8_t, Interaction> *Manifest::senses;

void Manifest::create() {
    senses = new std::map<uint8_t, Interaction>{};
    (*senses)[0] = Interaction{
            0, ActType::REW, 0, 0, 0, 0, 0, 0
    };
    (*senses)[1] = Interaction{ // back camera
            1, ActType::VIS, .7, .75, -1, 30, 80, 6
    };
    (*senses)[2] = Interaction{ // selfie camera
            2, ActType::VIS, 0, .9, 1, 6, 6, 3
    };
    (*senses)[3] = Interaction{ // microphone
            3, ActType::AUD, .4, 1, 0, 2, 4, 2
    };
    (*senses)[4] = Interaction{ // touch sensor
            4, ActType::HPT, 0, 0, 1, 1080, 2131, 2
    };

    controls = new std::map<uint8_t, Interaction>{};
    (*senses)[0] = Interaction{ // screen
            0, ActType::VIS, 0, 0, 1, 1080, 2131, 2
    };
    (*senses)[1] = Interaction{ // speaker
            1, ActType::AUD, .55, 1, 0, 10, 4, 5
    };
    (*senses)[2] = Interaction{ // vibration motor (inaccurate)
            2, ActType::MOV, -.5, -.75, 0, 15, 15, 5
    };
}

void Manifest::destroy() {
    delete &senses;
    senses = nullptr;
    delete &controls;
    controls = nullptr;
}
