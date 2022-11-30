#ifndef AUD_ENGINE_H
#define AUD_ENGINE_H

#include "recorder.h"

/**
 * Can be used for both playing and recording audio.
 * Do not merge it into Record.
 */
class AudioEngine {
private:
    SLObjectItf slEngineObj_;
    SLEngineItf slEngineItf_;
    AudioRecorder *recorder_;

public:
    AudioEngine();

    bool StartRecording();

    bool StopRecording();

    ~AudioEngine();
};

#endif //AUD_ENGINE_H
