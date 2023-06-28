#ifndef REW_REWARDER_H
#define REW_REWARDER_H

#include "../mem/manifest.h"

class Rewarder {
public:
    int8_t score; // -128..127

    Rewarder();

    ~Rewarder();
};

#endif //REW_REWARDER_H
