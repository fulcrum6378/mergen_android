#include "rewarder.h"

Rewarder::Rewarder() {
    criteria = new std::map<uint8_t, Criterion>{};
    AddCriterion(Criterion{
            0, 4, 1, "Gradient of pleasure and pain"});
    AddCriterion(Criterion{
            1, 0, 5, "Normality"});

    scores = new std::map<uint8_t, double>();
    for (auto cri: *criteria) (*scores)[cri.second.id] = 0.0;
}

void Rewarder::AddCriterion(Criterion criterion) {
    (*criteria)[(*criteria).size()] = criterion;
}

void Rewarder::AddOnRewardListener(Rewarder_OnRewardListener *listener) {
    rewardListeners[0] = listener;
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
    for (auto l : rewardListeners) if (l) l->onReward(fortuna);
}

Rewarder::~Rewarder() {
    delete &scores;
    scores = nullptr;
    delete &criteria;
    criteria = nullptr;
}
