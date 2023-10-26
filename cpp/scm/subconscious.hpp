#ifndef SCM_SUBCONSCIOUS_H
#define SCM_SUBCONSCIOUS_H

#include "../vis/visual_ltm.hpp"

class Subconscious {
public:
    VisualLTM *vis;

    Subconscious() : vis(new VisualLTM()) {}

    ~Subconscious() {
        delete vis;
    }
};

#endif //SCM_SUBCONSCIOUS_H
