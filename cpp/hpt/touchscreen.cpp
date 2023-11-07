#include "touchscreen.hpp"

Touchscreen::Touchscreen(Rewarder **rew) : rew_(rew), painfulPoint() {}

/** Device ID (dev) is always 3 in my phone. */
void Touchscreen::OnTouchEvent(int16_t /*dev*/, int8_t act, int16_t id, float x, float y,
                               float /*size*/) {
    //std::stringstream ss; #include <sstream>
    //ss << "DEV:" << dev << ", ";
    switch (act) {
        case 0: // ACTION_DOWN
        case 5: // ACTION_POINTER_1_DOWN
            on[id] = true;
            //ss << "CREATE";
            break;
        case 1: // ACTION_UP
        case 3: // ACTION_CANCEL
        case 4: // ACTION_OUTSIDE
        case 6: // ACTION_POINTER_1_UP
            //ss << "DELETE";
            on[id] = false;
            break;
        case 2:
            //ss << "UPDATE";
            break;
    }
    //ss << ", ID:" << id << ", X:" << x << ", Y:" << y << ", SIZE:" << size;
    //LOGW("%s", ss.str().c_str());

    if (act == 0 || act == 2 || act == 5) {
        x_ = x;
        y_ = y;
    }

    if (!painfulPoint)
        painfulPoint = dynamic_cast<PainfulPoint *>(
                (*rew_)->GetCriterion(CRITERION_ID_PAINFUL_POINT));
    double score = painfulPoint->CheckForRewards(
            (*rew_)->GetScore(CRITERION_ID_PAINFUL_POINT),
            (void *[3]) {&on, &x_, &y_});
    if (score != -2.0) {
        (*rew_)->SetScore(CRITERION_ID_PAINFUL_POINT, score);
        (*rew_)->Compute();
    }
}

Touchscreen::~Touchscreen() = default;
