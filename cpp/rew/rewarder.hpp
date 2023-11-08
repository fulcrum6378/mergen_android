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
    inline static std::atomic<double> fortuna{0.0};

    inline static std::unordered_map<uint8_t, Criterion *> criteria;
    inline static std::unordered_map<uint8_t, Expression *> expressions;

    static void AddCriterion(Criterion *criterion);

    static void AddExpression(Expression *expression);

public:
    Rewarder(Speaker *aud_out, Vibrator *mov, JavaVM *jvm, jobject main);

    static Criterion *GetCriterion(uint8_t criterionId);

    static void Compute();

    ~Rewarder();
};

#endif //REW_REWARDER_H
