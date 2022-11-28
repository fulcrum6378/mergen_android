#ifndef AUD_ENGINE_H
#define AUD_ENGINE_H

#include "recorder.h"

class AudioEngine {
public:
    AudioEngine();

    ~AudioEngine();

    bool StartRecording();

    bool StopRecording();

private:
    SLObjectItf slEngineObj_;
    SLEngineItf slEngineItf_;

    AudioRecorder *recorder_;
    AudioQueue *freeBufQueue_;  // Owner of the queue
    AudioQueue *recBufQueue_;   // Owner of the queue
    // recBufQueue_ is recQueue_ for the recorder and playQueue_ for the player.

    sample_buf *bufs_;
    uint32_t bufCount_;
};

#endif //AUD_ENGINE_H
