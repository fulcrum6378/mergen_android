#ifndef HPT_PAINFUL_POINT_H
#define HPT_PAINFUL_POINT_H

#include <cmath>

#include "../manifest.hpp"
#include "../rew/criterion.hpp"
#include "../rew/rewarder.hpp"

#define INPUT_ID_TOUCH 4

class PainfulPoint : public Criterion {
public:
    explicit PainfulPoint(float weight) :
            Criterion(CRITERION_ID_PAINFUL_POINT, INPUT_ID_TOUCH, weight) {}

    void CheckForRewards(void **data) override {
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
