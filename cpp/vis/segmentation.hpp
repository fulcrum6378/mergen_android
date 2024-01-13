#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

#include <array>
#include <atomic>
#include <jni.h>
#include <map>
#include <media/NdkImage.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "analyses.hpp"
#include "visual_stm.hpp"

// height of an image frame
#define H 720u
// width of an image frame
#define W 720u
// maximum allowed segments to be analysed extensively
#define MAX_SEGS 20u
// radii for searching through Volatile Indices
#define Y_RADIUS 15
#define U_RADIUS 10
#define V_RADIUS 10
#define R_RADIUS 15

// debug the results using VisualSTM
#define VISUAL_STM false
// debug the results using Analyses
#define ANALYSES true

/**
 * Image Segmentation, using a Region-Growing method
 *
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/segmentation/region_growing_4.py">
 * Region Growing 4 (image segmentation)</a>
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/tracing/comprehender_rg4.py">
 * Comprehender (image tracing)</a>
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/perception/tracking.py">
 * Tracking (object tracking)</a>
 */
class Segmentation {
public:
    Segmentation(JavaVM *jvm, jobject main, jmethodID *jmSignal, Analyses **analyses);

    /** Main processing function of Segmentation which execute all the necessary jobs.
     * do NOT refer to `debugMode_` in Camera. */
    void Process(AImage *image, const bool *recording, int8_t debugMode);

    ~Segmentation();


    std::atomic<bool> locked = false;

private:
    static bool CompareColours(uint8_t (*a)[3], uint8_t (*b)[3]);

    static uint32_t FindPixelOfASegmentToDissolveIn(Segment *seg);

    // Checks if this pixel is in border.
    void CheckIfBorder(uint16_t y1, uint16_t x1, uint16_t y2, uint16_t x2);

    // Recognises this pixel as border.
    void SetAsBorder(uint16_t y, uint16_t x);


    // multidimensional array of pixels
    uint8_t arr[H][W][3]{};
    // maps pixels to their Segment IDs
    uint32_t status[H][W]{};
    // maps pixels to their status of being border or not
    uint8_t b_status[H][W]{};
    // a vector containing all Segments of { current frame | previous frame }
    std::vector<Segment> segments, prev_segments;
    // simulates recursive programming (vector is always better for it than list!)
    std::vector<std::array<uint16_t, 3u>> stack;
    // maps IDs of Segments to their pointers
    std::unordered_map<uint32_t, Segment *> s_index;

    // 8-bit volatile indices (those preceding with `_` temporarily contain indices of current frame)
    std::map<uint8_t, std::unordered_set<uint16_t>> yi, _yi, ui, _ui, vi, _vi;
    // 16-bit volatile indices
    std::map<uint16_t, std::unordered_set<uint16_t>> ri, _ri;
    // helper containers for finding nearest candidates while object tracking
    std::unordered_set<uint16_t> a_y, a_u, a_v, a_r;
    // a final map for tracking visual objects and explaining their differences
    std::unordered_map<uint16_t, std::vector<int32_t>> diff;

#if VISUAL_STM
    VisualSTM *stm;
#endif
    JavaVM *jvm_;
    jobject main_;
    jmethodID *jmSignal_;
    Analyses **analyses_;
};

#endif //VIS_SEGMENTATION_H
