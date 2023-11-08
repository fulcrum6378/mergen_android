#ifndef MOV_EXPRESSION_H
#define MOV_EXPRESSION_H

#define EXPRESSION_ID_SHAKING 0u
#define EXPRESSION_ID_BEEPING 1u
#define EXPRESSION_ID_COLOURING 2u

class Expression {
public:
    Expression(uint16_t _id, [[maybe_unused]] int8_t interactionId) : id(_id), interactionId() {}

    virtual void OnReward(double fortuna) {}

    virtual ~Expression() = default;


    uint8_t id;
    [[maybe_unused]] int8_t interactionId;
};

#endif //MOV_EXPRESSION_H
