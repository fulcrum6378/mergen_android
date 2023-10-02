#ifndef REW_REWARDER_H
#define REW_REWARDER_H

#include "../mem/manifest.h"
#include "../rew/expression.h"

struct Criterion {
    uint16_t id;
    [[maybe_unused]] int8_t interaction_id;
    float weight;
    [[maybe_unused]] char const *desc;
};

/**
 * Each SENSE can have multiple criteria with their importance values.
 * Rewarder must measure the weighted mean of all those criteria.
 */
class Rewarder {
private:
    /** Range -1..+1 */
    double fortuna{0.0};

    std::unordered_map<uint8_t, Criterion> criteria;
    std::unordered_map<uint8_t, double> scores;
    std::unordered_map<uint16_t, Expression *> expressions;

    void AddCriterion(Criterion criterion);

    void AddExpression(Expression *expression);

    void Compute();

public:
    Rewarder(JNIEnv *env, jobject main);

    void SetScore(uint8_t criterionId, double score);

    ~Rewarder();
};

#endif //REW_REWARDER_H
