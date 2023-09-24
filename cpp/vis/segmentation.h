#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

#include <media/NdkImage.h>
#include <utility>

#include "global.h"

class Segmentation {
private:
    int16_t w, h;  // width, height

public:
    bool locked = false;

    Segmentation(std::pair<int16_t, int16_t> dimensions);

    void Process(AImage *image);

    ~Segmentation();
};

#endif //VIS_SEGMENTATION_H
