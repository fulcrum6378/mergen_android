#ifndef MOV_EXPRESSION_H
#define MOV_EXPRESSION_H

#include <jni.h>

class Expression {
protected:
    JNIEnv *env_;
    jobject main_;

public:
    Expression(JNIEnv *env, jobject main) : env_(env), main_(main) {}

    virtual void OnReward(double fortuna) = 0;

    virtual ~Expression() {}
};

#endif //MOV_EXPRESSION_H
