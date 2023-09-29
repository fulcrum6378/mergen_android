#ifndef VIS_COLOURING_H
#define VIS_COLOURING_H

#include "../rew/expression.h"

#define OUTPUT_ID_SCREEN (-1)

class Colouring : public Expression {
public:
    Colouring(JNIEnv *env, jobject main) :
            Expression(EXPRESSION_ID_SCREEN_1, OUTPUT_ID_SCREEN, env, main) {}

    void OnReward(double fortuna);

    ~Colouring() {}
};

#endif //VIS_COLOURING_H
