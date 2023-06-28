#ifndef MOV_EXPRESSION_H
#define MOV_EXPRESSION_H

#include <jni.h>

#define EXPRESSION_ID_VIBRATOR_1 0
#define EXPRESSION_ID_SCREEN_1 1

class Expression {
protected:
    JNIEnv *env_;
    jobject main_;

public:
    uint16_t id;
    int8_t interaction_id;

    Expression(uint16_t _id, int8_t interaction_id, JNIEnv *env, jobject main) :
            env_(env), main_(main), id(_id), interaction_id() {}

    virtual void OnReward(double fortuna) = 0;

    virtual ~Expression() {}
};

#endif //MOV_EXPRESSION_H
