#ifndef VIS_VISUAL_STM_H
#define VIS_VISUAL_STM_H

#include <list>
#include <unordered_map>

#include "../global.h"

using namespace std;

/** Visual Short-Term Memory */
class VisualSTM {
private:
    const string visDirPath = "/data/data/ir.mahdiparastesh.mergen/files/vis/";
    // maximum frames allowed to be present in memory at a time
    const uint16_t max_frames_stored = 5;
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
    // helper maps for altering 'uint8_t' indexes
    unordered_map<uint8_t, list<uint16_t>> ym, um, vm;
    // helper maps for altering 'uint16_t' indexes
    unordered_map<uint16_t, list<uint16_t>> rm;

    /** Forgets some of oldest frames. */
    void Forget();

    /** Iterates on every ID in a Sequence File. */
    void IterateIndex(const char *path, void onEach(VisualSTM *, uint16_t));

    /** Reads an entire Sequence File. */
    list<uint16_t> ReadIndex(const char *path);

    /** Removes an ID from a list (index/sequence file). */
    void RemoveFromIndex(list<uint16_t> *l, uint16_t id);

    /** Save index in non-volatile memory and clear its data from RAM. */
    template<class INT>
    void SaveIndexes(unordered_map<INT, list<uint16_t>> *indexes, string *dir);

public:
    VisualSTM();

    /** Inserts a new shape into memory. */
    void Insert(
            uint8_t *m,
            uint16_t w, uint16_t h,
            list<pair<float, float>> path
    );

    /** Anything that needs to be done at the end. */
    void OnFrameFinished();

    /** Saves current state: { nextFrameId, nextShapeId, earliestFrameId }
     * Don't save paths in variables in the constructor! */
    void SaveState();
};

#endif //VIS_VISUAL_STM_H
