#include "../global.hpp"
#include "rewarder.hpp"

Rewarder::Rewarder(Vibrator *mov, Colouring *vis_out) {
    // define criterions
    AddCriterion({0, 4, 4, "Painful point"});
    AddCriterion({1, 0, 1, "Normality"});

    // define expressions
    AddExpression(mov);
    AddExpression(vis_out);
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
    scores[criterionId] = score; // TODO why does this crash when Rewarder in initialised after the senses?!?
    Compute();
}

/** Computes the `fortuna` score. */
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

Rewarder::~Rewarder() = default;
