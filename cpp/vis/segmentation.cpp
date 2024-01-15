#include <algorithm> // std::sort
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <ios>

#include "../global.hpp"
#include "segmentation.hpp"

using namespace std;

Segmentation::Segmentation(JavaVM *jvm, jobject main, jmethodID *jmSignal, ANativeWindow **analyses) :
        jvm_(jvm), main_(main), jmSignal_(jmSignal), analyses_(analyses) {
#if VISUAL_STM
    stm = new VisualSTM;
#endif
}

void Segmentation::Process(AImage *image, const bool *recording, int8_t debugMode) {
    locked = true;
    //ofstream test(cacheDir + "test.yuv", ios::binary);

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

    for (uint16_t y = 0u; y < H; y++) {
        const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;
        int32_t uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);

        for (uint16_t x = 0u; x < W; x++) {
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
    uint32_t nextSeg = 1u;
    uint16_t thisY = 0u, thisX = 0u;
    int64_t last; // must be signed
    bool foundSthToAnalyse = true;
    while (foundSthToAnalyse) {
        foundSthToAnalyse = false;
        for (uint16_t y = thisY; y < H; y++) {
            for (uint16_t x = (y == thisY) ? thisX : 0u; x < W; x++)
                if (status[y][x] == 0u) {
                    foundSthToAnalyse = true;
                    thisY = y;
                    thisX = x;
                    break;
                }
            if (foundSthToAnalyse) break;
        }
        if (!foundSthToAnalyse) break;

        Segment seg{nextSeg};
        stack.push_back({thisY, thisX, 0u});
        nextSeg++;
        uint16_t y, x, dr;
        while ((last = static_cast<int64_t>(stack.size()) - 1) != -1) {
            y = stack[last][0], x = stack[last][1], dr = stack[last][2];
            if (dr == 0) {
                seg.p.push_back((y << 16) | x);
#if MIN_SEG_SIZE == 1u // add colours in order to compute their mean value later
                seg.ys += arr[y][x][0] * arr[y][x][0];
                seg.us += arr[y][x][1] * arr[y][x][1];
                seg.vs += arr[y][x][2] * arr[y][x][2];
#endif
                status[y][x] = seg.id;
                // left
                stack[last][2]++;
                if (x > 0u && status[y][x - 1u] == 0 && CompareColours(&arr[y][x], &arr[y][x - 1u])) {
                    stack.push_back({y, static_cast<uint16_t>(x - 1u), 0u});
                    continue;
                }
            }
            if (dr <= 1u) { // top
                stack[last][2]++;
                if (y > 0u && status[y - 1u][x] == 0 && CompareColours(&arr[y][x], &arr[y - 1u][x])) {
                    stack.push_back({static_cast<uint16_t>(y - 1u), x, 0u});
                    continue;
                }
            }
            if (dr <= 2u) { // right
                stack[last][2]++;
                if (x < (W - 1u) && status[y][x + 1u] == 0 && CompareColours(&arr[y][x], &arr[y][x + 1u])) {
                    stack.push_back({y, static_cast<uint16_t>(x + 1u), 0u});
                    continue;
                }
            }
            if (dr <= 3u) { // bottom
                stack[last][2]++;
                if (y < (H - 1u) && status[y + 1u][x] == 0 && CompareColours(&arr[y][x], &arr[y + 1u][x])) {
                    stack.push_back({static_cast<uint16_t>(y + 1u), x, 0u});
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
#if MIN_SEG_SIZE != 1u
    uint32_t absorber_i, size_bef = segments.size(), removal = 1u;
    Segment *absorber;
    for (int32_t seg = static_cast<int32_t>(size_bef) - 1; seg > -1; seg--)
        if (segments[seg].p.size() < MIN_SEG_SIZE) {
            absorber_i = FindPixelOfASegmentToDissolveIn(&segments[seg]);
            if (absorber_i == 0xFFFFFFFF) continue;
            absorber = &segments[status[absorber_i >> 16][absorber_i & 0xFFFF] - 1u];
            for (uint32_t &p: segments[seg].p) {
                absorber->p.push_back(p); // merge()
                status[p >> 16][p & 0xFFFF] = absorber->id;
            }
            swap(segments[seg], segments[size_bef - removal]);
            removal++;
        }
    segments.resize(size_bef - (removal - 1u));
    LOGI("Total segments: %zu / %u", segments.size(), size_bef);
#else
    LOGI("Total segments: %zu", segments.size());
#endif
    auto delta3 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 4. average colours + detect boundaries
    t0 = chrono::system_clock::now();
    uint32_t l_;
#if MIN_SEG_SIZE != 1u
    array<uint8_t, 3u> *col;
    uint64_t ys, us, vs;
#endif
    bool isFirst;
    uint16_t y, x;
    for (Segment &seg: segments) {
        // average colours of each segment
        l_ = seg.p.size();
#if MIN_SEG_SIZE != 1u
        ys = 0ull, us = 0ull, vs = 0ull;
        for (uint32_t p: seg.p) {
            col = reinterpret_cast<array<uint8_t, 3u> *>(&arr[p >> 16][p & 0xFFFF]);
            ys += (*col)[0] * (*col)[0]; // pow(, 2)
            us += (*col)[1] * (*col)[1];
            vs += (*col)[2] * (*col)[2];
        }
        seg.m = {static_cast<uint8_t>(sqrt(ys / l_)),
                 static_cast<uint8_t>(sqrt(us / l_)),
                 static_cast<uint8_t>(sqrt(vs / l_))};
#else
        seg.m = {static_cast<uint8_t>(sqrt(seg.ys / l_)),
                 static_cast<uint8_t>(sqrt(seg.us / l_)),
                 static_cast<uint8_t>(sqrt(seg.vs / l_))};
#endif
        // https://stackoverflow.com/questions/649454/what-is-the-best-way-to-average-two-colors-that-
        // define-a-linear-gradient

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
        seg.w = (seg.max_x + 1u) - seg.min_x;
        seg.h = (seg.max_y + 1u) - seg.min_y;

        // index the Segments by their IDs
        s_index[seg.id] = &seg;
    }
    auto delta4 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 5. trace border pixels
    t0 = chrono::system_clock::now();
    for (y = 0u; y < H; y++) {
        if (y == 0u || y == H - 1u)
            for (x = 0u; x < W; x++)
                SetAsBorder(y, x);
        else
            for (x = 0u; x < W; x++) {
                if (x == 0u or x == W - 1u) {
                    SetAsBorder(y, x);
                    continue;
                }
                if (b_status[y][x] == 1) continue;
                CheckIfBorder(y, x, y, x + 1u); //     eastern
                CheckIfBorder(y, x, y + 1u, x + 1u); // south-eastern
                CheckIfBorder(y, x, y + 1u, x); //     southern
                CheckIfBorder(y, x, y + 1u, x - 1u); // south-western
            }
    }
#if ANALYSES
    ANativeWindow_acquire(*analyses_);
    if (ANativeWindow_lock(*analyses_, &analysesBuf, nullptr) == 0) {
        auto *out = static_cast<uint32_t *>(analysesBuf.bits);
        out += analysesBuf.width - 1;
        for (int32_t yy = 0; yy < analysesBuf.height; yy++) {
            for (int32_t xx = 0; xx < analysesBuf.width; xx++) {
                out[xx * analysesBuf.stride] = (b_status[yy][xx] == 1) ? 0xFF0000FF : 0x00000000; // BGR
            }
            out -= 1; // move to the next column
        }
        ANativeWindow_unlockAndPost(*analyses_);
    }
    ANativeWindow_release(*analyses_);
#endif
    auto delta5 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 6. (save in VisualSTM and) track objects and measure their differences
    t0 = chrono::system_clock::now();
    sort(segments.begin(), segments.end(),
         [](const Segment &a, const Segment &b) { return a.p.size() > b.p.size(); });
    float nearest_dist, dist;
    int32_t best;
    l_ = segments.size();
    for (uint16_t sid = 0u; sid < MAX_SEGS; sid++) {// Segment &seg: segments
        if (sid >= l_) break;
        Segment *seg = &segments[sid];
        seg->ComputeRatioAndCentre();
#if VISUAL_STM
        stm->Insert(seg);
#endif
        if (!prev_segments.empty()) {
            for (uint8_t y_ = seg->m[0] - Y_RADIUS; y_ < seg->m[0] + Y_RADIUS; y_++) {
                auto it = yi.find(y_);
                if (it == yi.end()) continue;
                for (uint16_t i: (*it).second) a_y.insert(i);
            }
            for (uint8_t u_ = seg->m[1] - U_RADIUS; u_ < seg->m[1] + U_RADIUS; u_++) {
                auto it = ui.find(u_);
                if (it == ui.end()) continue;
                for (uint16_t i: (*it).second) a_u.insert(i);
            }
            for (uint8_t v_ = seg->m[2] - V_RADIUS; v_ < seg->m[2] + V_RADIUS; v_++) {
                auto it = vi.find(v_);
                if (it == vi.end()) continue;
                for (uint16_t i: (*it).second) a_v.insert(i);
            }
            for (uint16_t r_ = seg->r - R_RADIUS; r_ < seg->r + R_RADIUS; r_++) {
                auto it = ri.find(r_);
                if (it == ri.end()) continue;
                for (uint16_t i: (*it).second) a_r.insert(i);
            }
            best = -1;
            for (uint16_t can: a_y)
                if (a_u.find(can) != a_u.end() && a_v.find(can) != a_v.end()
                    && a_r.find(can) != a_r.end()) {
                    Segment *prev_seg = &prev_segments[can];
                    dist = static_cast<float>(sqrt(std::pow(seg->cx - prev_seg->cx, 2) +
                                                   std::pow(seg->cy - prev_seg->cy, 2)));
                    if (best == -1) { // NOLINT(bugprone-branch-clone)
                        nearest_dist = dist;
                        best = static_cast<int32_t>(can);
                    } else if (dist < nearest_dist) {
                        nearest_dist = dist;
                        best = static_cast<int32_t>(can);
                    } // else {don't set `best` here}
                }
            a_y.clear();
            a_u.clear();
            a_v.clear();
            a_r.clear();
            if (best != -1) {
                Segment *prev_seg = &prev_segments[best];
                diff[sid] = {
                        best, static_cast<int32_t>(nearest_dist),
                        prev_seg->w - seg->w, prev_seg->h - seg->h, prev_seg->r - seg->r,
                        prev_seg->m[0] - seg->m[0], prev_seg->m[1] - seg->m[1],
                        prev_seg->m[2] - seg->m[2],
                };
                /*LOGI("%u->%d : %d, %d, %d, %d, %d, %d, %d", sid, diff[sid][0], diff[sid][1],
                     diff[sid][2], diff[sid][3], diff[sid][4],
                     diff[sid][5], diff[sid][6], diff[sid][7]);*/
            }/* else
                LOGI("Segment %u was lost", sid);*/
        }
        // index segments of the current frame
        _yi[seg->m[0]].insert(sid);
        _ui[seg->m[1]].insert(sid);
        _vi[seg->m[2]].insert(sid);
        _ri[seg->r].insert(sid);
    }
    // replace indexes of the previous frame with the current one
    yi = std::move(_yi);
    ui = std::move(_ui);
    vi = std::move(_vi);
    ri = std::move(_ri);
#if VISUAL_STM
    stm->OnFrameFinished();
#endif
    auto delta6 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // summary: loading + segmentation + dissolution + segment_analysis + tracing + stm&tracking
    LOGI("Delta times: %lld + %lld + %lld + %lld + %lld + %lld => %lld",
         delta1, delta2, delta3, delta4, delta5, delta6,
         delta1 + delta2 + delta3 + delta4 + delta5 + delta6);
    LOGI("----------------------------------");

    // debugging
    bool singleTime = debugMode > 10 && debugMode <= 20;
    if (!*recording || singleTime) {
#if VISUAL_STM
        stm->SaveState();
#endif

        // inform the user that they can close the app safely
        JNIEnv *env;
        jvm_->GetEnv((void **) &env, JNI_VERSION_1_6);
        jvm_->AttachCurrentThread(&env, nullptr);
        env->CallVoidMethod(main_, *jmSignal_, 1);

        // save files for debugging if wanted
        if (debugMode == 11) {
            ofstream arrFile(cacheDir + "arr", ios::binary);
            arrFile.write(reinterpret_cast<char *>(&arr), sizeof(arr));
            arrFile.close();
            ofstream bstFile(cacheDir + "b_status", ios::binary);
            bstFile.write(reinterpret_cast<char *>(&b_status), sizeof(b_status));
            bstFile.close();
        }

        // signal Main.java to stop recording;
        // this MUST always be after saving the related files of debugding
        if (singleTime) env->CallVoidMethod(main_, *jmSignal_, 2);
    }

    // clear data and unlock the frame
    memset(status, 0, sizeof(status));
    memset(b_status, 0, sizeof(b_status));
    s_index.clear();
    prev_segments = std::move(segments);
    diff.clear();
    locked = false;
}

bool Segmentation::CompareColours(uint8_t (*a)[3], uint8_t (*b)[3]) {
    return abs((*a)[0] - (*b)[0]) <= 4 &&
           abs((*a)[1] - (*b)[1]) <= 4 &&
           abs((*a)[2] - (*b)[2]) <= 4;
}

uint32_t Segmentation::FindPixelOfASegmentToDissolveIn(Segment *seg) {
    uint32_t cor = seg->p.front();
    uint16_t a = cor >> 16, b = cor & 0xFFFF;
    if (a > 0u)
        return ((a - 1u) << 16) | b;
    if (b > 0u)
        return (a << 16) | (b - 1u);
    cor = seg->p.back();
    a = cor >> 16, b = cor & 0xFFFF;
    if (a < H - 1u)
        return ((a + 1u) << 16) | b;
    if (b < W - 1u)
        return (a << 16) | (b + 1u);
    return 0xFFFFFFFF;
}

void Segmentation::CheckIfBorder(uint16_t y1, uint16_t x1, uint16_t y2, uint16_t x2) {
    if (status[y1][x1] != status[y2][x2]) {
        SetAsBorder(y1, x1);
        SetAsBorder(y2, x2);
    }
}

void Segmentation::SetAsBorder(uint16_t y, uint16_t x) {
    b_status[y][x] |= 1u;
    Segment *seg = s_index[status[y][x]];
    seg->border.insert(
            (static_cast<SHAPE_POINT_T>((SHAPE_POINT_MAX / static_cast<float>(seg->h)) *
                                        static_cast<float>(y - seg->min_y)) // fractional Y
                    << SHAPE_POINT_EACH_BITS) |
            static_cast<SHAPE_POINT_T>((SHAPE_POINT_MAX / static_cast<float>(seg->w)) *
                                       static_cast<float>(x - seg->min_x))  // fractional X
    ); // they get reversed in while writing to a file
}

Segmentation::~Segmentation() {
#if VISUAL_STM
    delete stm;
#endif
}
