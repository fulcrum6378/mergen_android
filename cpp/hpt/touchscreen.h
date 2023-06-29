#ifndef HPT_TOUCH_H
#define HPT_TOUCH_H

#include "../rew/rewarder.h"

#define INPUT_ID_TOUCH 4

class Touchscreen {
private:
    Rewarder *rew_;
    std::map<int16_t, bool> *on;
    float x_{0.0};
    float y_{0.0};

    void CheckForRewards();

public:
    Touchscreen(Rewarder *rew);

    /** "int8_t dev" will corrupt the value! */
    void OnTouchEvent(int16_t dev, int8_t act, int16_t id, float x, float y, float size);

    ~Touchscreen();
};

#endif //HPT_TOUCH_H
