#include <chrono>

#include "segmentation_b.hpp"

using namespace std;

Segmentation::Segmentation(AAssetManager *assets) : edgeDetection(
        new EdgeDetection(assets, reinterpret_cast<uint32_t *>(img), reinterpret_cast<uint32_t *>(edges))
) {}

void Segmentation::Process(AImage *image) {
    locked = true;

    // 1. loading; bring separate YUV data into the multidimensional array of pixels `img`
    auto checkPoint = chrono::system_clock::now();
    AImageCropRect srcRect;
    AImage_getCropRect(image, &srcRect);
    int32_t yStride, uvStride;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneRowStride(image, 0, &yStride);
    AImage_getPlaneRowStride(image, 1, &uvStride);
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &uPixel, &uLen);
    AImage_getPlaneData(image, 2, &vPixel, &vLen);
    int32_t uvPixelStride;
    AImage_getPlanePixelStride(image, 1, &uvPixelStride);

    for (uint16_t y = 0u; y < H; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;
        int32_t uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (uint16_t x = 0u; x < W; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;
            img[y][x] = (pY[x] << 16) | (pU[uv_offset] << 8) | pV[uv_offset];
        }
    }
    AImage_delete(image);
    auto delta1 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - checkPoint).count();

    // 2. edge_detection
    checkPoint = chrono::system_clock::now();
    edgeDetection->runCommandBuffer();
    auto delta2 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - checkPoint).count();

    // 3. analyses; display the debug results in the window
    checkPoint = chrono::system_clock::now();
#if VIS_ANALYSES
    if (ANativeWindow_lock(analyses, &analysesBuf, nullptr) == 0) {
        auto *out = static_cast<uint32_t *>(analysesBuf.bits);
        out += analysesBuf.width - 1; // images are upside down
        for (int32_t y = 0; y < analysesBuf.height; y++) {
            for (int32_t x = 0; x < analysesBuf.width; x++)
                out[x * analysesBuf.stride] = (edges[y][x] == 0u) ? 0x00000000u : 0xFF00FF00u;
            out--; // move to next line in memory
        }
        ANativeWindow_unlockAndPost(analyses);
    }
#endif
    auto delta3 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - checkPoint).count();

    //TODO

    // summary: loading + edge_detection + analyses
    LOGI("Segmentation: %02lld + %02lld + %02lld => %04lld",
         delta1, delta2, delta3,
         delta1 + delta2 + delta3);

    locked = false;
}

Segmentation::~Segmentation() {
    delete edgeDetection;
}
