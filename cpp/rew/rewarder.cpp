#include <sstream>

#include "../global.h"
#include "rewarder.h"

Rewarder::Rewarder(JNIEnv *env, jobject main) {
    criteria = new std::map<uint8_t, Criterion>{};
    AddCriterion(Criterion{
            0, 4, 1, "Gradient of pleasure and pain"});
    /*AddCriterion(Criterion{
            1, 0, 5, "Normality"});*/

    scores = new std::map<uint8_t, double>();
    for (auto cri: *criteria) (*scores)[cri.second.id] = 0.0;

    expressions = new std::map<uint8_t, Expression *>{};
    (*expressions)[OUTPUT_ID_VIBRATOR] = new Vibrator(env, main);
}

void Rewarder::AddCriterion(Criterion criterion) {
    (*criteria)[(*criteria).size()] = criterion;
}

/** An interface for any sense to declare its score. */
void Rewarder::SetScore(uint8_t criterionId, double score) {
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
    ss  << "Fortuna is " << fortuna;
    LOGW("%s", ss.str().c_str());
    for (auto exp: *expressions) exp.second->OnReward(fortuna);
}

Rewarder::~Rewarder() {
    delete &expressions;
    expressions = nullptr;
    delete &scores;
    scores = nullptr;
    delete &criteria;
    criteria = nullptr;
}
