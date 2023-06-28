#include "vibrator.h"

void Vibrator::OnReward(double fortuna) {
    if (fortuna > -0.5 && fortuna < 0.5) return;
    double fortuna0to1 = (fortuna + 1.0) / 2.0;
    jlong milliseconds = 15;
    jmethodID method = env_->GetMethodID(
            env_->FindClass("ir/mahdiparastesh/mergen/Main"), "vibrate", "(JI)V");
    env_->CallVoidMethod(main_, method, milliseconds, 1 + (int32_t) (fortuna0to1 * 254));
}
