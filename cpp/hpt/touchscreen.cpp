#include "../rew/rewarder.hpp"
#include "touchscreen.hpp"

Touchscreen::Touchscreen() : painfulPoint() {}

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

    // PainfulPoint
    if (!painfulPoint)
        painfulPoint = dynamic_cast<PainfulPoint *>(
                Rewarder::GetCriterion(CRITERION_ID_PAINFUL_POINT));
    painfulPoint->CheckForRewards((void *[3u]) {&on, &x_, &y_});
}

Touchscreen::~Touchscreen() = default;
