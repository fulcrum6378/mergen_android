#include "vibrator.h"

void Vibrator::OnReward(double fortuna) {
    jint amplitude_;
    if (fortuna >= -0.75 && fortuna < 0.75) amplitude_ = 0;
    else amplitude_ = 1 + (int32_t) (((fortuna + 1.0) / 2.0) * 254);
    jmethodID method = env_->GetMethodID(
            env_->FindClass("ir/mahdiparastesh/mergen/Main"), "vibrate", "(I)V");
    env_->CallVoidMethod(main_, method, amplitude_);
}
