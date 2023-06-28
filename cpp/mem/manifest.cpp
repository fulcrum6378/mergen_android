#include "manifest.h"

/** Sensors */
std::map<int8_t, Interaction> *Manifest::input;
/** Controls/Expressions */
std::map<int8_t, Interaction> *Manifest::output;

void Manifest::create() {
    input = new std::map<int8_t, Interaction>{};
    (*input)[0] = Interaction{
            0, ActType::REW, 0, 0, 0, 0, 0, 0
    };
    (*input)[1] = Interaction{ // back camera
            1, ActType::VIS, .7, .75, -1, 30, 80, 6
    };
    (*input)[2] = Interaction{ // selfie camera
            2, ActType::VIS, 0, .9, 1, 6, 6, 3
    };
    (*input)[3] = Interaction{ // microphone
            3, ActType::AUD, .4, 1, 0, 2, 4, 2
    };
    (*input)[4] = Interaction{ // touch
            4, ActType::HPT, 0, 0, 1, 1080, 2340, 2
    };

    output = new std::map<int8_t, Interaction>{};
    (*input)[-1] = Interaction{ // monitor screen
            -1, ActType::VIS, 0, 0, 1, 1080, 2131, 2
    };
    (*input)[-2] = Interaction{ // speaker
            -2, ActType::AUD, .55, 1, 0, 10, 4, 5
    };
    (*input)[-3] = Interaction{ // vibration motor (inaccurate)
            -3, ActType::MOV, -.5, -.75, 0, 15, 15, 5
    };
}

void Manifest::destroy() {
    delete &input;
    input = nullptr;
    delete &output;
    output = nullptr;
}
