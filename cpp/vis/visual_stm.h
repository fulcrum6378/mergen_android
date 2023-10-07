#ifndef VIS_VISUAL_STM_H
#define VIS_VISUAL_STM_H

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sys/stat.h>

#include "../global.h"

using namespace std;

/** Visual Short-Term Memory */
class VisualSTM {
private:
    const string visDirPath = "/data/data/ir.mahdiparastesh.mergen/files/vis/";
    // maximum frames allowed to be present in memory at a time
    const uint16_t max_frames_stored = 3;
    // forget N frames whenever hit the maximum
    const uint64_t forget_n_frames = 1;

    string dirShapes = "shapes", dirFrame = "f", dirY = "y", dirU = "u", dirV = "v", dirRt = "r",
            savedStateFile = "saved_state";
    uint64_t nextFrameId = 0;
    uint16_t nextShapeId = 0;
    // ID of earliest frame which is still available in memory
    uint64_t earliestFrameId = 0;
    // total number of frames available in memory
    uint16_t framesStored = 0;
    // IDs of shapes inside current frame
    list<uint16_t> shapesInFrame;
    // container for file statistics
    struct stat sb;
    // helper maps for altering 'uint8_t' indexes
    unordered_map<uint8_t, list<uint16_t>> ym, um, vm;
    // helper maps for altering 'uint16_t' indexes
    unordered_map<uint16_t, list<uint16_t>> rm;

public:
    VisualSTM() {
        // create directories if they don't exist and resolves their path variables
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
        for (auto &_: filesystem::directory_iterator(filesystem::path{dirFrame}))
            framesStored++;

        // load saved state: { nextFrameId, nextShapeId, earliestFrameId }
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

    void Insert(
            uint8_t *m,
            uint16_t w, uint16_t h,
            list<pair<float, float>> path
    ) {
        uint16_t ratio = round(((float) w / (float) h) * 10);

        // write shape file
        ofstream shf(dirShapes + to_string(nextShapeId), ios::binary);
        shf.put(m[0]);
        shf.put(m[1]);
        shf.put(m[2]);
        shf.write((char *) &ratio, 2);
        shf.write((char *) &nextFrameId, 8);
        shf.write((char *) &w, 2);
        shf.write((char *) &h, 2);
        for (pair p: path) {
            shf.write((char *) &p.first, 4);
            shf.write((char *) &p.second, 4);
        }
        shf.close();

        // update Y indexes
        ofstream y_f((dirY + to_string(m[0])).c_str(),
                     ios::app | ios::binary);
        y_f.write((char *) &nextShapeId, 2);
        y_f.close();

        // update U indexes
        ofstream u_f((dirU + to_string(m[1])).c_str(),
                     ios::app | ios::binary);
        u_f.write((char *) &nextShapeId, 2);
        u_f.close();

        // update V indexes
        ofstream v_f((dirV + to_string(m[2])).c_str(),
                     ios::app | ios::binary);
        v_f.write((char *) &nextShapeId, 2);
        v_f.close();

        // update Ratio indexes
        ofstream rtf((dirRt + to_string(ratio)).c_str(),
                     ios::app | ios::binary);
        rtf.write((char *) &nextShapeId, 2);
        rtf.close();

        // save and increment shape ID
        shapesInFrame.push_back(nextShapeId);
        nextShapeId++;
        if (nextShapeId > 65535) nextShapeId = 0;
    }

    /** Anything that needs to be done at the end. */
    void OnFrameFinished() {
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
        // if (nextFrameId > 18446744073709552000) nextShapeId = 0;
    }

    /** Forgets some of oldest frames. */
    void Forget() {
        LOGI("Forgetting started...");
        auto t = chrono::system_clock::now();
        const char *fPath;
        //LOGE("%u .. %u", earliestFrameId, earliestFrameId + forget_n_frames);
        for (uint64_t f = earliestFrameId; f < earliestFrameId + forget_n_frames; f++) {
            fPath = (dirFrame + to_string(f)).c_str();
            IterateIndex(fPath, [](VisualSTM *stm, uint16_t sid) -> void {
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
                if (!stm->ym.contains(y)) {
                    static list<uint16_t> l;
                    stm->IterateIndex(to_string(y).c_str(),
                                      [](VisualSTM *stm, uint16_t sid) -> void { l.push_back(sid); });
                    stm->ym[y] = l;
                }
                if (!stm->um.contains(u)) {
                    static list<uint16_t> l;
                    stm->IterateIndex(to_string(u).c_str(),
                                      [](VisualSTM *stm, uint16_t sid) -> void { l.push_back(sid); });
                    stm->um[u] = l;
                }
                if (!stm->vm.contains(v)) {
                    static list<uint16_t> l;
                    stm->IterateIndex(to_string(v).c_str(),
                                      [](VisualSTM *stm, uint16_t sid) -> void { l.push_back(sid); });
                    stm->vm[v] = l;
                }
                if (!stm->rm.contains(r)) {
                    static list<uint16_t> l;
                    stm->IterateIndex(to_string(r).c_str(),
                                      [](VisualSTM *stm, uint16_t sid) -> void { l.push_back(sid); });
                    stm->rm[r] = l;
                }

                // remove this shape ID from all indexes FIXME problem is here
                stm->ym[y].remove(sid);
                stm->um[u].remove(sid);
                stm->vm[v].remove(sid);
                stm->rm[r].remove(sid);
                // FIXME implement a method that when a value was found, it breaks the loop
            });
            remove(fPath);
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

    /** Reads a Sequence File. */
    void IterateIndex(const char *path, void onEach(VisualSTM *, uint16_t)) {
        char buf[2];
        stat(path, &sb);
        ifstream sff(path, ios::binary);
        for (uint32_t _ = 0; _ < sb.st_size; _ += 2) {
            sff.read(buf, 2);
            onEach(this, (buf[1] << 8) | buf[0]);
        }
        sff.close();
    }

    /** Save index in non-volatile memory and clear its data from RAM. */
    template<class INT>
    void SaveIndexes(unordered_map<INT, list<uint16_t>> *indexes, string *dir) {
        for (pair<const INT, list<uint16_t>> &index: (*indexes)) {
            const char *path = ((*dir) + to_string(index.first)).c_str();
            if (!index.second.empty()) {
                ofstream sff(path, ios::binary);
                for (uint16_t sid: index.second)
                    sff.write((char *) &sid, 2);
                /*for(auto sid = begin(index.second); sid != end(index.second); ++sid)
                    sff.write((char *) &sid, 2);*/
                sff.close();
            } else remove(path);
        }
        indexes->clear();
    }

    /** Saves current state: { nextFrameId, nextShapeId, earliestFrameId }
     * Don't save paths in variables in the constructor! */
    void SaveState() {
        ofstream ssf((visDirPath + savedStateFile).c_str(), ios::binary);
        ssf.write((char *) &nextFrameId, 8);
        ssf.write((char *) &nextShapeId, 2);
        ssf.write((char *) &earliestFrameId, 8);
        ssf.close();
    }
};

#endif //VIS_VISUAL_STM_H
