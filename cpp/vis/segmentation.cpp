#include <algorithm> // std::sort
#include <chrono>
#include <cstring>
#include <fstream>
#include <ios>

#include "../global.hpp"
#include "segmentation.hpp"

using namespace std;

Segmentation::Segmentation(VisualSTM *stm, JavaVM *jvm, jobject main, jmethodID *jmSignal) :
        stm(stm), jvm_(jvm), main_(main), jmSignal_(jmSignal) {}

void Segmentation::Process(AImage *image, const bool *recording, int8_t debugMode) {
    locked = true;
    //ofstream test("/data/data/ir.mahdiparastesh.mergen/cache/test.yuv", ios::binary);

    // 1. loading; bring separate YUV data into the multidimensional array of pixels `arr`
    auto t0 = chrono::system_clock::now();
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

    for (uint16_t y = 0; y < H; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;
        int32_t uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (uint16_t x = 0; x < W; x++) {
            const int32_t uv_offset = (x >> 1) * uvPixelStride;
            arr[y][x][0] = pY[x];
            arr[y][x][1] = pU[uv_offset];
            arr[y][x][2] = pV[uv_offset];
            //test.put(pY[x]); test.put(pU[uv_offset]); test.put(pV[uv_offset]);
        }
    }
    AImage_delete(image); // test.close();
    auto delta1 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 2. segmentation
    t0 = chrono::system_clock::now();
    uint16_t thisY = 0, thisX = 0;
    int64_t last; // must be signed
    uint32_t nextSeg = 1;
    bool foundSthToAnalyse = true;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC" // false positive
    while (foundSthToAnalyse) {
#pragma clang diagnostic pop
        foundSthToAnalyse = false;
        for (uint16_t y = thisY; y < H; y++) {
            for (uint16_t x = (y == thisY) ? thisX : 0; x < W; x++)
                if (status[y][x] == 0) {
                    foundSthToAnalyse = true;
                    thisY = y;
                    thisX = x;
                    break;
                }
            if (foundSthToAnalyse) break;
        }
        if (!foundSthToAnalyse) break;

        Segment seg{nextSeg};
        stack.push_back({thisY, thisX, 0});
        nextSeg++;
        uint16_t y, x, dr;
        while ((last = ((int64_t) stack.size()) - 1) != -1) {
            y = stack[last][0], x = stack[last][1], dr = stack[last][2];
            if (dr == 0) {
                seg.p.push_back((y << 16) | x);
                status[y][x] = seg.id;
                // left
                stack[last][2]++;
                if (x > 0 && status[y][x - 1] == 0 && CompareColours(&arr[y][x], &arr[y][x - 1])) {
                    stack.push_back({y, static_cast<uint16_t>(x - 1), 0});
                    continue;
                }
            }
            if (dr <= 1) { // top
                stack[last][2]++;
                if (y > 0 && status[y - 1][x] == 0 && CompareColours(&arr[y][x], &arr[y - 1][x])) {
                    stack.push_back({static_cast<uint16_t>(y - 1), x, 0});
                    continue;
                }
            }
            if (dr <= 2) { // right
                stack[last][2]++;
                if (x < (W - 1) && status[y][x + 1] == 0 && CompareColours(&arr[y][x], &arr[y][x + 1])) {
                    stack.push_back({y, static_cast<uint16_t>(x + 1), 0});
                    continue;
                }
            }
            if (dr <= 3) { // bottom
                stack[last][2]++;
                if (y < (H - 1) && status[y + 1][x] == 0 && CompareColours(&arr[y][x], &arr[y + 1][x])) {
                    stack.push_back({static_cast<uint16_t>(y + 1), x, 0});
                    continue;
                }
            }
            stack.pop_back();
        }
        segments.push_back(seg);
    }
    auto delta2 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 3. dissolution
    t0 = chrono::system_clock::now();
#if MIN_SEG_SIZE > 1
    uint32_t absorber_i, size_bef = segments.size(), removal = 1;
    Segment *absorber;
    for (int32_t seg = ((int32_t) size_bef) - 1; seg > -1; seg--)
        if (segments[seg].p.size() < MIN_SEG_SIZE) {
            absorber_i = FindPixelOfASegmentToDissolveIn(&segments[seg]);
            if (absorber_i == 0xFFFFFFFF) continue;
            absorber = &segments[status[absorber_i >> 16][absorber_i & 0xFFFF] - 1];
            for (uint32_t &p: segments[seg].p) {
                absorber->p.push_back(p); // merge()
                status[p >> 16][p & 0xFFFF] = absorber->id;
            }
            swap(segments[seg], segments[size_bef - removal]);
            removal++;
        }
    segments.resize(size_bef - (removal - 1));
    LOGI("Total segments: %zu / %u", segments.size(), size_bef);
#else
    LOGI("Total segments: %zu", segments.size());
#endif
    auto delta3 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 4. average colours + detect boundaries
    t0 = chrono::system_clock::now();
    uint32_t l_;
    uint8_t *col;
    uint64_t aa, bb, cc;
    bool isFirst;
    uint16_t y, x;
    for (Segment &seg: segments) {
        // average colours of each segment
        aa = 0, bb = 0, cc = 0;
        for (uint32_t p: seg.p) {
            col = arr[p >> 16][p & 0xFFFF];
            aa += col[0];
            bb += col[1];
            cc += col[2];
        }
        l_ = seg.p.size();
        seg.m = new uint8_t[3]{static_cast<uint8_t>(aa / l_),
                               static_cast<uint8_t>(bb / l_),
                               static_cast<uint8_t>(cc / l_)};

        // detect boundaries (min_y, min_x, max_y, max_x)
        isFirst = true;
        for (uint32_t &p: seg.p) {
            y = p >> 16;
            x = p & 0xFFFF;
            if (isFirst) {
                seg.min_y = y;
                seg.min_x = x;
                seg.max_y = y;
                seg.max_x = x;
                isFirst = false;
            } else {
                if (y < seg.min_y) seg.min_y = y;
                if (x < seg.min_x) seg.min_x = x;
                if (y > seg.max_y) seg.max_y = y;
                if (x > seg.max_x) seg.max_x = x;
            }
        }
        seg.w = (seg.max_x + 1) - seg.min_x;
        seg.h = (seg.max_y + 1) - seg.min_y;

        // index the Segments by their IDs
        s_index[seg.id] = &seg;
    }
    auto delta4 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 5. trace border pixels
    t0 = chrono::system_clock::now();
    for (y = 0; y < H; y++) {
        if (y == 0 || y == H - 1)
            for (x = 0; x < W; x++)
                SetAsBorder(y, x);
        else
            for (x = 0; x < W; x++) {
                if (x == 0 or x == W - 1) {
                    SetAsBorder(y, x);
                    continue;
                }
                if (b_status[y][x] == 1) continue;
                CheckIfBorder(y, x, y, x + 1); //     eastern
                CheckIfBorder(y, x, y + 1, x + 1); // south-eastern
                CheckIfBorder(y, x, y + 1, x); //     southern
                CheckIfBorder(y, x, y + 1, x - 1); // south-western
            }
    }
    auto delta5 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 6. store the segments
    t0 = chrono::system_clock::now();
    sort(segments.begin(), segments.end(),
         [](const Segment &a, const Segment &b) { return a.p.size() > b.p.size(); });
    l_ = segments.size();
    for (uint16_t seg = 0; seg < MAX_SEGS; seg++) {// Segment &seg: segments
        if (seg >= l_) break;
        stm->Insert(&segments[seg].m, &segments[seg].w, &segments[seg].h,
                    (segments[seg].min_x + segments[seg].max_x + 1) / 2, // central point X
                    (segments[seg].min_y + segments[seg].max_y + 1) / 2, // central point Y
                    &segments[seg].border);
    }
    stm->OnFrameFinished();
    auto delta6 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // summary: loading + segmentation + dissolution + segment_analysis + tracing + saving
    LOGI("Delta times: %lld + %lld + %lld + %lld + %lld + %lld => %lld",
         delta1, delta2, delta3, delta4, delta5, delta6,
         delta1 + delta2 + delta3 + delta4 + delta5 + delta6);
    LOGI("----------------------------------");

    // if recording is over, save state of VisualSTM and...
    bool singleTime = debugMode > 10 && debugMode <= 20;
    if (!*recording || singleTime) {
        stm->SaveState();

        // inform the user that they can close the app safely
        JNIEnv *env;
        jvm_->GetEnv((void **) &env, JNI_VERSION_1_6);
        jvm_->AttachCurrentThread(&env, nullptr);
        env->CallVoidMethod(main_, *jmSignal_, 1);

        // save files for debugging if wanted
        if (debugMode == 11) {
            ofstream arrFile("/data/data/ir.mahdiparastesh.mergen/cache/arr", ios::binary);
            arrFile.write((char *) &arr, sizeof(arr));
            arrFile.close();
            ofstream bstFile("/data/data/ir.mahdiparastesh.mergen/cache/b_status", ios::binary);
            bstFile.write((char *) &b_status, sizeof(b_status));
            bstFile.close();
        }

        // signal Main.java to stop recording;
        // this MUST always be after saving the related files of debugding
        if (singleTime) env->CallVoidMethod(main_, *jmSignal_, 2);
    }

    // clear data and unlock the frame
    memset(status, 0, sizeof(status));
    s_index.clear();
    segments.clear();
    locked = false;
}

