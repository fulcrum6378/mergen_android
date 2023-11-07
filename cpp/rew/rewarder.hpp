#ifndef REW_REWARDER_H
#define REW_REWARDER_H

#include <jni.h>

#include "../aud/speaker.hpp"
#include "../manifest.hpp"
#include "../mov/vibrator.hpp"
#include "criterion.hpp"
#include "expression.hpp"

/**
 * Each SENSE can have multiple criteria with their importance values.
 * Rewarder must measure the weighted mean of all those criteria.
 */
class Rewarder {
private:
    /** Range -1..+1 */
    double fortuna{0.0};

    std::unordered_map<uint8_t, Criterion *> criteria;
    std::unordered_map<uint8_t, double> scores;
    std::unordered_map<uint8_t, Expression *> expressions;

    void AddCriterion(Criterion *criterion);

    void AddExpression(Expression *expression);

public:
    Rewarder(Speaker *aud_out, Vibrator *mov, JNIEnv *env, jobject main);

    Criterion *GetCriterion(uint8_t criterionId);

    double GetScore(uint8_t criterionId);

    void SetScore(uint8_t criterionId, double score);

    void Compute();

    ~Rewarder();
};

#endif //REW_REWARDER_H
