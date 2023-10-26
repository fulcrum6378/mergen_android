#ifndef VIS_VISUAL_STM_H
#define VIS_VISUAL_STM_H

#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>

// Shapes' paths can be saved in 2 ways:      uint16_t, uint32_t
#define SHAPE_POINT_T uint16_t
static int8_t shape_point_bytes = 2;       // 2,        4
static uint8_t shape_point_each_bits = 8;  // 8,        16
static float shape_point_max = 256.0;      // 256.0,    65535.0
// don't make them compiler-level constants, because of their types.

// maximum frames allowed to be present in memory at a time
#define MAX_FRAMES_STORED 10
// forget N frames whenever hit the maximum
#define FORGET_N_FRAMES 1

static bool littleEndian = std::endian::native == std::endian::little;

/** Visual Short-Term Memory */
class VisualSTM {
private:
    const std::string visDirPath = "/data/data/ir.mahdiparastesh.mergen/files/vis/stm/";
    std::string dirShapes = "shapes", dirY = "y", dirU = "u", dirV = "v", dirRt = "r",
            framesFile = "frames", savedStateFile = "saved_state";
    // frame ID incrementor | ID of earliest frame which is STILL available in memory
    uint64_t nextFrameId = 0, firstFrameId = 0;
    // shape ID incrementor | ID of first shape in THIS FRAME
    uint16_t nextShapeId = 0, firstShapeId = 0;
    // total number of frames available in memory
    uint16_t framesStored = 0;
    // frame index (8-bit)
    std::map<uint64_t, std::pair<uint16_t, uint16_t>> fi;
    // 1-bit volatile indices
    std::map<uint8_t, std::unordered_set<uint16_t>> yi, ui, vi;
    // 2-bit volatile indices
    std::map<uint16_t, std::unordered_set<uint16_t>> ri;

    /** Forgets N of oldest frames. */
    void Forget();

    /** Save an index in non-volatile memory. */
    template<class INT>
    void SaveIndexes(std::unordered_map<INT, std::list<uint16_t>> *indexes, std::string *dir);

public:
    VisualSTM();

    /** Inserts a new shape into memory. */
    void Insert(
            uint8_t **m, // average colour
            uint16_t *w, uint16_t *h, // width and height
            uint16_t cx, uint16_t cy, // central points
            std::unordered_set<SHAPE_POINT_T> *path
    );

    /** Anything that needs to be done at the end. */
    void OnFrameFinished();

    /** Saves current state { nextFrameId, nextShapeId, earliestFrameId }
     * Don't save paths as variables in the constructor! */
    void SaveState();
};

#endif //VIS_VISUAL_STM_H
