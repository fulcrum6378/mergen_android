#ifndef AUD_RECORDER_H
#define AUD_RECORDER_H

#include <fstream>

#include "commons.h"

class AudioRecorder {
    SLObjectItf recObjectItf_{};
    SLRecordItf recItf_{};
    SLAndroidSimpleBufferQueueItf recBufQueueItf_{};

    sample_buf *bufs_{};
    uint32_t bufCount_{};
    SampleFormat sampleInfo_{};
    ProducerConsumerQueue<sample_buf *> *freeQueue_;       // user
    ProducerConsumerQueue<sample_buf *> *recQueue_;        // user
    // recBufQueue_ is recQueue_ for the recorder and playQueue_ for the player.
    ProducerConsumerQueue<sample_buf *> *devShadowQueue_;  // owner
    uint32_t audioBufCount{};
    sample_buf silentBuf_{};

    std::ofstream test;

    void SetBufQueues();

    void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);

public:
    AudioRecorder(SLEngineItf slEngine);

    bool Start(void);

    bool Stop(void);

    ~AudioRecorder();
};

#endif //AUD_RECORDER_H
