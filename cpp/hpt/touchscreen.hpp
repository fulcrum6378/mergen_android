#ifndef HPT_TOUCH_H
#define HPT_TOUCH_H

#include "../rew/rewarder.hpp"
#include "painful_point.hpp"

class Touchscreen {
private:
    Rewarder **rew_;
    std::unordered_map<int16_t, bool> on;
    float x_{0.0}, y_{0.0};
    PainfulPoint *painfulPoint{};

public:
    explicit Touchscreen(Rewarder **rew);

    /** "int8_t dev" will corrupt the value! */
    void OnTouchEvent(int16_t dev, int8_t act, int16_t id, float x, float y, float size);

    ~Touchscreen();
};

#endif //HPT_TOUCH_H
