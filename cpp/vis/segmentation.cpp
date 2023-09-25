#include "../global.h"
#include "segmentation.h"

Segmentation::Segmentation() {
}

void Segmentation::Process(AImage *image) {
    locked = true;
    /*std::ofstream test; // #include <fstream> #include <ios>
    test.open("/data/data/ir.mahdiparastesh.mergen/cache/test.yuv",
              std::ios::out | std::ios::binary);*/
    auto t0 = std::chrono::system_clock::now();

    // bring separate YUV data into the multidimensional array of pixels `arr`
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
    LOGI("%zu", segments.size());
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
            if (foundSthToAnalyse) break;
        }
        if (!foundSthToAnalyse) break;
        Segment segment{static_cast<uint32_t>(segments.size())};
        NeighboursOf(thisY, thisX, &segment);
        segments.push_back(segment);
    }

    // Dissolution

    auto t1 = std::chrono::system_clock::now();
    LOGI("%lld", std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
    LOGI("%zu", segments.size());

    AImage_delete(image); // test.close();
    Reset();
    locked = false;
}

void Segmentation::NeighboursOf(int16_t y, int16_t x, Segment *seg) {
    seg->p.push_back(std::pair(y, x));
    status[y][x] = seg->id;
    if (x > 0 && status[y][x - 1] == 0 && CompareColours(arr[y][x], arr[y][x - 1])) // left
        NeighboursOf(y, x - 1, seg);
    if (y > 0 && status[y - 1][x] == 0 && CompareColours(arr[y][x], arr[y - 1][x])) // top
        NeighboursOf(y - 1, x, seg);
    if (x < (w - 1) && status[y][x + 1] == 0 && CompareColours(arr[y][x], arr[y][x + 1])) // right
        NeighboursOf(y, x + 1, seg);
    if (y < (h - 1) && status[y + 1][x] == 0 && CompareColours(arr[y][x], arr[y + 1][x])) // bottom
        NeighboursOf(y + 1, x, seg);
}

bool Segmentation::CompareColours(uint8_t a[3], uint8_t b[3]) {
    return abs(a[0] - b[0]) <= 4 && abs(a[1] - b[1]) <= 4 && abs(a[2] - b[2]) <= 4;
}

void Segmentation::Reset() {
    // https://stackoverflow.com/questions/23039134/how-to-use-memset-function-in-two-dimensional-
    // array-for-initialization-of-member
    memset(arr, 0, sizeof(arr)); // these might not work correctly
    memset(status, 0, sizeof(status));
    segments.clear();
}

Segmentation::~Segmentation() {
}
