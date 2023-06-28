#include "rewarder.h"

Rewarder::Rewarder() {
    criteria = new std::map<uint8_t, Criterion>{};
    (*criteria)[0] = Criterion{0, 4, 1, "Gradient of pleasure and pain"};

    scores = new std::map<uint8_t, double>();
    for (auto cri: *criteria) (*scores)[cri.second.id] = 0.0;
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
}

Rewarder::~Rewarder() {
    delete &scores;
    scores = nullptr;
    delete &criteria;
    criteria = nullptr;
}
