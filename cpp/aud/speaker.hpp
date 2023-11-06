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

    SLAndroidDataFormat_PCM_EX pcmFormat{
            SL_DATAFORMAT_PCM,
            1,
            SL_SAMPLINGRATE_48, // 48000000 millihertz
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16, // change both together
            SL_SPEAKER_FRONT_LEFT,
            SL_BYTEORDER_LITTLEENDIAN, // FIXME
            SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT, // TODO
            // SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT, SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT,
            // SL_ANDROID_PCM_REPRESENTATION_FLOAT
    };

    ProducerConsumerQueue<sample_buf *> *freeQueue_;       // user
    ProducerConsumerQueue<sample_buf *> *playQueue_;       // user
    ProducerConsumerQueue<sample_buf *> *devShadowQueue_;  // owner

    sample_buf silentBuf_{};
    std::mutex stopMutex_;

public:
    Speaker(SLEngineItf engine);

    void SetBufQueue(ProducerConsumerQueue<sample_buf *> *playQ, ProducerConsumerQueue<sample_buf *> *freeQ);

    SLresult Start(void);

    void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq);

    void Stop(void);

    ~Speaker();
};

#endif  // AUD_SPEAKER_H
