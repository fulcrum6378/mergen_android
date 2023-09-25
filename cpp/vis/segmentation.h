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

struct SegmentSorter {
    inline bool operator()(const Segment &a, const Segment &b) {
        return (a.p.size() > b.p.size());
    }
};

/** Dimensions are defined explicitly in order to avoid type-casting complications. */
class Segmentation {
private:
    const int16_t h = 1088, w = 1088;  // width, height
    uint8_t arr[1088][1088][3];
    uint32_t status[1088][1088];
    std::vector<Segment> segments;
    std::vector<int16_t *> stack;

    bool CompareColours(uint8_t a[3], uint8_t b[3]);

    void Reset();

public:
    bool locked = false;

    Segmentation();

    void Process(AImage *image);

    ~Segmentation();
};

#endif //VIS_SEGMENTATION_H
