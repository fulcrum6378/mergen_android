#ifndef HPT_TOUCH_H
#define HPT_TOUCH_H

#include "../rew/rewarder.h"

#define INPUT_ID_TOUCH 4

class Touch {
private:
    Rewarder *rew_;

public:
    Touch(Rewarder *rew) : rew_(rew) {}

    /** "int8_t dev" will corrupt the value! */
    void OnTouchEvent(int16_t dev, int8_t act, int16_t id, float x, float y, float size);

    ~Touch() {}
};

#endif //HPT_TOUCH_H
