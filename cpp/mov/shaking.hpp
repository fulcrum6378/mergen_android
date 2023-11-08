#ifndef MOV_SHAKING_H
#define MOV_SHAKING_H

#include <jni.h>

#include "../rew/expression.hpp"
#include "vibrator.hpp"

#define OUTPUT_ID_VIBRATOR (-3)

class Shaking : public Expression {
public:
    explicit Shaking(Vibrator *mov) :
            Expression(EXPRESSION_ID_SHAKING, OUTPUT_ID_VIBRATOR),
            mov_(mov) {}

    void OnReward(double fortuna) override {
        jint amplitude_;
        if (fortuna >= -0.5 && fortuna < 0.5) amplitude_ = 0;
        else amplitude_ = 1 + static_cast<int32_t>(((fortuna + 1.0) / 2.0) * 254);
        mov_->SetAmplitude(amplitude_);
    }

    ~Shaking() override = default;

private:
    Vibrator *mov_;
};

#endif //MOV_SHAKING_H
