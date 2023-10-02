#include "../global.h"
#include "../mov/vibrator.h"
#include "../vis/colouring.h"
#include "rewarder.h"

Rewarder::Rewarder(JNIEnv *env, jobject main) {
    // define criterions
    AddCriterion(Criterion{
            0, 4, 4, "Painful point"});
    AddCriterion(Criterion{
            1, 0, 1, "Normality"});

    // define expressions
    AddExpression(new Vibrator(env, main));
    AddExpression(new Colouring(env, main));
}

void Rewarder::AddCriterion(Criterion criterion) {
    criteria[criterion.id] = criterion;
    scores[criterion.id] = 0.0;
}

void Rewarder::AddExpression(Expression *expression) {
    expressions[expression->id] = expression;
}

/** An interface for any sense to declare its score. */
void Rewarder::SetScore(uint8_t criterionId, double score) {
    // ASSERT(score >= -1.0 && score <= 1.0, "Invalid reward score!")
    if (scores[criterionId] == score) return;
    scores[criterionId] = score;
    Compute();
}

/** Computes the "fortuna" score. */
void Rewarder::Compute() {
    double sum = 0.0;
    double totalWeights = 0.0;
    for (auto score: scores) {
        sum += score.second * criteria[score.first].weight;
        totalWeights += criteria[score.first].weight;
    }
    fortuna = sum / totalWeights;
    LOGI("Fortuna is %f", fortuna);
    for (auto exp: expressions) exp.second->OnReward(fortuna);
}

Rewarder::~Rewarder() {
    for (auto &exp: expressions) delete exp.second;
}
