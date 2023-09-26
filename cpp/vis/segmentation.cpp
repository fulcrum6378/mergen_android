#include <sys/resource.h>

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

        Segment seg{segments.size() + 1};
        stack.push_back(new int16_t[3]{thisY, thisX, 0});
        int32_t last;
        while ((last = stack.size() - 1) != 0) {
            int16_t y = stack[last][0], x = stack[last][1], dr = stack[last][2];
            if (dr == 0) {
                seg.p.push_back(std::pair(y, x));
                status[y][x] = seg.id;
                // left
                stack[last][2]++;
                if (x > 0 && status[y][x - 1] == 0 && CompareColours(arr[y][x], arr[y][x - 1])) {
                    stack.push_back(new int16_t[3]{y, static_cast<int16_t>(x - 1), 0});
                    continue;
                }
            }
            if (dr <= 1) { // top
                stack[last][2]++;
                if (y > 0 && status[y - 1][x] == 0 && CompareColours(arr[y][x], arr[y - 1][x])) {
                    stack.push_back(new int16_t[3]{static_cast<int16_t>(y - 1), x, 0});
                    continue;
                }
            }
            if (dr <= 2) { // right
                stack[last][2]++;
                if (x < (w - 1) && status[y][x + 1] == 0 &&
                    CompareColours(arr[y][x], arr[y][x + 1])) {
                    stack.push_back(new int16_t[3]{y, static_cast<int16_t>(x + 1), 0});
                    continue;
                }
            }
            if (dr <= 3) { // bottom
                stack[last][2]++;
                if (y < (h - 1) && status[y + 1][x] == 0 &&
                    CompareColours(arr[y][x], arr[y + 1][x])) {
                    stack.push_back(new int16_t[3]{static_cast<int16_t>(y + 1), x, 0});
                    continue;
                }
            }
            stack.pop_back();
        }
        segments.push_back(seg);
    }

    // dissolution
    if (min_seg > 1) {
        for (int32_t seg = segments.size() - 1; seg > 0; seg--)
            if (segments[seg].p.size() < min_seg) {
                absorber_index = find_a_segment_to_dissolve_in(segments[seg])
                if (absorber_index is None) continue
                absorber:
                Segment = segments[status[*absorber_index] - 1]
                for (p in segments[seg].p) {
                    absorber.p.append(p)
                    status[*p] = absorber.id
                }
                segments.pop(seg)
            }
    }

    auto t1 = std::chrono::system_clock::now();
    LOGI("Segmentations time: %lld",
         std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
    LOGI("Total segments: %zu", segments.size());

    std::sort(segments.begin(), segments.end(), SegmentSorter());

    LOGI("#0 : %zu", segments[0].p.size());
    LOGI("#1 : %zu", segments[1].p.size());
    LOGI("#2 : %zu", segments[2].p.size());
    int last = segments.size() - 1;
    LOGI("#%d : %zu", last, segments[last].p.size());
    LOGI("----------------------");

    AImage_delete(image); // test.close();
    Reset();
    locked = false;
}

bool Segmentation::CompareColours(uint8_t a[3], uint8_t b[3]) {
    // you can try this: 256 - static_cast<uint8_t>(a - b)
    return abs(a[0] - b[0]) <= 4 && abs(a[1] - b[1]) <= 4 && abs(a[2] - b[2]) <= 4;
}

int16_t** Segmentation::FindASegmentToDissolveIn(Segment *seg) {
    if (seg->p[0].first > 0)
        return &{static_cast<int16_t>(seg->p[0].first - 1), seg->p[0].second};
    /*if (seg_.p[0][1] > 0)
    return seg->p[0][0], seg_.p[0][1] - 1
    if (seg->p[len(seg_.p) - 1][0] < dim - 1)
    return seg_.p[len(seg_.p) - 1][0] + 1, seg_.p[len(seg_.p) - 1][1]
    if (seg_.p[len(seg_.p) - 1][1] < dim - 1)
    return seg_.p[len(seg_.p) - 1][0], seg_.p[len(seg_.p) - 1][1] + 1*/
    return nullptr;
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
