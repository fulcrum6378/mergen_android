#ifndef AUD_SPEAKER_H
#define AUD_SPEAKER_H

#include <sys/types.h>
#include <mutex>

#include "../global.hpp"
#include "commons.hpp"

class Speaker {
    // buffer queue player interfaces
    SLObjectItf outputMixObjectItf_{};
    SLObjectItf playerObjectItf_{};
    SLPlayItf playItf_{};
    SLAndroidSimpleBufferQueueItf playBufferQueueItf_{};

    ProducerConsumerQueue<sample_buf *> *freeQueue_;       // user
    ProducerConsumerQueue<sample_buf *> *playQueue_;       // user
    ProducerConsumerQueue<sample_buf *> *devShadowQueue_;  // owner

    sample_buf silentBuf_{};
    std::mutex stopMutex_;

public:
    explicit Speaker(SLEngineItf engine);

    ~Speaker();

    void SetBufQueue(ProducerConsumerQueue<sample_buf *> *playQ, ProducerConsumerQueue<sample_buf *> *freeQ);

    SLresult Start(void);

    void Stop(void);

    void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);
};

#endif  // AUD_SPEAKER_H
