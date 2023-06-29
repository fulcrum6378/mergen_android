#include <cmath>
#include <sstream>

#include "../global.h"
#include "touchscreen.h"

Touchscreen::Touchscreen(Rewarder *rew) : rew_(rew) {
    on = new std::map<int16_t, bool>();
}

void Touchscreen::OnTouchEvent(int16_t dev, int8_t act, int16_t id, float x, float y, float size) {
    std::stringstream ss;
    ss << "DEV:" << dev << ", ";
    switch (act) {
        case 0: // ACTION_DOWN
        case 5: // ACTION_POINTER_1_DOWN
            (*on)[id] = true;
            ss << "CREATE";
            break;
        case 1: // ACTION_UP
        case 3: // ACTION_CANCEL
        case 4: // ACTION_OUTSIDE
        case 6: // ACTION_POINTER_1_UP
            ss << "DELETE";
            (*on)[id] = false;
            break;
        case 2:
            ss << "UPDATE";
            break;
    }
    ss << ", ID:" << id << ", X:" << x << ", Y:" << y << ", SIZE:" << size;
    LOGW("%s", ss.str().c_str());

    if (act == 0 || act == 2 || act == 5) {
        x_ = x;
        y_ = y;
    }

    CheckForRewards();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "UnreachableCode"
#pragma ide diagnostic ignored "ConstantConditionsOC"

void Touchscreen::CheckForRewards() {
    bool anyOn = false;
    for (auto o: *on) if (o.second) anyOn = true;
    auto screen = (*Manifest::input)[INPUT_ID_TOUCH];

    /*double final = 0.0;
    if (anyOn) final = (((y_ / (double) screen.height) * 2.0) - 1.0) * -1;
    rew_->SetScore(0, final);*/

    /*bool inRange = false;
    if (anyOn) {
        double hCent = (double) screen.height / 100.0, yFactor = y_ / hCent,
                minYPercent = 80.0, maxYPercent = 90.0;
        if (yFactor >= minYPercent && yFactor < maxYPercent) {
            double circleSize = (maxYPercent * hCent) - (minYPercent * hCent);
            double minX = (screen.width - circleSize) / 2.0, maxX = minX + circleSize;
            if (x_ >= minX && x_ < maxX) {
                inRange = true;

            }
        }
    }
    if (inRange) rew_->SetScore(0, -1.0);
    else rew_->SetScore(0, 0.0);*/

    // √[(x2 − x1)2 + (y2 − y1)2]
    bool inRange = false;
    if (anyOn) {
        double xM = (double) screen.width / 2.0, yM = 0.0 * ((double) screen.height / 100.0);
        double distance = sqrt(pow(x_ - xM, 2) + pow(y_ - yM, 2));
        std::stringstream ss;
        ss << "DISTANCE IS " << distance;
        LOGW("%s", ss.str().c_str());
        if (distance < 150.0) inRange = true;
    }
    if (inRange) rew_->SetScore(0, -1.0);
    else rew_->SetScore(0, 0.0);
}

#pragma clang diagnostic pop

Touchscreen::~Touchscreen() {
    delete on;
    on = nullptr;
}
