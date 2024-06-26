#ifndef REW_REWARDER_H
#define REW_REWARDER_H

#include <jni.h>

#include "../aud/speaker.hpp"
#include "../mov/vibrator.hpp"
#include "../scm/manifest.hpp"
#include "criterion.hpp"
#include "expression.hpp"

/**
 * Each SENSE can have multiple criteria with their importance values.
 * Rewarder must measure the weighted mean of all those criteria.
 */
class Rewarder {
public:
    Rewarder(Speaker *aud_out, Vibrator *mov, JavaVM *jvm, jobject main);

    static Criterion *GetCriterion(uint8_t criterionId);

    static void Compute();

    ~Rewarder();


    /** Range -1..+1 */
    inline static std::atomic<double> fortuna{0.0};

private:
    static void AddCriterion(Criterion *criterion);

    static void AddExpression(Expression *expression);

    inline static std::unordered_map<uint8_t, Criterion *> criteria;
    inline static std::unordered_map<uint8_t, Expression *> expressions;
};

#endif //REW_REWARDER_H
