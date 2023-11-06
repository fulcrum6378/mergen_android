#ifndef AUD_SPEAKER_H
#define AUD_SPEAKER_H

#include "commons.hpp"

class Speaker {
public:
    Speaker();

    void Start(void);

    void Stop(void);

    ~Speaker();
};

#endif  // AUD_SPEAKER_H
