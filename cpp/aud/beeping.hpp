#ifndef AUD_BEEPING_H
#define AUD_BEEPING_H

#include <cmath>

#include "../aud/speaker.hpp"
#include "../rew/expression.hpp"

#define OUTPUT_ID_SPEAKER (-2)

class Beeping : public Expression {
public:
    explicit Beeping(Speaker *aud_out) :
            Expression(EXPRESSION_ID_BEEPING, OUTPUT_ID_SPEAKER),
            aud_out_(aud_out) {}

    void OnReward(double fortuna) override {
        bool qualified = fortuna < -0.5 || fortuna > 0.5;
        if (qualified == aud_out_->IsStarted()) return;
        if (qualified) aud_out_->Start(&Beeping::AudCallback, this);
        else aud_out_->Stop();
    }

    static void AudCallback(int16_t *buf, int32_t numFrames, void */*data*/) {
        int togval = 0, togmax = 20, overval = 0, overmax = 400;
        float x, overadd = 0.1f;
        bool togg = true, over = false;
        for (int f = 0; f < numFrames; f++) {

            x = 0;
            int median = numFrames / 2;
            if (f < median)
                x += (float) f;
            else
                x += (float) abs(f - numFrames);
            //x *= 0.000001;

            if (over) x += overadd * (x + 0.1f);
            if (overval >= overmax) {
                over = !over;
                overval = 0;
            } else overval += 1;

            if (!togg) x = 0 - x;
            if (togval >= togmax) {
                togg = !togg;
                togval = 0;
            } else togval += 1;

            buf[f] = static_cast<int16_t>(x * 32767); //(rand() % 65535) - 32768;
        }
        //auto *beeping = static_cast<Beeping *>(data);
    }

    ~Beeping() override = default;

private:
    Speaker *aud_out_;
};

#endif //AUD_BEEPING_H
