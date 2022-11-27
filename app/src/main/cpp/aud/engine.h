#ifndef AUD_ENGINE_H
#define AUD_ENGINE_H

#include "recorder.h"

class AudioEngine {
public:
    AudioEngine(int32_t sampleRate, int32_t framesPerBuf);

    ~AudioEngine();

    bool StartRecording();

    bool StopRecording();

private:
    SLmilliHertz fastPathSampleRate_;
    uint32_t fastPathFramesPerBuf_;
    uint16_t sampleChannels_;
    uint16_t bitsPerSample_;

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
