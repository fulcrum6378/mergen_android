#ifndef HPT_PAINFUL_POINT_H
#define HPT_PAINFUL_POINT_H

#include <cmath>

#include "../rew/criterion.hpp"
#include "../rew/rewarder.hpp"
#include "../scm/manifest.hpp"

#define INPUT_ID_TOUCH 4

class PainfulPoint : public Criterion {
public:
    explicit PainfulPoint(float weight) :
            Criterion(CRITERION_ID_PAINFUL_POINT, INPUT_ID_TOUCH, weight) {}

    void CheckForRewards(std::unordered_map<int16_t, bool> *on, float x_, float y_) {
        bool anyOn = false;
        for (auto o: *on)
            if (o.second) anyOn = true;
        auto screen = Manifest::interactions[INPUT_ID_TOUCH];
        bool inRange = false;
        if (anyOn) {
            float xM = screen.width / 2.0f, yM = 10.0f * (screen.height / 100.0f);
            float distance = sqrt(std::pow(x_ - xM, 2) + std::pow(y_ - yM, 2));
            if (distance < 150.0f) inRange = true;
        }
        if (inRange) {
            StopElasticity();
            score = score - 0.03f;
            if (score < -1.0f) score = -1.0f;
            Rewarder::Compute();
        } else if (score != 0.0f)
            StartElasticity();
        // else {}
    }

    ~PainfulPoint() override = default;
};

#endif //HPT_PAINFUL_POINT_H
