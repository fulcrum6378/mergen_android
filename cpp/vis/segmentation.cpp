#include <sys/resource.h>

#include "../global.h"
#include "segmentation.h"

Segmentation::Segmentation() {
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "ConstantConditionsOC"

void Segmentation::Process(AImage *image) {
    locked = true;
    /*std::ofstream test; // #include <fstream> #include <ios>
    test.open("/data/data/ir.mahdiparastesh.mergen/cache/test.yuv",
              std::ios::out | std::ios::binary);*/

    // 1. loading; bring separate YUV data into the multidimensional array of pixels `arr`
    auto t0 = std::chrono::system_clock::now();
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

    for (uint16_t y = 0; y < h; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;
        int32_t uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (uint16_t x = 0; x < w; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;
            arr[y][x] = (pY[x] << 16) | (pU[uv_offset] << 8) | pV[uv_offset];
            //test.put(pY[x]); test.put(pU[uv_offset]); test.put(pV[uv_offset]);
        }
    }
    AImage_delete(image); // test.close();
    auto delta1 = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - t0).count();

    // 2. segmentation
    t0 = std::chrono::system_clock::now();
    uint16_t thisY = 0, thisX = 0;
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

        Segment seg{static_cast<uint32_t>(segments.size() + 1)};
        stack.push_back(new uint16_t[3]{thisY, thisX, 0});
        uint32_t last;
        while ((last = stack.size() - 1) != -1) {
            uint16_t y = stack[last][0], x = stack[last][1], dr = stack[last][2];
            if (dr == 0) {
                seg.p.push_back((y << 16) | x);
                status[y][x] = seg.id;
                // left
                stack[last][2]++;
                if (x > 0 && status[y][x - 1] == 0 && CompareColours(arr[y][x], arr[y][x - 1])) {
                    stack.push_back(new uint16_t[3]{y, static_cast<uint16_t>(x - 1), 0});
                    continue;
                }
            }
            if (dr <= 1) { // top
                stack[last][2]++;
                if (y > 0 && status[y - 1][x] == 0 && CompareColours(arr[y][x], arr[y - 1][x])) {
                    stack.push_back(new uint16_t[3]{static_cast<uint16_t>(y - 1), x, 0});
                    continue;
                }
            }
            if (dr <= 2) { // right
                stack[last][2]++;
                if (x < (w - 1) && status[y][x + 1] == 0 &&
                    CompareColours(arr[y][x], arr[y][x + 1])) {
                    stack.push_back(new uint16_t[3]{y, static_cast<uint16_t>(x + 1), 0});
                    continue;
                }
            }
            if (dr <= 3) { // bottom
                stack[last][2]++;
                if (y < (h - 1) && status[y + 1][x] == 0 &&
                    CompareColours(arr[y][x], arr[y + 1][x])) {
                    stack.push_back(new uint16_t[3]{static_cast<uint16_t>(y + 1), x, 0});
                    continue;
                }
            }
            stack.pop_back();
        }
        segments.push_back(seg);
    }
    auto delta2 = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - t0).count();

    // 3. dissolution
    t0 = std::chrono::system_clock::now();
    if (min_seg > 1) {
        uint32_t absorber_i, size_bef = segments.size();
        Segment *absorber; // not putting a `*` COST 20 SECONDS!
        std::list <int32_t> removal;
        for (int32_t seg = size_bef - 1; seg > -1; seg--)
            if (segments[seg].p.size() < min_seg) {
                absorber_i = FindASegmentToDissolveIn(&segments[seg]);
                if (absorber_i == 0xFFFFFFFF) continue;
                absorber = &segments[status[(absorber_i >> 16) & 0xFF][absorber_i & 0xFF] - 1];
                for (uint32_t &p: segments[seg].p) {
                    absorber->p.push_back(p); // merge()
                    status[(p >> 16) & 0xFF][p & 0xFF] = absorber->id;
                }
            }
        for (int32_t &rem: removal) {
            // https://stackoverflow.com/questions/3487717/erasing-multiple-objects-from-a-stdvector
            segments[rem] = segments.back();
            segments.pop_back();
        }
        LOGI("Total segments: %zu / %zu", segments.size(), size_bef);
    } else
        LOGI("Total segments: %zu", segments.size());
    auto delta3 = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - t0).count();

    // 4. average colours + detect boundaries
    t0 = std::chrono::system_clock::now();
    uint32_t col, l_;
    uint64_t aa, bb, cc;
    bool isFirst;
    uint16_t y, x;
    for (Segment &seg: segments) {
        // average colours of each segment
        aa = 0, bb = 0, cc = 0;
        for (uint32_t &p: seg.p) {
            col = arr[(p >> 16) & 0xFF][p & 0xFF];
            aa += (col >> 16) & 0xFF;
            bb += (col >> 8) & 0xFF;
            cc += col & 0xFF;
        }
        l_ = seg.p.size();
        seg.m = new uint8_t[3]{static_cast<uint8_t>(aa / l_),
                               static_cast<uint8_t>(bb / l_),
                               static_cast<uint8_t>(cc / l_)};

        // detect boundaries (min_y, min_x, max_y, max_x)
        isFirst = true;
        for (uint32_t &p: seg.p) {
            y = (p >> 16) & 0xFF;
            x = p & 0xFF;
            if (isFirst) {
                seg.min_y = y;
                seg.min_x = x;
                seg.max_y = y;
                seg.max_x = x;
            } else {
                if (y < seg.min_y) seg.min_y = y;
                if (x < seg.min_x) seg.min_x = x;
                if (y > seg.max_y) seg.max_y = y;
                if (x > seg.max_x) seg.max_x = x;
            }
            seg.w = (seg.max_x + 1) - seg.min_x;
            seg.h = (seg.max_y + 1) - seg.min_y;
        }

        // index the Segments by their IDs
        s_index[seg.id] = &seg;
    }
    auto delta4 = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - t0).count();

    // std::sort(segments.begin(), segments.end(), SegmentSorter());

    // 5. trace border pixels
    t0 = std::chrono::system_clock::now();
    for (Segment &seg: segments) {
        // find the first encountering border pixel as a checkpoint...
        for (uint32_t &p: seg.p) {
            y = (p >> 16) & 0xFF;
            x = p & 0xFF;
            if (((status[y][x] >> 30) & 1) == 0) CheckIfBorder(&seg, y, x);
            if ((status[y][x] >> 31) == 1) break;
        }
        // then explore its neighbours recursively.
        CheckNeighbours(&seg, y, x, -1);
    }
    auto delta5 = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - t0).count();

    // summary: loading + segmentation + dissolution + average colours + tracing
    LOGI("Delta times: %lld + %lld + %lld + %lld + %lld => %lld",
         delta1, delta2, delta3, delta4, delta5,
         delta1 + delta2 + delta3 + delta4 + delta5);
    LOGI("----------------------------------");

    Reset();
    locked = false;
}

