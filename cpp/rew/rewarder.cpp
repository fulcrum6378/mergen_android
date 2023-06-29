#include <sstream>

#include "../global.h"
#include "../mov/vibrator.h"
#include "../vis/colouring.h"
#include "rewarder.h"

Rewarder::Rewarder(JNIEnv *env, jobject main) {
    criteria = new std::map<uint8_t, Criterion>{};
    AddCriterion(Criterion{
            0, 4, 4, "Painful point"});
    AddCriterion(Criterion{
            1, 0, 1, "Normality"});

    scores = new std::map<uint8_t, double>();
    for (auto cri: *criteria) (*scores)[cri.second.id] = 0.0;

    expressions = new std::map<uint16_t, Expression *>{};
    AddExpression(new Vibrator(env, main));
    AddExpression(new Colouring(env, main));
}

void Rewarder::AddCriterion(Criterion criterion) {
    (*criteria)[(*criteria).size()] = criterion;
}

void Rewarder::AddExpression(Expression *expression) {
    (*expressions)[expression->id] = expression;
}

/** An interface for any sense to declare its score. */
void Rewarder::SetScore(uint8_t criterionId, double score) {
    ASSERT(score >= -1.0 && score <= 1.0, "Invalid reward score!")
    (*scores)[criterionId] = score;
    Compute();
}

/** Computes the "fortuna" score. */
void Rewarder::Compute() {
    double sum = 0.0;
    double totalWeights = 0.0;
    for (auto score: *scores) {
        sum += score.second * (*criteria)[score.first].weight;
        totalWeights += (*criteria)[score.first].weight;
    }
    fortuna = sum / totalWeights;
    std::stringstream ss;
    ss << "Fortuna is " << fortuna;
    LOGW("%s", ss.str().c_str());
    for (auto exp: *expressions) exp.second->OnReward(fortuna);
}

Rewarder::~Rewarder() {
    for (auto exp: *expressions) delete exp.second;
    delete &expressions;
    expressions = nullptr;
    delete &scores;
    scores = nullptr;
    delete &criteria;
    criteria = nullptr;
}
