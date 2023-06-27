#ifndef AUD_RECORDER_H
#define AUD_RECORDER_H

#include <fstream>

#include "commons.h"
#include "../mem/queuer.h"

//#define SENSE_ID_MICROPHONE 2

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
    [[maybe_unused]] Queuer *queuer_;

    void SetBufQueues();

    void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq, int64_t time);

public:
    AudioRecorder(SLEngineItf slEngine, Queuer *queuer);

    bool Start(void);

    bool Stop(void);

    ~AudioRecorder();
};

#endif //AUD_RECORDER_H
