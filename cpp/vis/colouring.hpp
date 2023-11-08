#ifndef VIS_COLOURING_H
#define VIS_COLOURING_H

#include <cmath>

#include "../global.hpp"
#include "../rew/expression.hpp"

#define OUTPUT_ID_SCREEN (-1)

class Colouring : public Expression {
public:
    Colouring(JavaVM *jvm, jobject main) :
            Expression(EXPRESSION_ID_COLOURING, OUTPUT_ID_SCREEN),
            jvm_(jvm), main_(main) {
        JNIEnv *env;
        jvm_->GetEnv((void **) &env, JNI_VERSION_1_6);
        jmColouring = env->GetMethodID(
                env->FindClass("ir/mahdiparastesh/mergen/Main"), "colouring", "(I)V");
    }

    void OnReward(double fortuna) override {
        int32_t ret;
        if (fortuna > 0) ret = /*green*/ 0x004CAF50;
        else if (fortuna < 0) ret = /*red*/ 0x00F44336;
        else ret = 0x00000000;
        if (ret != 0x00000000)
            ret = ret | static_cast<int8_t>(ceil(std::abs(fortuna) * 255.0)) << 24;
        JNIEnv *env;
        if (jvm_->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_EDETACHED)
            jvm_->AttachCurrentThread(&env, nullptr);
        env->CallVoidMethod(main_, jmColouring, ret);
    }

    ~Colouring() override = default;

private:
    JavaVM *jvm_;
    jobject main_;
    jmethodID jmColouring;
};

#endif //VIS_COLOURING_H