#pragma clang diagnostic pop

bool Segmentation::CompareColours(uint32_t a, uint32_t b) {
    return abs(static_cast<int16_t>(((a >> 16) & 0xFF) - ((b >> 16) & 0xFF))) <= 4 &&
           abs(static_cast<int16_t>(((a >> 8) & 0xFF) - ((b >> 8) & 0xFF))) <= 4 &&
           abs(static_cast<int16_t>((a & 0xFF) - (b & 0xFF))) <= 4;
    // abs() is much more efficient than `256 - static_cast<uint8_t>(a - b)`!
}

uint32_t Segmentation::FindASegmentToDissolveIn(Segment *seg) {
    if (seg->p.size() == 0) LOGI("FUCK");
    uint32_t cor = seg->p.front();
    uint16_t a = (cor >> 16) & 0xFF, b = cor & 0xFF;
    if (a > 0)
        return (static_cast<uint16_t>(a - 1) << 16) | b;
    if (b > 0)
        return (a << 16) | static_cast<uint16_t>(b - 1);
    cor = seg->p.back();
    a = (cor >> 16) & 0xFF, b = cor & 0xFF;
    if (a < h - 1)
        return (static_cast<uint16_t>(a + 1) << 16) | b;
    if (b < w - 1)
        return (a << 16) | static_cast<uint16_t>(b + 1);
    return 0xFFFFFFFF;
}

void Segmentation::CheckIfBorder(Segment *seg, uint16_t y, uint16_t x) {
    status[y][x] |= (1 << 30); // resembling a non-null value
    if ((x == (w - 1) || seg->id != status[y][x + 1] ||  // right
         y == (h - 1) || seg->id != status[y + 1][x] ||  // bottom
         x == 0 || seg->id != status[y][x - 1] ||  // left
         y == 0 || seg->id != status[y - 1][x])) {  // top
        status[y][x] |= (1 << 31);
        seg->border.push_back(std::pair(
                (100.0 / seg->w) * (seg->min_x - x), // fractional X
                (100.0 / seg->h) * (seg->min_y - y)  // fractional Y
        ));
    }
}

void Segmentation::CheckNeighbours(Segment *seg, uint16_t y, uint16_t x, int8_t avoidDr) {
    uint16_t ny = y, nx = x;
    if (avoidDr != 0 && y > 0) { // northern
        ny -= 1;
        if (IsNextB(seg, ny, nx)) CheckNeighbours(seg, ny, nx, 0);
    }
    if (avoidDr != 1 && y > 0 && x < (w - 1)) { // north-eastern
        ny -= 1;
        nx += 1;
        if (IsNextB(seg, ny, nx)) CheckNeighbours(seg, ny, nx, 1);
    }
    if (avoidDr != 2 && x < (w - 1)) { // eastern
        nx += 1;
        if (IsNextB(seg, ny, nx)) CheckNeighbours(seg, ny, nx, 2);
    }
    if (avoidDr != 3 && y < (h - 1) && x < (w - 1)) { // south-eastern
        ny += 1;
        nx += 1;
        if (IsNextB(seg, ny, nx)) CheckNeighbours(seg, ny, nx, 3);
    }
    if (avoidDr != 4 && y < (h - 1)) { // southern
        ny += 1;
        if (IsNextB(seg, ny, nx)) CheckNeighbours(seg, ny, nx, 4);
    }
    if (avoidDr != 5 && y < (h - 1) && x > 0) { // south-western
        ny += 1;
        nx -= 1;
        if (IsNextB(seg, ny, nx)) CheckNeighbours(seg, ny, nx, 5);
    }
    if (avoidDr != 6 && x > 0) { // western
        nx -= 1;
        if (IsNextB(seg, ny, nx)) CheckNeighbours(seg, ny, nx, 6);
    }
    if (avoidDr != 7 && y > 0 && x > 0) { // north-western
        ny -= 1;
        nx -= 1;
        if (IsNextB(seg, ny, nx)) CheckNeighbours(seg, ny, nx, 7);
    }
}

bool Segmentation::IsNextB(Segment *org_s, uint16_t y, uint16_t x) {
    uint32_t s_ = (status[y][x] << 2) >> 2;
    if (s_ == org_s->id) return false;
    if (((status[y][x] >> 30) & 1) == 0) {
        CheckIfBorder(s_index[s_], y, x);
        return (status[y][x] >> 31) == 1;
    }
    return false;
}

void Segmentation::Reset() {
    // zeroing `arr` is not necessary.
    memset(status, 0, sizeof(status));
    segments.clear();
}

Segmentation::~Segmentation() {
}
