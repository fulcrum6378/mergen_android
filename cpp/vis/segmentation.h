#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

#include <media/NdkImage.h>
#include <utility>
#include <vector>

#include "global.h"

struct Segment {
    // starting from 1
    int32_t id;
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

/**
 * Image Segmentation, using a Region-Growing method
 *
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/segmentation/region_growing_4.py">
 * Region Growing 4 (image segmentation)</a>
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/tracing/surrounder_rg3.py">
 * Surrounder (image tracing)</a>
 *
 * Dimensions are defined explicitly in order to avoid type-casting complications.
 */
class Segmentation {
private:
    // configurations
    const int16_t h = 1088, w = 1088;  // width, height
    // minimum allowed number of pixels for a segment to contain
    const int32_t min_seg = 30;

    uint8_t arr[1088][1088][3];
    int32_t status[1088][1088];
    std::vector<Segment> segments;
    std::vector<int16_t *> stack;

    bool CompareColours(uint8_t a[3], uint8_t b[3]);

    int16_t** FindASegmentToDissolveIn(Segment *seg);

    void Reset();

public:
    bool locked = false;

    Segmentation();

    void Process(AImage *image);

    ~Segmentation();
};

#endif //VIS_SEGMENTATION_H
