#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

#include <media/NdkImage.h>
#include <utility>
#include <vector>

#include "global.h"

struct Segment {
    // starting from 1
    uint32_t id;
    // pixel coordinates
    std::vector<std::pair<int16_t, int16_t>> p;
    // average colour
    uint8_t m[3];
};

class Segmentation {
private:
    int16_t w, h;  // width, height

    void NeighboursOf(int16_t y, int16_t x, Segment *seg);

public:
    bool locked = false;

    Segmentation(std::pair<int16_t, int16_t> dimensions);

    void Process(AImage *image);

    ~Segmentation();
};

#endif //VIS_SEGMENTATION_H
