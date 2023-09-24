#include <fstream>
#include <ios>

#include "segmentation.h"

Segmentation::Segmentation(std::pair<int16_t, int16_t> dimensions) {
    w = dimensions.first;
    h = dimensions.second;
}

void Segmentation::Process(AImage *image) {
    locked = true;

    std::ofstream test;
    test.open("/data/data/ir.mahdiparastesh.mergen/cache/test.yuv",
              std::ios::out | std::ios::binary);

    AImageCropRect srcRect;
    AImage_getCropRect(image, &srcRect);
    int32_t yStride, uvStride;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneRowStride(image, 0, &yStride);
    AImage_getPlaneRowStride(image, 1, &uvStride);
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &vPixel, &vLen);
    AImage_getPlaneData(image, 2, &uPixel, &uLen);
    int32_t uvPixelStride;
    AImage_getPlanePixelStride(image, 1, &uvPixelStride);

    for (int16_t y = 0; y < h; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;
        int32_t uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (int16_t x = 0; x < w; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;
            test.put(pY[x]);
            test.put(pU[uv_offset]);
            test.put(pV[uv_offset]);
        }
    }

    AImage_delete(image);
    test.close();
    locked = false;
}

Segmentation::~Segmentation() {
}
