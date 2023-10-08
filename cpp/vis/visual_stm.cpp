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
    struct stat sb;
    string root("");
    for (string *dir: {&root, &dirShapes, &dirFrame, &dirY, &dirU, &dirV, &dirRt}) {
        string branch = (*dir);
        if (branch != "") {
            dir->insert(0, visDirPath);
            dir->append("/");
        }
        auto path = (visDirPath + branch).c_str();
        if (stat(path, &sb) != 0) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    // count frames that are stored in memory
    for ([[maybe_unused]] auto &_:
            filesystem::directory_iterator(filesystem::path{dirFrame}))
        framesStored++;

    // load saved state { nextFrameId, nextShapeId, earliestFrameId }
    const char *savedStatePath = (visDirPath + savedStateFile).c_str();
    if (filesystem::exists(savedStatePath)) {
        ifstream ssf(savedStatePath, ios::binary);
        char buf[18];
        ssf.read(buf, sizeof(buf));
        nextFrameId = ((uint64_t) buf[7] << 56) | ((uint64_t) buf[6] << 48) |
                      ((uint64_t) buf[5] << 40) | ((uint64_t) buf[4] << 32) |
                      (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
        nextShapeId = (buf[9] << 8) | buf[8];
        earliestFrameId = ((uint64_t) buf[17] << 56) | ((uint64_t) buf[16] << 48) |
                          ((uint64_t) buf[15] << 40) | ((uint64_t) buf[14] << 32) |
                          (buf[13] << 24) | (buf[12] << 16) | (buf[11] << 8) | buf[10];
        ssf.close();
    }
}

void VisualSTM::Insert(
        uint8_t *m,
        uint16_t w, uint16_t h,
        unordered_set<shape_point_t> path
) {
    uint16_t r = round(((float) w / (float) h) * 10);

    // write shape file
    ofstream shf(dirShapes + to_string(nextShapeId), ios::binary);
    shf.put(m[0]); // Y
    shf.put(m[1]); // U
    shf.put(m[2]); // V
    shf.write((char *) &r, 2); // Ratio
    shf.write((char *) &nextFrameId, 8); // Frame ID
    shf.write((char *) &w, 2); // Width
    shf.write((char *) &h, 2); // Height
    for (shape_point_t p: path)
        shf.write((char *) &p, shape_point_bytes); // Point {X, Y}
    shf.close();

    // update Y indexes
    ofstream y_f((dirY + to_string(m[0])).c_str(), ios::app | ios::binary);
    y_f.write((char *) &nextShapeId, 2);
    y_f.close();

    // update U indexes
    ofstream u_f((dirU + to_string(m[1])).c_str(), ios::app | ios::binary);
    u_f.write((char *) &nextShapeId, 2);
    u_f.close();

    // update V indexes
    ofstream v_f((dirV + to_string(m[2])).c_str(), ios::app | ios::binary);
    v_f.write((char *) &nextShapeId, 2);
    v_f.close();

    // update Ratio indexes
    ofstream rtf((dirRt + to_string(r)).c_str(), ios::app | ios::binary);
    rtf.write((char *) &nextShapeId, 2);
    rtf.close();

    // save and increment shape ID
    shapesInFrame.push_back(nextShapeId);
    nextShapeId++;
    if (nextShapeId > 65535) nextShapeId = 0;
}

void VisualSTM::OnFrameFinished() {
    // save Frame Index
    ofstream f_f((dirFrame + to_string(nextFrameId)).c_str(), ios::binary);
    for (uint16_t sid: shapesInFrame)
        f_f.write((char *) &sid, 2);
    f_f.close();
    shapesInFrame.clear();

    // check if some frames need to be forgotten
    framesStored++;
    if (framesStored > max_frames_stored) Forget();

    nextFrameId++;
    // if (nextFrameId > 18446744073709552000) nextFrameId = 0;
}

void VisualSTM::Forget() {
    auto t = chrono::system_clock::now();
    for (uint64_t f = earliestFrameId; f < earliestFrameId + forget_n_frames; f++) {
        IterateIndex((dirFrame + to_string(f)).c_str(), [](VisualSTM *stm, uint16_t sid) -> void {
            uint8_t y, u, v;
            uint16_t r;

            // open shape file, read its necessary details and then remove it
            const char *sPath = (stm->dirShapes + to_string(sid)).c_str();
            ifstream shf(sPath, ios::binary);
            shf.read(reinterpret_cast<char *>(&y), 1);
            shf.read(reinterpret_cast<char *>(&u), 1);
            shf.read(reinterpret_cast<char *>(&v), 1);
            char buf[2];
            shf.read(buf, 2);
            r = (buf[1] << 8) | buf[0];
            shf.close();
            remove(sPath);

            // read unread indices
            if (!stm->ym.contains(y))
                stm->ym[y] = stm->ReadIndex((stm->dirY + to_string(y)).c_str());
            if (!stm->um.contains(u))
                stm->um[u] = stm->ReadIndex((stm->dirU + to_string(u)).c_str());
            if (!stm->vm.contains(v))
                stm->vm[v] = stm->ReadIndex((stm->dirV + to_string(v)).c_str());
            if (!stm->rm.contains(r))
                stm->rm[r] = stm->ReadIndex((stm->dirRt + to_string(r)).c_str());

            // remove this shape ID from all indexes
            stm->RemoveFromIndex(&stm->ym[y], sid);
            stm->RemoveFromIndex(&stm->um[u], sid);
            stm->RemoveFromIndex(&stm->vm[v], sid);
            stm->RemoveFromIndex(&stm->rm[r], sid);
        });
        remove((dirFrame + to_string(f)).c_str()); // don't put it in a variable
    }
    SaveIndexes<uint8_t>(&ym, &dirY);
    SaveIndexes<uint8_t>(&um, &dirU);
    SaveIndexes<uint8_t>(&vm, &dirV);
    SaveIndexes<uint16_t>(&rm, &dirRt);

    earliestFrameId += forget_n_frames;
    framesStored -= forget_n_frames;
    LOGI("Forgetting time: %lld", chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t).count());
}

