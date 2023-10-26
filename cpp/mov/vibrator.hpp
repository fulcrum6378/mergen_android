#ifndef MOV_VIBRATOR_H
#define MOV_VIBRATOR_H

#include "../rew/expression.hpp"

#define OUTPUT_ID_VIBRATOR (-3)

class Vibrator : public Expression {
public:
    Vibrator(JNIEnv *env, jobject main) :
            Expression(EXPRESSION_ID_VIBRATOR_1, OUTPUT_ID_VIBRATOR, env, main) {}

    void OnReward(double fortuna) override {
        jint amplitude_;
        if (fortuna >= -0.75 && fortuna < 0.75) amplitude_ = 0;
        else amplitude_ = 1 + (int32_t) (((fortuna + 1.0) / 2.0) * 254);
        jmethodID method = env_->GetMethodID(
                env_->FindClass("ir/mahdiparastesh/mergen/Main"), "vibrate", "(I)V");
        env_->CallVoidMethod(main_, method, amplitude_);
    }

    ~Vibrator() override = default;
};

#endif //MOV_VIBRATOR_H
