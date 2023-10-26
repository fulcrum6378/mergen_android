#ifndef VIS_COLOURING_H
#define VIS_COLOURING_H

#include <cmath>

#include "../rew/expression.hpp"

#define OUTPUT_ID_SCREEN (-1)

class Colouring : public Expression {
public:
    Colouring(JNIEnv *env, jobject main) :
            Expression(EXPRESSION_ID_SCREEN_1, OUTPUT_ID_SCREEN, env, main) {}

    void OnReward(double fortuna) override {
        int32_t ret;
        if (fortuna > 0) ret = /*green*/ 0x004CAF50;
        else if (fortuna < 0) ret = /*red*/ 0x00F44336;
        else ret = 0x00000000;
        if (ret != 0x00000000)
            ret = ret | static_cast<int8_t>(ceil(abs(fortuna) * 255.0)) << 24;
        jmethodID method = env_->GetMethodID(
                env_->FindClass("ir/mahdiparastesh/mergen/Main"), "colouring", "(I)V");
        env_->CallVoidMethod(main_, method, ret);
    }

    ~Colouring() override = default;
};

#endif //VIS_COLOURING_H
