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
    std::string visDirPath = "/data/data/ir.mahdiparastesh.mergen/files/vis/",
            dirShapes = "shapes", dirFrame = "f", dirY = "y", dirU = "u", dirV = "v", dirRt = "r",
            nextFrameIdFile = "next_frame_id", nextShapeIdFile = "next_shape_id";
    uint64_t nextFrameId;
    uint16_t nextShapeId;
    std::list <uint16_t> shapesInFrame;

public:
    VisualSTM() {
        // create directories if they don't exist and resolves their path variables
        struct stat sb;
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

        // find ID of the latest shape
        const char *nextShapeIdPath = (visDirPath + nextShapeIdFile).c_str();
        if (std::filesystem::exists(nextShapeIdPath)) {
            std::ifstream nsf(nextShapeIdPath, std::ios::binary);
            char buf[2];
            nsf.read(buf, 2);
            nextShapeId = (buf[1] << 8) | buf[0];
            nsf.close();
        }

        // find ID of the latest frame
        const char *nextFrameIdPath = (visDirPath + nextFrameIdFile).c_str();
        if (std::filesystem::exists(nextFrameIdPath)) {
            std::ifstream nff(nextFrameIdPath, std::ios::binary);
            char buf[8];
            nff.read(buf, 8);
            nextFrameId = ((uint64_t) buf[7] << 56) | ((uint64_t) buf[6] << 48) |
                          ((uint64_t) buf[5] << 40) | ((uint64_t) buf[4] << 32) |
                          (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
            nff.close();
        }
    }

    void Insert(
            uint8_t *m,
            uint16_t w,
            uint16_t h,
            std::list<std::pair<float, float>> path
    ) {
        // write the shape file
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

        shapesInFrame.push_back(nextShapeId);
        nextShapeId++;
        if (nextShapeId > 65535) nextShapeId = 0;
    }

    /** Anything that needs to be done at the end.
     * Don't save paths in variables in the constructor! */
    void SaveState() {
        // save the Frame Index
        std::ofstream f_f((dirFrame + std::to_string(nextFrameId)).c_str(), std::ios::binary);
        for (uint16_t sid: shapesInFrame)
            f_f.write((char *) &sid, 2);
        f_f.close();
        shapesInFrame.clear();
        nextFrameId++;
        // if (nextFrameId > 18446744073709552000) nextShapeId = 0;

        // save the `nextFrameId`
        std::ofstream nff((visDirPath + nextFrameIdFile).c_str(), std::ios::binary);
        nff.write((char *) &nextFrameId, 8);
        nff.close();

        // save the `nextShapeId`
        std::ofstream nsf((visDirPath + nextShapeIdFile).c_str(), std::ios::binary);
        nsf.write((char *) &nextShapeId, 2);
        nsf.close();
    }
};

#endif //VIS_VISUAL_STM_H
