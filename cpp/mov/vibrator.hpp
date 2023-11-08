#ifndef MOV_VIBRATOR_H
#define MOV_VIBRATOR_H

#include <jni.h>

#include "../rew/expression.hpp"

#define OUTPUT_ID_VIBRATOR (-3)

class Vibrator {
public:
    Vibrator(JavaVM *jvm, jobject main) : jvm_(jvm), main_(main) {
        JNIEnv *env;
        jvm_->GetEnv((void **) &env, JNI_VERSION_1_6);
        jmVibrate = env->GetMethodID(
                env->FindClass("ir/mahdiparastesh/mergen/Main"),
                "vibrate", "(I)V");
    }

    void SetAmplitude(int32_t amplitude) {
        JNIEnv *env;
        if (jvm_->GetEnv((void **) &env, JNI_VERSION_1_6) == JNI_EDETACHED)
            jvm_->AttachCurrentThread(&env, nullptr);
        env->CallVoidMethod(main_, jmVibrate, amplitude);
    }

    ~Vibrator() = default;

private:
    JavaVM *jvm_;
    jobject main_;
    jmethodID jmVibrate;
};

#endif //MOV_VIBRATOR_H
