#ifndef MOV_VIBRATOR_H
#define MOV_VIBRATOR_H

#include <mutex>
#include <thread>

#include "../rew/expression.h"

#define OUTPUT_ID_VIBRATOR -3

class Vibrator : std::thread, public Expression {
private:
    const jlong timeframe = 200;

    bool active_{false};
    jint amplitude_{0};
    JavaVM *jvm_;
    std::mutex lock;

public:
    Vibrator(JavaVM *jvm, JNIEnv *env, jobject main);

    void Run();

    void OnReward(double fortuna);

    ~Vibrator();
};

#endif //MOV_VIBRATOR_H
