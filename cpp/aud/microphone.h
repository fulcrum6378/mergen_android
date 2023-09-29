#ifndef AUD_MICROPHONE_H
#define AUD_MICROPHONE_H

#include <fstream>

#include "commons.h"

static const bool AUD_SAVE = true;

class Microphone {
private:
    std::ofstream test;
    Engine *slEngine_;

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

    void SetBufQueues();

    void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq, int64_t time);

public:
    Microphone();

    bool Start(void);

    bool Stop(void);

    ~Microphone();
};

#endif //AUD_MICROPHONE_H
