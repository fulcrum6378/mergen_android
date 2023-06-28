#include "vibrator.h"

void Vibrator::OnReward(double fortuna) {
    if (fortuna > 0.25 && fortuna < 0.75) return;
    jlong milliseconds = 8;
    jmethodID vibMethod = env_->GetMethodID(
            env_->FindClass("ir/mahdiparastesh/mergen/Main"), "vibrate", "(JI)V");
    env_->CallVoidMethod(main_, vibMethod, milliseconds, 1 + (int32_t) (fortuna * 254));
}
