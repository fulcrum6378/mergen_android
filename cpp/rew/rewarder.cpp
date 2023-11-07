#include "../aud/beeping.hpp"
#include "../global.hpp"
#include "../hpt/painful_point.hpp"
#include "../mov/shaking.hpp"
#include "../vis/colouring.hpp"
#include "rewarder.hpp"

Rewarder::Rewarder(Speaker *aud_out, Vibrator *mov, JNIEnv *env, jobject main) {
    // define criterions
    AddCriterion(new PainfulPoint(3.0f));
    AddCriterion(new Criterion(255u, 0, 1.0f));

    // define expressions
    AddExpression(new Shaking(mov));
    AddExpression(new Beeping(aud_out));
    AddExpression(new Colouring(env, main));
}

void Rewarder::AddCriterion(Criterion *criterion) {
    criteria[criterion->id] = criterion;
}

Criterion *Rewarder::GetCriterion(uint8_t criterionId) {
    return criteria[criterionId];
}

void Rewarder::AddExpression(Expression *expression) {
    expressions[expression->id] = expression;
}

/** Computes the `fortuna` score. */
void Rewarder::Compute() {
    double sum = 0.0;
    float totalWeights = 0.0f;
    for (auto cri: criteria) {
        sum += cri.second->score * cri.second->weight;
        totalWeights += cri.second->weight;
    }
    fortuna = sum / totalWeights;
    LOGI("Fortuna is %f", fortuna.load());
    for (auto exp: expressions) exp.second->OnReward(fortuna);
}

Rewarder::~Rewarder() {
    for (auto &exp: expressions) delete exp.second;
    for (auto &cri: criteria) delete cri.second;
}
