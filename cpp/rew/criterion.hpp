#ifndef REW_CRITERION_H
#define REW_CRITERION_H

#include <atomic>
#include <cstdint>
#include <thread>

#define CRITERION_ID_PAINFUL_POINT 0u
// ID `255`=>"Normality" will be removed later

class Criterion {
public:
    Criterion(uint8_t _id, int8_t _interactionId, float _weight) :
            id(_id), interactionId(_interactionId), weight(_weight) {}

    //virtual void CheckForRewards(std::vector<void *> data) {}
    // we better don't hardcode this function.

    virtual ~Criterion() = default;


    uint8_t id;
    [[maybe_unused]] int8_t interactionId;
    const float weight;
    std::atomic<float> score{0.0f};

protected:
    void StartElasticity() {
        elasticity = new std::thread(&Criterion::Elasticity, this);
        elasticity->detach();
    }

    void StopElasticity() {
        elasticise = false;
    }

private:
    /** Implemented at the bottom of `rewarder.cpp`. */
    void Elasticity();


    constexpr static const float elsApproach = 0.02f;
    constexpr static const int64_t elsFrame = 20ll;
    std::thread *elasticity = nullptr;
    std::atomic<bool> elasticise = false;
};

#endif //REW_CRITERION_H
