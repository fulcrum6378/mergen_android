#ifndef VIS_SEGMENTATION_B_H
#define VIS_SEGMENTATION_B_H

#include <android/native_window.h>
#include <atomic>
#include <media/NdkImage.h>

#include "edge_detection.hpp"

class Segmentation {
public:
    explicit Segmentation(AAssetManager *assets);

    void Process(AImage *image);

    ~Segmentation();


    std::atomic<bool> locked = false;
#if VIS_ANALYSES
    ANativeWindow *analyses = nullptr;
    ANativeWindow_Buffer analysesBuf{};
#endif

private:
    // multidimensional array of pixels
    uint32_t img[H][W]{};
    // a map that highlights marginal pixels
    uint32_t edges[H][W]{};
    // pointer to the only instance of EdgeDetection
    EdgeDetection *edgeDetection;
};

#endif //VIS_SEGMENTATION_B_H
