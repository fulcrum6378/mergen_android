#ifndef VIS_SHORT_TERM_MEMORY_H
#define VIS_SHORT_TERM_MEMORY_H

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>

#include "global.h"

class ShortTermMemory {
private:
    std::string visDirPath = "/data/data/ir.mahdiparastesh.mergen/files/vis/";
    std::string dirShapes = "shapes";
    std::string dirY = "y";
    std::string dirU = "u";
    std::string dirV = "v";
    std::string dirRt = "r";
    uint16_t nextId;
    const char *nextIdPath;

public:
    ShortTermMemory() {
        // create directories if they don't exist and resolves their path variables
        struct stat sb;
        std::string root("");
        for (std::string *dir: {&root, &dirShapes, &dirY, &dirU, &dirV, &dirRt}) {
            std::string branch = (*dir);
            if (branch != "") {
                dir->insert(0, visDirPath);
                dir->append("/");
            }
            auto path = (visDirPath + branch).c_str();
            if (stat(path, &sb) != 0) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

        // find ID of the latest shape
        nextIdPath = (visDirPath + "next_shape_id").c_str();
        if (stat(nextIdPath, &sb) >= 2) {
            std::ifstream ssf(nextIdPath, std::ios::binary);
            char buf[2];
            ssf.read(buf, 2);
            nextId = (buf[0] << 8) + buf[1];
            ssf.close();
        } else nextId = 0;
    }

    void Insert(
            uint8_t *m,
            uint16_t w,
            uint16_t h,
            std::list<std::pair<float, float>> path
    ) {
        // write the shape file
        std::ofstream shf(dirShapes + std::to_string(nextId), std::ios::binary);
        shf.put(m[0]);
        shf.put(m[1]);
        shf.put(m[2]);
        shf.write((char *) &w, sizeof(w));
        shf.write((char *) &h, sizeof(h));
        for (std::pair p: path) {
            shf.write((char *) &p.first, sizeof(p.first));
            shf.write((char *) &p.second, sizeof(p.second));
        }
        shf.close();

        // update Y index
        std::ofstream y_f((dirY + std::to_string(m[0])).c_str(),
                          std::ios::app | std::ios::binary);
        y_f.write((char *) &nextId, sizeof(nextId));
        y_f.close();

        // update U index
        std::ofstream u_f((dirU + std::to_string(m[1])).c_str(),
                          std::ios::app | std::ios::binary);
        u_f.write((char *) &nextId, sizeof(nextId));
        u_f.close();

        // update V index
        std::ofstream v_f((dirV + std::to_string(m[2])).c_str(),
                          std::ios::app | std::ios::binary);
        v_f.write((char *) &nextId, sizeof(nextId));
        v_f.close();

        // update Ratio index
        int16_t ratio = round(((float) w / (float) h) * 10);
        std::ofstream rtf((dirRt + std::to_string(ratio)).c_str(),
                          std::ios::app | std::ios::binary);
        rtf.write((char *) &nextId, sizeof(nextId));
        rtf.close();

        nextId++;
        if (nextId > 65535) nextId = 0;
    }

    void SaveState() { // FIXME Segmentation::Process cannot write, but the main thread can!!?!
        std::ofstream ssf(nextIdPath, std::ios::binary);
        // LOGI("%d %d", ssf.is_open(), ssf.good());
        ssf.write((char *) &nextId, sizeof(nextId));
        ssf.close();
    }
};

#endif //VIS_SHORT_TERM_MEMORY_H
