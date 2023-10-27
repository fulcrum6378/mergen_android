#ifndef VIS_PERCEPTION_H
#define VIS_PERCEPTION_H

#include "visual_stm.hpp"

class Perception {
public:
    VisualSTM *stm;

    Perception() : stm(new VisualSTM()) {}

    ~Perception() {
        delete stm;
    }
};

#endif //VIS_PERCEPTION_H
