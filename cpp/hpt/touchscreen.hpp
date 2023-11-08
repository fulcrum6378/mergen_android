#ifndef HPT_TOUCHSCREEN_H
#define HPT_TOUCHSCREEN_H

#include "painful_point.hpp"

class Touchscreen {
public:
    Touchscreen();

    /** "int8_t dev" will corrupt the value! */
    void OnTouchEvent(int16_t dev, int8_t act, int16_t id, float x, float y, float size);

    ~Touchscreen();

private:
    std::unordered_map<int16_t, bool> on;
    float x_{0.0f}, y_{0.0f};
    PainfulPoint *painfulPoint{};
};

#endif //HPT_TOUCHSCREEN_H