void VisualSTM::IterateIndex(const char *path, void onEach(VisualSTM *, uint16_t)) {
    char buf[2];
    struct stat sb; // never make it a class member!
    stat(path, &sb);
    ifstream sff(path, ios::binary);
    for (off_t _ = 0; _ < sb.st_size; _ += 2) {
        sff.read(buf, 2);
        onEach(this, (buf[1] << 8) | buf[0]);
    }
    sff.close();
}

list<uint16_t> VisualSTM::ReadIndex(const char *path) {
    char buf[2];
    struct stat sb;
    stat(path, &sb);
    ifstream sff(path, ios::binary);
    list<uint16_t> l;
    for (off_t _ = 0; _ < sb.st_size; _ += 2) {
        sff.read(buf, 2);
        l.push_back((buf[1] << 8) | buf[0]);
    }
    sff.close();
    return l;
}

void VisualSTM::RemoveFromIndex(list<uint16_t> *l, uint16_t id) {
    for (list<uint16_t>::iterator sid = begin(*l); sid != end(*l); ++sid)
        if (*sid == id) {
            l->erase(sid);
            break;
        }
}

template<class INT>
void VisualSTM::SaveIndexes(unordered_map<INT, list<uint16_t>> *indexes, string *dir) {
    for (pair<const INT, list<uint16_t>> &index: (*indexes)) {
        const char *path = ((*dir) + to_string(index.first)).c_str();
        if (!index.second.empty()) {
            ofstream sff(path, ios::trunc | ios::binary);
            for (uint16_t sid: index.second)
                sff.write((char *) &sid, 2);
            sff.close();
        } else remove(path);
    }
    indexes->clear();
}

void VisualSTM::SaveState() {
    ofstream ssf((visDirPath + savedStateFile).c_str(), ios::binary);
    ssf.write((char *) &nextFrameId, 8);
    ssf.write((char *) &nextShapeId, 2);
    ssf.write((char *) &earliestFrameId, 8);
    ssf.close();
}
