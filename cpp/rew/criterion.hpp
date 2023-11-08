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
    void StartElasticity() {
        elasticity = new std::thread(&Criterion::Elasticity, this);
        elasticity->detach();
    }

    void StopElasticity() {
        if (elasticity && elasticity->joinable()) {
            elasticise = false;
            elasticity->join();
        }
    }

private:
    constexpr static const float elasticityApproach = 0.02f;
    constexpr static const int64_t elasticityFrame = 20ll;
    std::thread *elasticity = nullptr;
    std::atomic<bool> elasticise = false;

    /** Implemented at the bottom of `rewarder.cpp`. */
    void Elasticity();
};

#endif //REW_CRITERION_H
