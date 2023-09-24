#include "segmentation.h"

Segmentation::Segmentation(std::pair<int16_t, int16_t> dimensions) {
    w = dimensions.first;
    h = dimensions.second;
}

void Segmentation::Process(AImage *image) {
    locked = true;
    /*std::ofstream test; // #include <fstream> #include <ios>
    test.open("/data/data/ir.mahdiparastesh.mergen/cache/test.yuv",
              std::ios::out | std::ios::binary);*/
    int8_t arr[h][w][3];

    // bring separate YUV data into the above multidimensional array of pixels `arr`
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
            //test.put(pY[x]); test.put(pU[uv_offset]); test.put(pV[uv_offset]);
            arr[y][x][0] = pY[x];
            arr[y][x][1] = pU[uv_offset];
            arr[y][x][2] = pV[uv_offset];
        }
    }

    // segmentation begins
    uint32_t status[h][w];
    std::vector<Segment> segments;
    int16_t thisY = 0, thisX = 0;
    bool foundSthToAnalyse = true;
    while (foundSthToAnalyse) {
        foundSthToAnalyse = false;
        for (int16_t y = thisY; y < h; y++) {
            for (int16_t x = (y == thisY) ? thisX : 0; x < w; x++)
                if (status[y][x] == 0) {
                    foundSthToAnalyse = true;
                    thisY = y;
                    thisX = x;
                    break;
                }
            if (!foundSthToAnalyse) break;
        }
        if (!foundSthToAnalyse) break;
        Segment segment;
        NeighboursOf(thisY, thisX, &segment);
        segments.push_back(segment);
    }

    AImage_delete(image); // test.close();
    locked = false;
}

void Segmentation::NeighboursOf(int16_t y, int16_t x, Segment *seg) {

}

Segmentation::~Segmentation() {
}
