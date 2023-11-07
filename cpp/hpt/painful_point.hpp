#ifndef HPT_PAINFUL_POINT_H
#define HPT_PAINFUL_POINT_H

#include <cmath>
#include <thread>

#include "../manifest.hpp"
#include "../rew/criterion.hpp"

#define INPUT_ID_TOUCH 4

class PainfulPoint : public Criterion {
private:
    std::thread elasticity;

public:
    explicit PainfulPoint(float weight) :
            Criterion(CRITERION_ID_PAINFUL_POINT, INPUT_ID_TOUCH, weight) {}

    double CheckForRewards(double score, void **data) override {
        bool anyOn = false;
        for (auto o: *static_cast<std::unordered_map<int16_t, bool> *>(data[0]))
            if (o.second) anyOn = true;
        auto screen = Manifest::interactions[INPUT_ID_TOUCH];
        bool inRange = false;
        if (anyOn) {
            double xM = (double) screen.width / 2.0, yM = 10.0 * ((double) screen.height / 100.0),
                    x_ = *static_cast<float *>(data[1]),
                    y_ = *static_cast<float *>(data[2]);
            double distance = sqrt(pow(x_ - xM, 2) + pow(y_ - yM, 2));
            if (distance < 150.0) inRange = true;
        }
        if (inRange) {
            score -= 0.03;
            if (score < -1.0) score = -1.0;
            return score;
        } else if (score != 0) {
            elasticity = std::thread(&PainfulPoint::Elasticity, this);
            elasticity.detach();
            return -2.0;
        } else return -2.0;
    }

    void Elasticity() {
        // TODO gradual decrease in pain
    }

    ~PainfulPoint() override = default;
};

#endif //HPT_PAINFUL_POINT_H
