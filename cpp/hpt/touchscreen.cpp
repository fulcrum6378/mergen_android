#include <cmath>

#include "../global.hpp"
#include "touchscreen.hpp"

Touchscreen::Touchscreen(Rewarder *rew) : rew_(rew) {}

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

    CheckForRewards();
}

void Touchscreen::CheckForRewards() {
    bool anyOn = false;
    for (auto o: on) if (o.second) anyOn = true;
    auto screen = Manifest::interactions[INPUT_ID_TOUCH];
    bool inRange = false;
    if (anyOn) {
        double xM = (double) screen.width / 2.0, yM = 10.0 * ((double) screen.height / 100.0);
        double distance = sqrt(pow(x_ - xM, 2) + pow(y_ - yM, 2));
        if (distance < 150.0) inRange = true;
    }
    if (inRange) {
        score -= 0.03;
        if (score < -1.0) score = -1.0;
    } else score = 0.0;
    rew_->SetScore(0, score);
}

Touchscreen::~Touchscreen() = default;
