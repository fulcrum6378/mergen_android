#ifndef MOV_EXPRESSION_H
#define MOV_EXPRESSION_H

#define EXPRESSION_ID_SHAKING 0
#define EXPRESSION_ID_BEEPING 1
#define EXPRESSION_ID_COLOURING 2

class Expression {
public:
    uint8_t id;
    [[maybe_unused]] int8_t interaction_id;

    Expression(uint16_t _id, [[maybe_unused]] int8_t interaction_id) : id(_id), interaction_id() {}

    virtual void OnReward(double fortuna) {}

    virtual ~Expression() = default;
};

#endif //MOV_EXPRESSION_H
