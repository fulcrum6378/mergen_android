#include "vibrator.h"

Vibrator::Vibrator(JavaVM *jvm, JNIEnv *env, jobject main) :
        std::thread(&Vibrator::Run, this),
        Expression(EXPRESSION_ID_VIBRATOR_1, OUTPUT_ID_VIBRATOR, env, main) {
    jvm_ = jvm;
    active_ = true;
}

void Vibrator::Run() {
    jvm_->AttachCurrentThread(&env_, nullptr);
    while (active_) {
        lock.lock();
        jint amplitude = amplitude_;
        lock.unlock();

        if (amplitude_ > 0) {
            jmethodID method = env_->GetMethodID(
                    env_->FindClass("ir/mahdiparastesh/mergen/Main"), "vibrate", "(JI)V");
            env_->CallVoidMethod(main_, method, timeframe, amplitude);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeframe));
    }
}

void Vibrator::OnReward(double fortuna) {
    if (fortuna >= -0.75 && fortuna < 0.75) amplitude_ = 0;
    else amplitude_ = 1 + (int32_t) (((fortuna + 1.0) / 2.0) * 254);

}

Vibrator::~Vibrator() {
    active_ = false;
}
