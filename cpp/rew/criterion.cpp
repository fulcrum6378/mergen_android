#include "criterion.hpp"
#include "rewarder.hpp"

/** do NOT move this to the header file;
 * apparently C++ doesn't allow direct circular import, but allows an indirect one! */
void Criterion::Elasticity() {
    // TODO score <= 0.0f
    while (score != 0.0f) {
        //Rewarder::Compute();
    }
    elasticity = nullptr;
}
