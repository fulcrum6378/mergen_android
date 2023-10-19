#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

#include <atomic>
#include <jni.h>
#include <list>
#include <media/NdkImage.h>
#include <unordered_map>
#include <utility>
#include <unordered_set>
#include <vector>

#include "visual_stm.h"

// height of an image frame
#define H 1088
// width of an image frame
#define W 1088
// minimum allowed number of pixels for a segment to contain
#define MIN_SEG_SIZE 1
// maximum allowed segments to be stored in the short-term memory
#define MAX_SEGS 10

struct Segment {
    // starting from 1
    uint32_t id;
    // pixel coordinates
    std::list<uint32_t> p;
    // average colour
    uint8_t *m;
    // boundaries and dimensions
    uint16_t min_y, min_x, max_y, max_x, w, h;
    // border pixels
    std::unordered_set<SHAPE_POINT_T> border;
};

/**
 * Image Segmentation, using a Region-Growing method
 *
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/segmentation/region_growing_4.py">
 * Region Growing 4 (image segmentation)</a>
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/tracing/comprehender_rg4.py">
 * Comprehender (image tracing)</a>
 *
 * Dimensions are defined explicitly in order to avoid type-casting complications.
 */
class Segmentation {
private:
    // multidimensional array of pixels + last byte indicates whether it is a border pixel or not.
    uint32_t arr[H][W]{}; // four bytes: b(1=true,0=false), Y, U, V
    // maps pixels to their Segment IDs
    uint32_t status[H][W]{};
    // a vector containing all Segments
    std::vector<Segment> segments;
    // simulates recursive programming (vector is always better for it than list!)
    std::vector<uint16_t *> stack;
    // maps IDs of Segments to their pointers
    std::unordered_map<uint32_t, Segment *> s_index;
    // visual short-term memory (output)
    VisualSTM *stm;

    JavaVM *jvm_;
    jobject main_;
    jmethodID *jmSignal_;

    static bool CompareColours(uint32_t a, uint32_t b);

    static uint32_t FindPixelOfASegmentToDissolveIn(Segment *seg);

    // Checks if this pixel is in border.
    void CheckIfBorder(uint16_t y1, uint16_t x1, uint16_t y2, uint16_t x2);

    // Recognises this pixel as border.
    void SetAsBorder(uint16_t y, uint16_t x);

public:
    std::atomic<bool> locked = false;

    Segmentation(VisualSTM *stm, JavaVM *jvm, jobject main, jmethodID *jmSignal);

    /** Main processing function of Segmentation which execute all the necessary jobs.
     * do NOT refer to `debugMode_` in Camera. */
    void Process(AImage *image, const bool *recording, int8_t debugMode);

    ~Segmentation();
};

#endif //VIS_SEGMENTATION_H
