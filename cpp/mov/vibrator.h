#ifndef MOV_VIBRATOR_H
#define MOV_VIBRATOR_H

#include "../rew/expression.h"

#define OUTPUT_ID_VIBRATOR -3

class Vibrator : public Expression {
public:
    Vibrator(JNIEnv *env, jobject main) :
            Expression(EXPRESSION_ID_VIBRATOR_1, OUTPUT_ID_VIBRATOR, env, main) {}

    void OnReward(double fortuna);

    ~Vibrator() {}
};

#endif //MOV_VIBRATOR_H
