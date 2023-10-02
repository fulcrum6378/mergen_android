#include <cmath>

#include "colouring.h"

void Colouring::OnReward(double fortuna) {
    int32_t ret;
    if (fortuna > 0) ret = /*green*/ 0x004CAF50;
    else if (fortuna < 0) ret = /*red*/ 0x00F44336;
    else ret = 0x00000000;
    if (ret != 0x00000000)
        ret = ret | static_cast<int8_t>(ceil(std::abs(fortuna) * 255.0)) << 24;
    jmethodID method = env_->GetMethodID(
            env_->FindClass("ir/mahdiparastesh/mergen/Main"), "colouring", "(I)V");
    env_->CallVoidMethod(main_, method, ret);
}
