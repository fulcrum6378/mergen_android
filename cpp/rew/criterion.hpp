#ifndef REW_CRITERION_H
#define REW_CRITERION_H

#include <cstdint>

// ID `0` => "Normality"
#define CRITERION_ID_PAINFUL_POINT 1

class Criterion {
public:
    uint8_t id;
    [[maybe_unused]] int8_t interactionId;
    float weight;

    Criterion(uint8_t id, int8_t interactionId, float weight) {
        this->id = id;
        this->interactionId = interactionId;
        this->weight = weight;
    }

    virtual double CheckForRewards(double score, void **data) {}

    virtual ~Criterion() = default;
};

#endif //REW_CRITERION_H
