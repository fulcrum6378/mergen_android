#include <sstream>

#include "../global.h"
#include "touch.h"

void Touch::OnTouchEvent(int16_t dev, int8_t act, int16_t id, float x, float y, float size) {
    std::stringstream ss;
    ss << "DEV:" << dev << ", ";
    switch (act) {
        case 0: // ACTION_DOWN
        case 5: // ACTION_POINTER_1_DOWN
            ss << "CREATE";
            break;
        case 1: // ACTION_UP
        case 3: // ACTION_CANCEL
        case 4: // ACTION_OUTSIDE
        case 6: // ACTION_POINTER_1_UP
            ss << "DELETE";
            break;
        case 2:
            ss << "UPDATE";
            break;
    }
    ss << ", ID:" << id << ", X:" << x << ", Y:" << y << ", SIZE:" << size;
    LOGW("%s", ss.str().c_str());

    if (act == 0 || act == 2 || act == 5) {
        (*x_) = x;
        (*y_) = y;
    } else {
        x_ = nullptr;
        y_ = nullptr;
    }
    CheckForRewards();
}

void Touch::CheckForRewards() {
    auto screen = (*Manifest::input)[INPUT_ID_TOUCH];
    double final = 0.0;
    if (y_) final = (*y_) / ((double) screen.height / 100.0);
    rew_->SetScore(0, final);
}
