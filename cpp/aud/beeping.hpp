#ifndef AUD_BEEPING_H
#define AUD_BEEPING_H

#include "../aud/speaker.hpp"
#include "../rew/expression.hpp"

#define OUTPUT_ID_SPEAKER (-2)

class Beeping : public Expression {
private:
    Speaker *aud_out_;

public:
    explicit Beeping(Speaker *aud_out) :
            Expression(EXPRESSION_ID_BEEPING, OUTPUT_ID_SPEAKER),
            aud_out_(aud_out) {}

    void OnReward(double fortuna) override {
        // TODO
    }

    ~Beeping() override = default;
};

#endif //AUD_BEEPING_H
