#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

#include <list>
#include <media/NdkImage.h>
#include <utility>
#include <vector>

#include "global.h"

struct Segment {
    // starting from 1
    uint32_t id;
    // pixel coordinates
    std::list<uint32_t> p;
    // average colour
    uint8_t *m;
    // dimensions
    uint16_t min_y, min_x, max_y, max_x, w, h;
    // border pixels
    std::list<std::pair<float, float>> border;
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
    const uint16_t h = 1088, w = 1088;  // width, height
    // minimum allowed number of pixels for a segment to contain
    const uint32_t min_seg = 30;

    // multidimensional array of pixels
    uint32_t arr[1088][1088];
    // maps pixels to their Segment IDs + the first 2 bits indicate whether it is a border pixel or not.
    uint32_t status[1088][1088];
    // vector of all Segments
    std::vector<Segment> segments;
    // simulates recursive programming
    std::vector<uint16_t *> stack;
    // maps IDs of Segments to themselves
    std::unordered_map<uint32_t, Segment*> s_index;

    bool CompareColours(uint32_t a, uint32_t b);

    uint32_t FindASegmentToDissolveIn(Segment *seg);

    // Checks if this pixel is in border.
    void CheckIfBorder(Segment* seg, uint16_t y, uint16_t x);

    // Recursively checks if neighbours are border pixels. Directions range are 0..7.
    void CheckNeighbours(Segment* seg, uint16_t y, uint16_t x, int8_t avoidDr);

    // Checks if this is a border pixel and not detected before.
    bool IsNextB(Segment* org_s, uint16_t y, uint16_t x);

    void Reset();

public:
    bool locked = false;

    Segmentation();

    void Process(AImage *image);

    ~Segmentation();
};

#endif //VIS_SEGMENTATION_H
