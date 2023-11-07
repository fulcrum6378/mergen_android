#ifndef MOV_VIBRATOR_H
#define MOV_VIBRATOR_H

#include <jni.h>

#include "../rew/expression.hpp"

#define OUTPUT_ID_VIBRATOR (-3)

class Vibrator {
private:
    JNIEnv *env_;
    jobject main_;

public:
    Vibrator(JNIEnv *env, jobject main) : env_(env), main_(main) {}

    void SetAmplitude(int32_t amplitude) {
        env_->CallVoidMethod(
                main_,
                env_->GetMethodID(
                        env_->FindClass("ir/mahdiparastesh/mergen/Main"),
                        "vibrate", "(I)V"),
                amplitude);
    }

    ~Vibrator() = default;
};

#endif //MOV_VIBRATOR_H
