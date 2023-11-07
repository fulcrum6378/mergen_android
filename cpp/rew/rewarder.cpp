#include "../aud/beeping.hpp"
#include "../global.hpp"
#include "../hpt/painful_point.hpp"
#include "../mov/shaking.hpp"
#include "../vis/colouring.hpp"
#include "rewarder.hpp"

Rewarder::Rewarder(Speaker *aud_out, Vibrator *mov, JNIEnv *env, jobject main) {
    // define criterions
    AddCriterion(new Criterion(0, 0, 1.0));
    AddCriterion(new PainfulPoint(3.0));

    // define expressions
    AddExpression(new Shaking(mov));
    AddExpression(new Beeping(aud_out));
    AddExpression(new Colouring(env, main));
}

Criterion *Rewarder::GetCriterion(uint8_t criterionId) {
    return criteria[criterionId];
}

void Rewarder::AddCriterion(Criterion *criterion) {
    criteria[criterion->id] = criterion;
    scores[criterion->id] = 0.0;
}

void Rewarder::AddExpression(Expression *expression) {
    expressions[expression->id] = expression;
}

/** Retrieves the score of this criterion. */
double Rewarder::GetScore(uint8_t criterionId) {
    return scores[criterionId];
}

/** An interface for any sense to declare its own score. */
void Rewarder::SetScore(uint8_t criterionId, double score) {
    if (scores[criterionId] != score) scores[criterionId] = score;
}

/** Computes the `fortuna` score. */
void Rewarder::Compute() {
    double sum = 0.0;
    double totalWeights = 0.0;
    for (auto score: scores) {
        sum += score.second * criteria[score.first]->weight;
        totalWeights += criteria[score.first]->weight;
    }
    fortuna = sum / totalWeights;
    LOGI("Fortuna is %f", fortuna);
    for (auto exp: expressions) exp.second->OnReward(fortuna);
}

Rewarder::~Rewarder() {
    for (auto &exp: expressions) delete exp.second;
    for (auto &cri: criteria) delete cri.second;
}
