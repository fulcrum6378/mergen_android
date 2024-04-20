#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

#include <android/native_window.h>
#include <array>
#include <atomic>
#include <jni.h>
#include <map>
#include <media/NdkImage.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "global.hpp"
#include "segment.hpp"

// maximum allowed segments to be extensively analysed
#define MAX_SEGS 40u
// radii for searching through Volatile Indices
#define Y_RADIUS 24
#define U_RADIUS 16
#define V_RADIUS 16
#define R_RADIUS 24

/**
 * Image Segmentation, using a Region-Growing method (DEPRECATED)
 *
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/segmentation/region_growing_4.py">
 * Region Growing 4 (image segmentation)</a>
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/tracing/comprehender_rg4.py">
 * Comprehender (image tracing)</a>
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/perception/tracking.py">
 * Tracking (object tracking)</a>
 */
class [[deprecated]] Segmentation {
public:
    Segmentation(JavaVM *jvm, jobject main, jmethodID *jmSignal);

    /** Main processing function of Segmentation which execute all the necessary jobs.
     * do NOT refer to `debugMode_` in Camera. */
    void Process(AImage *image, const bool *recording);

    ~Segmentation();


    std::atomic<bool> locked = false;
#if VIS_ANALYSES
    ANativeWindow *analyses = nullptr;
    ANativeWindow_Buffer analysesBuf;
#endif
#if VIS_SEG_MARKERS
    jmethodID jmSegMarker = nullptr;
#endif

private:
    static bool CompareColours(
            uint8_t (*a)[3], uint8_t (*b)[3]
#if SEG_BASE_COLOUR
            , uint8_t (*base)[3]
#endif
    );

    static uint32_t FindPixelOfASegmentToDissolveIn(Segment *seg);

    // Checks if this pixel is in border.
    void CheckIfBorder(uint16_t y1, uint16_t x1, uint16_t y2, uint16_t x2);

    // Recognises this pixel as border.
    void SetAsBorder(uint16_t y, uint16_t x);


    /*** IMAGE SEGMENTATION ***/

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

    /*** OBJECT TRACKING ***/

    // incrementer of the segments IDs of the volatile indices (max: 32767u but not signed)
    uint16_t sidInc = 0u;
    // 8-bit volatile indices (those preceding with `_` temporarily contain indices of current frame)
    std::map<uint8_t, std::unordered_set<uint16_t>> yi, _yi, ui, _ui, vi, _vi;
    // 16-bit volatile indices
    std::map<uint16_t, std::unordered_set<uint16_t>> ri, _ri;
    // helper containers for finding nearest candidates while object tracking
    std::unordered_set<uint16_t> a_y, a_u, a_v, a_r;
    // a final map for tracking visual objects and explaining their differences
    std::unordered_map<uint16_t, std::vector<int32_t>> diff;

    /*** DEBUGGING ***/

    JavaVM *jvm_;
    jobject main_;
    jmethodID *jmSignal_;
#if VIS_SEG_MARKERS
    // data to be sent to SegmentMarkers.java
    jlong segMarkers[MAX_SEGS]{};
#endif
};

#endif //VIS_SEGMENTATION_H
