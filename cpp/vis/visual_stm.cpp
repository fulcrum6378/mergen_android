#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sys/stat.h>

#include "../global.h"
#include "visual_stm.h"

using namespace std;

VisualSTM::VisualSTM() {
    // create directories if they don't exist and resolves their path variables
    struct stat sb{};
    string root;
    for (string *dir: {&root, &dirShapes, &dirY, &dirU, &dirV, &dirRt}) {
        string branch = *dir;
        dir->insert(0, visDirPath);
        if (!branch.empty()) dir->append("/");
        auto path = (*dir).c_str();
        if (stat(path, &sb) != 0) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    // load volatile indices
    // TODO

    // count frames that are stored in memory
    for ([[maybe_unused]] auto &_:
            filesystem::directory_iterator(filesystem::path{dirFrame}))
        framesStored++;

    // load saved state { nextFrameId, nextShapeId, earliestFrameId }
    string savedStatePath = visDirPath + savedStateFile;
    if (filesystem::exists(savedStatePath)) {
        ifstream ssf(savedStatePath, ios::binary);
        char buf[18];
        ssf.read(buf, sizeof(buf));
        nextFrameId = littleEndian
                      ? (((uint64_t) buf[7] << 56) | ((uint64_t) buf[6] << 48) |
                         ((uint64_t) buf[5] << 40) | ((uint64_t) buf[4] << 32) |
                         (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0])
                      : (((uint64_t) buf[0] << 56) | ((uint64_t) buf[1] << 48) |
                         ((uint64_t) buf[2] << 40) | ((uint64_t) buf[3] << 32) |
                         (buf[4] << 24) | (buf[5] << 16) | (buf[1] << 6) | buf[7]);
        nextShapeId = littleEndian
                      ? ((buf[9] << 8) | buf[8])
                      : ((buf[8] << 8) | buf[9]);
        firstShapeId = nextShapeId;
        firstFrameId = littleEndian
                          ? (((uint64_t) buf[17] << 56) | ((uint64_t) buf[16] << 48) |
                             ((uint64_t) buf[15] << 40) | ((uint64_t) buf[14] << 32) |
                             (buf[13] << 24) | (buf[12] << 16) | (buf[11] << 8) | buf[10])
                          : (((uint64_t) buf[10] << 56) | ((uint64_t) buf[11] << 48) |
                             ((uint64_t) buf[12] << 40) | ((uint64_t) buf[13] << 32) |
                             (buf[14] << 24) | (buf[15] << 16) | (buf[16] << 8) | buf[17]);
        ssf.close();
    }
}

void VisualSTM::Insert(
        uint8_t **m, // average colour
        uint16_t *w, uint16_t *h,  // width and height
        uint16_t cx, uint16_t cy, // central points
        unordered_set<SHAPE_POINT_T> *path
) {
    auto r = (uint16_t) round(((float) *w / (float) *h) * 10.0);

    // write shape file
    ofstream shf(dirShapes + to_string(nextShapeId), ios::binary);
    shf.write((char *) *m, 3); // Mean Colour
    shf.write((char *) &r, 2); // Ratio
    shf.write((char *) &nextFrameId, 8); // Frame ID
    shf.write((char *) w, 2); // Width
    shf.write((char *) h, 2); // Height
    shf.write((char *) &cx, 2); // Centre (X)
    shf.write((char *) &cy, 2); // Centre (Y)
    for (SHAPE_POINT_T p: *path)
        shf.write((char *) &p, shape_point_bytes); // Point {X, Y}
    shf.close();

    // update the volatile indices
    yi[(*m)[0]].insert(nextShapeId);
    ui[(*m)[1]].insert(nextShapeId);
    vi[(*m)[2]].insert(nextShapeId);
    ri[r].insert(nextShapeId);

    // increment shape ID
    nextShapeId++;
    if (nextShapeId > 65535) nextShapeId = 0;
}

void VisualSTM::OnFrameFinished() {
    // index this frame
    fi[nextFrameId] = pair(firstShapeId, nextShapeId);
    firstShapeId = nextShapeId;

    // check if some frames need to be forgotten
    framesStored++;
    if (framesStored > MAX_FRAMES_STORED) Forget();

    nextFrameId++;
    // if (nextFrameId > 18446744073709552000) nextFrameId = 0;
}

void VisualSTM::Forget() {
    auto t = chrono::system_clock::now();
    pair<uint16_t, uint16_t> *range;
    for (uint64_t f = firstFrameId; f < firstFrameId + FORGET_N_FRAMES; f++) {
        range = &fi[f];
        for (uint16_t sid = range->first; sid < range->second; sid++) {
            uint8_t y, u, v;
            uint16_t r;

            // open shape file, read its necessary details and then remove it
            string sPath = dirShapes + to_string(sid);
            ifstream shf(sPath, ios::binary);
            shf.read(reinterpret_cast<char *>(&y), 1);
            shf.read(reinterpret_cast<char *>(&u), 1);
            shf.read(reinterpret_cast<char *>(&v), 1);
            char buf[2];
            shf.read(buf, 2);
            r = littleEndian
                ? ((buf[1] << 8) | buf[0])
                : ((buf[0] << 8) | buf[1]);
            shf.close();
            remove(sPath.c_str());

            // remove this shape ID from all indexes
            yi[y].erase(sid);
            ui[u].erase(sid);
            vi[v].erase(sid);
            ri[r].erase(sid);
        }
        fi.erase(f);
    }

    firstFrameId += FORGET_N_FRAMES;
    framesStored -= FORGET_N_FRAMES;
    LOGI("Forgetting time: %lld", chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t).count());
}

void VisualSTM::SaveState() {
    // `saved_state`
    ofstream ssf(visDirPath + savedStateFile, ios::binary);
    ssf.write((char *) &nextFrameId, 8);
    ssf.write((char *) &nextShapeId, 2);
    ssf.write((char *) &firstFrameId, 8);
    ssf.close();
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter" // true negative!

template<class INT>
void VisualSTM::SaveIndexes(unordered_map<INT, list<uint16_t>> *indexes, string *dir) {
    for (pair<const INT, list<uint16_t>> &index: (*indexes)) {
        string path = (*dir) + to_string(index.first);
        if (!index.second.empty()) {
            ofstream sff(path, ios::binary);
            for (uint16_t sid: index.second)
                sff.write((char *) &sid, 2);
            sff.close();
        } else remove(path.c_str());
    }
    indexes->clear();
}

#pragma clang diagnostic pop
