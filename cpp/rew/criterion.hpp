#ifndef REW_CRITERION_H
#define REW_CRITERION_H

#include <atomic>
#include <cstdint>
#include <thread>

#define CRITERION_ID_PAINFUL_POINT 0u
// ID `255`=>"Normality" will be removed later

class Criterion {
public:
    uint8_t id;
    [[maybe_unused]] int8_t interactionId;
    const float weight;
    std::atomic<float> score{0.0f};

    Criterion(uint8_t _id, int8_t _interactionId, float _weight) :
            id(_id), interactionId(_interactionId), weight(_weight) {}

    virtual void CheckForRewards(void **data) {}

    virtual ~Criterion() = default;

protected:
    std::thread *elasticity = nullptr;

    /** Implemented at the bottom of `rewarder.cpp`. */
    void Elasticity();
};

#endif //REW_CRITERION_H
