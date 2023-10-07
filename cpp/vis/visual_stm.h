#ifndef VIS_VISUAL_STM_H
#define VIS_VISUAL_STM_H

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>

#include "../global.h"

/** Visual Short-Term Memory */
class VisualSTM {
private:
    const std::string visDirPath = "/data/data/ir.mahdiparastesh.mergen/files/vis/";
    // maximum frames allowed to be present in memory at a time
    const uint16_t max_frames_stored = 5;
    // forget N frames whenever hit the maximum
    const uint64_t forget_n_frames = 1;

    std::string dirShapes = "shapes", dirFrame = "f", dirY = "y", dirU = "u", dirV = "v", dirRt = "r",
            savedStateFile = "saved_state";
    uint64_t nextFrameId;
    uint16_t nextShapeId;
    // ID of earliest frame which is still available in memory
    uint64_t earliestFrameId;
    // total number of frames available in memory
    uint16_t framesStored;
    // IDs of shapes inside current frame
    std::list<uint16_t> shapesInFrame;
    // container for file statistics
    struct stat sb;

public:
    VisualSTM() {
        // create directories if they don't exist and resolves their path variables
        std::string root("");
        for (std::string *dir: {&root, &dirShapes, &dirFrame, &dirY, &dirU, &dirV, &dirRt}) {
            std::string branch = (*dir);
            if (branch != "") {
                dir->insert(0, visDirPath);
                dir->append("/");
            }
            auto path = (visDirPath + branch).c_str();
            if (stat(path, &sb) != 0) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

        // count frames that are stored in memory
        for (auto &_: std::filesystem::directory_iterator(std::filesystem::path{dirFrame}))
            framesStored++;

        // load saved state: { nextFrameId, nextShapeId, earliestFrameId }
        const char *savedStatePath = (visDirPath + savedStateFile).c_str();
        if (std::filesystem::exists(savedStatePath)) {
            std::ifstream ssf(savedStatePath, std::ios::binary);
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
            std::list<std::pair<float, float>> path
    ) {
        // write shape file
        std::ofstream shf(dirShapes + std::to_string(nextShapeId), std::ios::binary);
        shf.write((char *) &nextFrameId, 8);
        shf.put(m[0]);
        shf.put(m[1]);
        shf.put(m[2]);
        shf.write((char *) &w, 2);
        shf.write((char *) &h, 2);
        for (std::pair p: path) {
            shf.write((char *) &p.first, 4);
            shf.write((char *) &p.second, 4);
        }
        shf.close();

        // update Y indexes
        std::ofstream y_f((dirY + std::to_string(m[0])).c_str(),
                          std::ios::app | std::ios::binary);
        y_f.write((char *) &nextShapeId, 2);
        y_f.close();

        // update U indexes
        std::ofstream u_f((dirU + std::to_string(m[1])).c_str(),
                          std::ios::app | std::ios::binary);
        u_f.write((char *) &nextShapeId, 2);
        u_f.close();

        // update V indexes
        std::ofstream v_f((dirV + std::to_string(m[2])).c_str(),
                          std::ios::app | std::ios::binary);
        v_f.write((char *) &nextShapeId, 2);
        v_f.close();

        // update Ratio indexes
        int16_t ratio = round(((float) w / (float) h) * 10);
        std::ofstream rtf((dirRt + std::to_string(ratio)).c_str(),
                          std::ios::app | std::ios::binary);
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
        std::ofstream f_f((dirFrame + std::to_string(nextFrameId)).c_str(), std::ios::binary);
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
        auto t = std::chrono::system_clock::now();
        char buf[2];
        uint16_t sid;
        uint8_t y, u, v;
        const char *fPath, *sPath;
        for (uint64_t f = earliestFrameId; f < earliestFrameId + forget_n_frames; f++) {
            fPath = (dirFrame + std::to_string(f)).c_str();
            stat(fPath, &sb);
            std::ifstream f_f(fPath, std::ios::binary);
            for (uint32_t _ = 0; _ < sb.st_size; _ += 2) {
                f_f.read(buf, 2);
                sid = (buf[1] << 8) | buf[0];
                sPath = (dirShapes + std::to_string(sid)).c_str();
                std::ifstream shf(fPath, std::ios::binary);
                shf.seekg(8);
                shf.read(reinterpret_cast<char *>(&y), 1);
                shf.read(reinterpret_cast<char *>(&u), 1);
                shf.read(reinterpret_cast<char *>(&v), 1);
                // TODO read ratio which should be calculated again!!
                shf.close();
                std::remove(sPath);

                // TODO remove from indices Y, U, V and R
            }
            f_f.close();
            std::remove(fPath);
        }
        LOGI("Forgetting time: %lld", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - t).count());
    }

    /** Saves current state: { nextFrameId, nextShapeId, earliestFrameId }
     * Don't save paths in variables in the constructor! */
    void SaveState() {
        std::ofstream ssf((visDirPath + savedStateFile).c_str(), std::ios::binary);
        ssf.write((char *) &nextFrameId, 8);
        ssf.write((char *) &nextShapeId, 2);
        ssf.write((char *) &earliestFrameId, 8);
        ssf.close();
    }
};

#endif //VIS_VISUAL_STM_H
