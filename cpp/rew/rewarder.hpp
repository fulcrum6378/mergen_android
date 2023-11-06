#ifndef REW_REWARDER_H
#define REW_REWARDER_H

#include "../manifest.hpp"
#include "../mov/vibrator.hpp"
#include "../rew/expression.hpp"
#include "../vis/colouring.hpp"

struct Criterion {
    uint8_t id;
    [[maybe_unused]] int8_t interaction_id;
    float weight;
    [[maybe_unused]] char const *desc;
};

/**
 * Each SENSE can have multiple criteria with their importance values.
 * Rewarder must measure the weighted mean of all those criteria.
 */
class Rewarder {
private:
    /** Range -1..+1 */
    double fortuna{0.0};

    std::unordered_map<uint8_t, Criterion> criteria;
    std::unordered_map<uint8_t, double> scores;
    std::unordered_map<uint8_t, Expression *> expressions;

    void AddCriterion(Criterion criterion);

    void AddExpression(Expression *expression);

    void Compute();

public:
    Rewarder(Vibrator *mov, Colouring *vis_out);

    void SetScore(uint8_t criterionId, double score);

    ~Rewarder();
};

#endif //REW_REWARDER_H