/**
 * - `abs()` is much more efficient than `256 - static_cast<uint8_t>(a - b)`!
 * - There's no need for `static_cast<int16_t>`.
 * - `https://stackoverflow.com/questions/649454/what-is-the-best-way-to-average-two-colors-
 * that-define-a-linear-gradient` doesn't make a (big) difference.
 * - Geometric mean didn't work correctly (0,0,0).
 */
bool Segmentation::CompareColours(uint8_t (*a)[3], uint8_t (*b)[3]) {
    return abs((*a)[0] - (*b)[0]) <= 4 &&
           abs((*a)[1] - (*b)[1]) <= 4 &&
           abs((*a)[2] - (*b)[2]) <= 4;
}

uint32_t Segmentation::FindPixelOfASegmentToDissolveIn(Segment *seg) {
    uint32_t cor = seg->p.front();
    uint16_t a = cor >> 16, b = cor & 0xFFFF;
    if (a > 0)
        return ((a - 1) << 16) | b;
    if (b > 0)
        return (a << 16) | (b - 1);
    cor = seg->p.back();
    a = cor >> 16, b = cor & 0xFFFF;
    if (a < H - 1)
        return ((a + 1) << 16) | b;
    if (b < W - 1)
        return (a << 16) | (b + 1);
    return 0xFFFFFFFF;
}

void Segmentation::CheckIfBorder(uint16_t y1, uint16_t x1, uint16_t y2, uint16_t x2) {
    if (status[y1][x1] != status[y2][x2]) {
        SetAsBorder(y1, x1);
        SetAsBorder(y2, x2);
    }
}

void Segmentation::SetAsBorder(uint16_t y, uint16_t x) {
    b_status[y][x] |= 1;
    Segment *seg = s_index[status[y][x]];
    seg->border.insert(
            (static_cast<SHAPE_POINT_T>((shape_point_max / (float) seg->w) *
                                        (float) (x - seg->min_x)) // fractional X
                    << shape_point_each_bits) |
            static_cast<SHAPE_POINT_T>((shape_point_max / (float) seg->h) *
                                       (float) (y - seg->min_y))  // fractional Y
    );
}

Segmentation::~Segmentation() = default;
