#ifndef SCM_CRAWLER_H
#define SCM_CRAWLER_H

#include "../vis/visual_stm.h"

class Crawler {
public:
    VisualSTM *vis;

    Crawler() : vis(new VisualSTM()) {}

    ~Crawler() {
        delete vis;
    }
};

#endif //SCM_CRAWLER_H
