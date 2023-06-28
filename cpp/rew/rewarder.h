#ifndef REW_REWARDER_H
#define REW_REWARDER_H

#include "../mem/manifest.h"

struct Criterion {
    uint16_t id;
    int8_t interaction_id;
    float weight;
    char const *desc;
};

/**
 * Each SENSE can have multiple criteria with their importance values.
 * Rewarder must measure the weighted mean of all those criteria.
 */
class Rewarder {
private:
    /** Range -128..127 */
    double fortuna{0.0};

    std::map<uint8_t, Criterion> *criteria;
    std::map<uint8_t, double> *scores;

    void Compute();

public:
    Rewarder();

    void SetScore(uint8_t criterionId, double score);

    ~Rewarder();
};

#endif //REW_REWARDER_H
